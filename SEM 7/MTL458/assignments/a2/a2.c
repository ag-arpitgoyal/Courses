#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>


#define MAX_QUEUES  8
#define MAX_JOBS    16
#define MAX_EVENTS  1024
#define NAME_CAP    32
#define LINEBUF     4096

#define MSG_FD      3
#define WORKLOAD_PATH "./workload"

#define NO_DEADLINE 0x3fffffffffffffffLL  // for no deadline, a very large number

#define MSG_NEAR_US 15000LL
#define IO_GRACE_MS 40

#define TIMER_TIE_US 5000LL
#define IO_TIE_WAIT_MS 10


typedef enum {
    ST_NEW, ST_READY, ST_RUNNING, ST_BLOCKED, ST_FINISHED
} state_t;

typedef struct {
    pid_t     pid;
    char      name[NAME_CAP];
    int       input_pos;
    long long arrival_us;
    int       level;
    long long turn_left_us;
    int       allotment_left;
    state_t   state;
    int       stop_confirmed;
    int       child_status;
    int       child_status_valid;
    long long resumed_at_us;
    int       performs_io;
    int       grace_spent;
    int       turn_expired;      
    long long io_every_us;
    long long cpu_since_io_us;
    long long cpu_left_us;
    long long io_end_due_us;
} job_t;


typedef struct {
    char name[NAME_CAP];
    int  arrival, cpu, io_every, io_dur;
} job_spec_t;

typedef struct {
    int items[MAX_JOBS];
    int head, count;
} deque_t;

typedef enum {
    EV_FINISH,
    EV_IO_START,
    EV_IO_END
} event_type_t;

typedef struct {
    event_type_t type;
    int          job_pos;
} event_t;

static event_t pending[MAX_EVENTS];
static int     n_pending;

static char    msg_buf[LINEBUF];
static size_t  msg_len;


static int n_queues, boost_period_ms, gaming_tolerance, n_jobs;
static int quantum_ms[MAX_QUEUES];
static int allotment[MAX_QUEUES];

static job_t      jobs[MAX_JOBS];
static job_spec_t job_specs[MAX_JOBS];
static deque_t    queues[MAX_QUEUES];

static struct timespec clock_start;
static int       msg_pipe = -1;
static int       running_job = -1;
static int       idle_announced;
static int       redispatch;
static int       n_finished;
static long long next_boost_us;

static long long now_us(void)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
        perror("clock_gettime");
        exit(1);
    }

    long long current = (long long)ts.tv_sec * 1000000LL + ts.tv_nsec / 1000LL;
    long long start = (long long)clock_start.tv_sec * 1000000LL + clock_start.tv_nsec / 1000LL;

    return current - start;
}

static long long to_ms(long long us)
{
    return (us + 500LL) / 1000LL;
}

static void charge_cpu_time(job_t *job)
{
    long long now = now_us();
    long long elapsed = now - job->resumed_at_us;

    if (elapsed > 0) {
        job->turn_left_us -= elapsed;

        if (job->turn_left_us < 0)
            job->turn_left_us = 0;

        job->cpu_since_io_us += elapsed;
        job->cpu_left_us -= elapsed;

        if (job->cpu_left_us < 0)
            job->cpu_left_us = 0;
    }

    job->resumed_at_us = now;
}


static int q_empty(const deque_t *q)
{
    return q->count == 0;
}

static void q_push_back(deque_t *q, int job_pos)
{
    if (q->count >= MAX_JOBS) {
        fprintf(stderr, "queue overflow\n");
        exit(1);
    }

    int pos = (q->head + q->count) % MAX_JOBS;

    q->items[pos] = job_pos;
    q->count++;
}

static int q_pop_front(deque_t *q)
{
    if (q_empty(q))
        return -1;

    int job_pos = q->items[q->head];

    q->head = (q->head + 1) % MAX_JOBS;
    q->count--;

    return job_pos;
}

static int q_peek(const deque_t *q)
{
    if (q_empty(q))
        return -1;

    return q->items[q->head];
}


static int q_remove(deque_t *q, int job_pos)
{
    for (int k = 0; k < q->count; ++k) {
        int pos = (q->head + k) % MAX_JOBS;

        if (q->items[pos] != job_pos)
            continue;

        for (int m = k; m < q->count - 1; ++m) {
            int here = (q->head + m) % MAX_JOBS;
            int next = (q->head + m + 1) % MAX_JOBS;

            q->items[here] = q->items[next];
        }

        q->count--;

        return 1;
    }

    return 0;
}


/*   t ARRIVE name | t RUN name Qk | t IDLE | t QEXPIRE name Qk
 *   t DEMOTE name Qfrom Qto | t IO_START name | t IO_END name
 *   t BOOST | t FINISH name
 */
static void print_event(const char *fmt, ...)
{
    va_list ap;

    printf("%lld ", to_ms(now_us()));

    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    putchar('\n');
}

static void drain_pipe(void)
{
    for (;;) {
        ssize_t r = read(msg_pipe, msg_buf + msg_len, LINEBUF - msg_len);

        if (r > 0) {
            msg_len += (size_t)r;
        }
        else if (r == 0) {
            break;
        }
        else {
            if (errno == EINTR)
                continue;

            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;

            perror("read");
            exit(1);
        }

        size_t start = 0;

        for (size_t i = 0; i < msg_len; ++i) {
            if (msg_buf[i] != '\n')
                continue;

            msg_buf[i] = '\0';

            char type[16];
            char name[NAME_CAP];

            if (sscanf(msg_buf + start, "%15s %31s", type, name) != 2) {
                fprintf(stderr, "invalid pipe message: %s\n",
                        msg_buf + start);
                exit(1);
            }

            int job_pos = -1;

            for (int k = 0; k < n_jobs; ++k) {
                if (strcmp(jobs[k].name, name) == 0) {
                    job_pos = k;
                    break;
                }
            }

            if (job_pos == -1) {
                fprintf(stderr, "unknown job in pipe message: %s\n", name);
                exit(1);
            }

            if (n_pending >= MAX_EVENTS) {
                fprintf(stderr, "event buffer overflow\n");
                exit(1);
            }

            if (strcmp(type, "DONE") == 0) {
                pending[n_pending].type = EV_FINISH;
            }
            else if (strcmp(type, "IO") == 0) {
                pending[n_pending].type = EV_IO_START;
            }
            else if (strcmp(type, "RDY") == 0) {
                pending[n_pending].type = EV_IO_END;
            }
            else {
                fprintf(stderr, "unknown pipe message type: %s\n", type);
                exit(1);
            }

            pending[n_pending].job_pos = job_pos;
            n_pending++;

            start = i + 1;
        }

        if (start > 0) {
            msg_len -= start;
            memmove(msg_buf, msg_buf + start, msg_len);
        }

        if (msg_len == LINEBUF) {
            fprintf(stderr, "pipe message too long\n");
            exit(1);
        }
    }
}


/*
 * Return 1 when the child was stopped, or 0 when it exited before the
 * requested stop could be observed.  Exit status is retained for on_finish().
 */
static int confirm_stopped(job_t *job)
{
    int status;

    for (;;) {
        pid_t ret = waitpid(job->pid, &status, WUNTRACED);

        if (ret == -1) {
            if (errno == EINTR)
                continue;

            perror("waitpid");
            exit(1);
        }

        if (WIFSTOPPED(status)) {
            job->stop_confirmed = 1;
            return 1;
        }

        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            job->child_status = status;
            job->child_status_valid = 1;
            job->stop_confirmed = 0;
            return 0;
        }
    }
}

static void resume(job_t *job)
{
    if (job->stop_confirmed) {
        if (kill(job->pid, SIGCONT) == -1) {
            perror("kill(SIGCONT)");
            exit(1);
        }

        job->stop_confirmed = 0;
    }

    job->resumed_at_us = now_us();
    job->grace_spent = 0;
}

static void spawn_all(void)
{
    int fds[2];

    if (pipe(fds) == -1) {
        perror("pipe");
        exit(1);
    }

    msg_pipe = fds[0];

    int flags = fcntl(msg_pipe, F_GETFL);
    if (flags == -1) {
        perror("fcntl");
        exit(1);
    }

    if (fcntl(msg_pipe, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl");
        exit(1);
    }

    for (int i = 0; i < n_jobs; ++i) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            exit(1);
        }

        if (pid == 0) {
            close(fds[0]);

            if (fds[1] != MSG_FD) {
                if (dup2(fds[1], MSG_FD) == -1) {
                    perror("dup2");
                    _exit(127);
                }

                close(fds[1]);
            }

            char cpu_buf[32];
            char io_every_buf[32];
            char io_dur_buf[32];

            snprintf(cpu_buf, sizeof(cpu_buf),
                     "%d", job_specs[i].cpu);

            snprintf(io_every_buf, sizeof(io_every_buf),
                     "%d", job_specs[i].io_every);

            snprintf(io_dur_buf, sizeof(io_dur_buf),
                     "%d", job_specs[i].io_dur);

            char *argv[] = {
                (char *)WORKLOAD_PATH,
                job_specs[i].name,
                cpu_buf,
                io_every_buf,
                io_dur_buf,
                NULL
            };

            execv(WORKLOAD_PATH, argv);

            argv[0] = (char *)"workload";
            execvp("workload", argv);

            perror("exec workload");
            _exit(127);
        }

        jobs[i].pid = pid;
    }

    close(fds[1]);

    for (int i = 0; i < n_jobs; ++i)
        if (!confirm_stopped(&jobs[i])) {
            fprintf(stderr, "child %s exited before its initial stop\n",
                    jobs[i].name);
            exit(1);
        }
}


static void on_finish(job_t *job)
{
    int status;

    print_event("FINISH %s", job->name);

    if (job->child_status_valid) {
        status = job->child_status;
        job->child_status_valid = 0;
    }
    else {
        while (waitpid(job->pid, &status, 0) == -1) {
            if (errno != EINTR) {
                perror("waitpid");
                exit(1);
            }
        }
    }

    if (!q_remove(&queues[job->level], job->input_pos)) {
        fprintf(stderr, "queue corruption on FINISH %s\n", job->name);
        exit(1);
    }

    job->state = ST_FINISHED;

    if (running_job == job->input_pos)
        running_job = -1;

    n_finished++;

    if (n_finished == n_jobs)
        exit(0);
}

/* QEXPIRE or DEMOTE a job whose turn ran out; the caller handles queues. */
static void expire_turn(job_t *job)
{
    int level = job->level;

    job->allotment_left--;

    if (job->allotment_left > 0 || level == 0) {
        print_event("QEXPIRE %s Q%d", job->name, level);

        if (job->allotment_left <= 0)
            job->allotment_left = allotment[0];

        job->turn_left_us = (long long)quantum_ms[level] * 1000LL;
    }
    else {
        print_event("DEMOTE %s Q%d Q%d", job->name, level, level - 1);

        job->level = level - 1;
        job->turn_left_us = (long long)quantum_ms[level - 1] * 1000LL;
        job->allotment_left = allotment[level - 1];
    }
}

static void on_io_start(job_t *job)
{
    print_event("IO_START %s", job->name);

    if (!q_remove(&queues[job->level], job->input_pos)) {
        fprintf(stderr, "queue corruption on IO_START %s\n", job->name);
        exit(1);
    }

    job->state = ST_BLOCKED;
    job->cpu_since_io_us = 0;
    job->io_end_due_us = now_us() + (long long)job_specs[job->input_pos].io_dur * 1000LL;

    if (gaming_tolerance == 0) {
        job->turn_left_us = (long long)quantum_ms[job->level] * 1000LL;
        job->allotment_left = allotment[job->level];
    }
    else if (job->turn_left_us <= 0) {
        job->turn_expired = 1;      /* printed after this instant's IO_ENDs */
    }

    running_job = -1;
}

static void on_io_end(job_t *job)
{
    if (!confirm_stopped(job)) {
        fprintf(stderr, "child %s exited before I/O completion stop\n",
                job->name);
        exit(1);
    }

    print_event("IO_END %s", job->name);

    job->state = ST_READY;

    q_push_back(&queues[job->level], job->input_pos);
}

static void on_quantum(job_t *job)
{
    if (q_pop_front(&queues[job->level]) != job->input_pos) {
        fprintf(stderr, "queue corruption on quantum expiry %s\n", job->name);
        exit(1);
    }

    expire_turn(job);

    q_push_back(&queues[job->level], job->input_pos);

    redispatch = 1;
}

static void on_arrival(job_t *job)
{
    print_event("ARRIVE %s", job->name);

    job->state = ST_READY;
    job->level = n_queues - 1;

    job->turn_left_us = (long long)quantum_ms[job->level] * 1000LL;
    job->allotment_left = allotment[job->level];

    q_push_back(&queues[job->level], job->input_pos);
}

static void on_boost(void)
{
    print_event("BOOST");
    for (int i = 0; i < n_jobs; ++i) {
        if (jobs[i].state == ST_READY ||
            jobs[i].state == ST_RUNNING ||
            jobs[i].state == ST_BLOCKED) {

            jobs[i].level = n_queues - 1;
            jobs[i].turn_left_us =
                (long long)quantum_ms[n_queues - 1] * 1000LL;
            jobs[i].allotment_left = allotment[n_queues - 1];
        }
    }

    for (int level = 0; level < n_queues; ++level) {
        queues[level].head = 0;
        queues[level].count = 0;
    }

    for (int i = 0; i < n_jobs; ++i) {
        if (jobs[i].state == ST_READY ||
            jobs[i].state == ST_RUNNING) {

            q_push_back(&queues[n_queues - 1], i);
        }
    }

    redispatch = 1;

    next_boost_us += (long long)boost_period_ms * 1000LL;
}

static int find_leave_message(const job_t *job)
{
    for (int i = 0; i < n_pending; ++i) {
        if (pending[i].job_pos == job->input_pos &&
            (pending[i].type == EV_IO_START || pending[i].type == EV_FINISH))
            return i;
    }

    return -1;
}

/* Returns 1 if the job is now parked, 0 if its IO/DONE is in pending[] (a job
 * in I/O is never left stopped). */
static int stop_running(job_t *job)
{
    drain_pipe();

    if (find_leave_message(job) >= 0)
        return 0;

    if (!job->stop_confirmed) {
        if (kill(job->pid, SIGSTOP) == -1 && errno != ESRCH) {
            perror("kill(SIGSTOP)");
            exit(1);
        }

        if (!confirm_stopped(job)) {
            drain_pipe();
            return 0;
        }
    }

    drain_pipe();

    if (find_leave_message(job) < 0)
        return 1;

    if (kill(job->pid, SIGCONT) == -1) {
        perror("kill(SIGCONT)");
        exit(1);
    }

    job->stop_confirmed = 0;

    return 0;
}

static void preempt_safely(job_t *job)
{
    if (stop_running(job))
        return;

    int i = find_leave_message(job);
    event_t event = pending[i];

    for (int k = i; k < n_pending - 1; ++k)
        pending[k] = pending[k + 1];

    n_pending--;

    if (event.type == EV_FINISH) {
        on_finish(job);
        return;
    }

    on_io_start(job);

    if (job->turn_expired) {
        job->turn_expired = 0;
        expire_turn(job);
    }
}

static int highest_ready_job(void)
{
    for (int level = n_queues - 1; level >= 0; --level) {
        if (!q_empty(&queues[level]))
            return q_peek(&queues[level]);
    }

    return -1;
}

static void dispatch(void)
{
    int chosen = highest_ready_job();

    if (chosen == running_job && !redispatch) {
        if (chosen != -1)
            return;
    }

    if (running_job != -1 && chosen != running_job) {
        job_t *current = &jobs[running_job];

        preempt_safely(current);
        if (current->state == ST_RUNNING)
            current->state = ST_READY;
    }

    if (chosen == -1) {
        if (!idle_announced) {
            print_event("IDLE");
            idle_announced = 1;
        }
    }
    else {
        job_t *next = &jobs[chosen];

        resume(next);

        next->state = ST_RUNNING;
        idle_announced = 0;

        print_event("RUN %s Q%d", next->name, next->level);
    }

    running_job = chosen;
    redispatch = 0;
}

/* --------------------------------------------------------------- input */

static void parse_input(const char *path)
{
    FILE *fp = fopen(path, "r");
    if (!fp) {
        perror("fopen");
        exit(1);
    }

    if (fscanf(fp, "%d %d %d",
               &n_queues, &boost_period_ms, &gaming_tolerance) != 3) {
        fprintf(stderr, "invalid scheduler configuration\n");
        exit(1);
    }

    for (int i = 0; i < n_queues; ++i) {
        if (fscanf(fp, "%d", &quantum_ms[n_queues - 1 - i]) != 1) {
            fprintf(stderr, "invalid quantum configuration\n");
            exit(1);
        }
    }

    for (int i = 0; i < n_queues; ++i) {
        if (fscanf(fp, "%d", &allotment[n_queues - 1 - i]) != 1) {
            fprintf(stderr, "invalid allotment configuration\n");
            exit(1);
        }
    }

    if (fscanf(fp, "%d", &n_jobs) != 1) {
        fprintf(stderr, "invalid job count\n");
        exit(1);
    }

    for (int i = 0; i < n_jobs; ++i) {
        int arrival, cpu, io_every, io_dur;

        if (fscanf(fp, "%31s %d %d %d %d",
                   job_specs[i].name,
                   &arrival,
                   &cpu,
                   &io_every,
                   &io_dur) != 5) {
            fprintf(stderr, "invalid job specification\n");
            exit(1);
        }

        job_specs[i].arrival = arrival;
        job_specs[i].cpu = cpu;
        job_specs[i].io_every = io_every;
        job_specs[i].io_dur = io_dur;

        jobs[i].pid = -1;

        size_t len = strnlen(job_specs[i].name, NAME_CAP - 1);

        memcpy(jobs[i].name, job_specs[i].name, len);
        jobs[i].name[len] = '\0';

        jobs[i].input_pos = i;
        jobs[i].arrival_us = (long long)arrival * 1000LL;

        jobs[i].level = n_queues - 1;
        jobs[i].turn_left_us = 0;
        jobs[i].allotment_left = 0;

        jobs[i].state = ST_NEW;
        jobs[i].stop_confirmed = 0;
        jobs[i].resumed_at_us = 0;
        jobs[i].performs_io = (io_every > 0);
        jobs[i].grace_spent = 0;
        jobs[i].turn_expired = 0;
        jobs[i].io_every_us = (long long)io_every * 1000LL;
        jobs[i].cpu_since_io_us = 0;
        jobs[i].cpu_left_us = (long long)cpu * 1000LL;
    }

    for (int i = 0; i < n_queues; ++i) {
        queues[i].head = 0;
        queues[i].count = 0;
    }

    fclose(fp);
}

/* -------------------------------------------------------  main loop */

static int event_rank(event_type_t type)
{
    switch (type) {
    case EV_FINISH:
        return 0;
    case EV_IO_START:
        return 1;
    case EV_IO_END:
        return 2;
    default:
        return 99;
    }
}

static void sort_events(void)
{
    for (int i = 1; i < n_pending; ++i) {
        event_t key = pending[i];
        int key_rank = event_rank(key.type);
        int key_pos = jobs[key.job_pos].input_pos;
        int at = i - 1;

        while (at >= 0) {
            int rank = event_rank(pending[at].type);
            int pos = jobs[pending[at].job_pos].input_pos;

            if (rank < key_rank ||
                (rank == key_rank && pos <= key_pos))
                break;

            pending[at + 1] = pending[at];
            --at;
        }

        pending[at + 1] = key;
    }
}

static void handle_events(void)
{
    sort_events();

    while (n_pending > 0) {
        event_t ev = pending[0];

        for (int i = 1; i < n_pending; ++i)
            pending[i - 1] = pending[i];
        --n_pending;

        job_t *job = &jobs[ev.job_pos];

        switch (ev.type) {
        case EV_FINISH:
            on_finish(job);
            break;
        case EV_IO_START:
            on_io_start(job);
            break;
        case EV_IO_END:
            on_io_end(job);
            break;
        }
    }
}


static int fire_due_timers(long long now)
{
    int fired = 0;
    int other_timer_due = boost_period_ms > 0 && next_boost_us <= now;

    for (int i = 0; i < n_jobs; ++i) {
        if (jobs[i].turn_expired) {
            jobs[i].turn_expired = 0;
            expire_turn(&jobs[i]);
            fired = 1;
        }

        if (jobs[i].state == ST_NEW && jobs[i].arrival_us <= now)
            other_timer_due = 1;
    }

    long long tie = other_timer_due ? TIMER_TIE_US : 0;

    if (running_job != -1 && jobs[running_job].turn_left_us <= tie) {
        on_quantum(&jobs[running_job]);
        fired = 1;
    }

    for (int i = 0; i < n_jobs; ++i) {
        if (jobs[i].state == ST_NEW && jobs[i].arrival_us <= now) {
            on_arrival(&jobs[i]);
            fired = 1;
        }
    }

    if (boost_period_ms > 0 && next_boost_us <= now) {
        on_boost();
        fired = 1;
    }

    return fired;
}

static int message_imminent(const job_t *job)
{
    if (job->cpu_left_us <= MSG_NEAR_US)
        return 1;               /* it is about to finish */

    return job->performs_io
           && job->io_every_us - job->cpu_since_io_us <= MSG_NEAR_US;
}

static int grace_for_message(job_t *job)
{
    if (job->grace_spent || !message_imminent(job))
        return 0;

    job->grace_spent = 1;

    struct pollfd pfd;
    pfd.fd = msg_pipe;
    pfd.events = POLLIN | POLLHUP | POLLERR;
    pfd.revents = 0;

    int ret = poll(&pfd, 1, IO_GRACE_MS);

    if (ret < 0 && errno != EINTR) {
        perror("poll");
        exit(1);
    }

    drain_pipe();

    return 1;
}

static int timer_due(long long now)
{
    if (running_job != -1 && jobs[running_job].turn_left_us <= 0)
        return 1;

    if (boost_period_ms > 0 && next_boost_us <= now)
        return 1;

    for (int i = 0; i < n_jobs; ++i) {
        if (jobs[i].state == ST_NEW && jobs[i].arrival_us <= now)
            return 1;
    }

    return 0;
}

static int io_end_imminent(long long now)
{
    for (int i = 0; i < n_jobs; ++i) {
        if (jobs[i].state == ST_BLOCKED &&
            jobs[i].io_end_due_us <= now + TIMER_TIE_US)
            return 1;
    }

    return 0;
}

static long long next_deadline(long long now)
{
    long long deadline = NO_DEADLINE;

    if (running_job != -1) {
        long long turn_ends = now + jobs[running_job].turn_left_us;

        if (turn_ends < deadline)
            deadline = turn_ends;
    }

    for (int i = 0; i < n_jobs; ++i) {
        if (jobs[i].state == ST_NEW && jobs[i].arrival_us < deadline)
            deadline = jobs[i].arrival_us;
    }

    if (boost_period_ms > 0 && next_boost_us < deadline)
        deadline = next_boost_us;

    return deadline;
}

static void run_loop(void)
{
    n_pending = 0;
    msg_len = 0;
    running_job = -1;
    redispatch = 0;
    idle_announced = 0;
    n_finished = 0;

    if (boost_period_ms > 0)
        next_boost_us = (long long)boost_period_ms * 1000LL;
    else
        next_boost_us = 0;

    fire_due_timers(now_us());
    dispatch();

    int tie_waited = 0;

    for (;;) {
        long long now = now_us();

        if (running_job != -1)
            charge_cpu_time(&jobs[running_job]);

        drain_pipe();

        /* Don't expire a turn if the job blocked or finished at the same instant. */
        if (n_pending == 0 && running_job != -1 &&
            jobs[running_job].turn_left_us <= 0) {
            job_t *job = &jobs[running_job];

            if (grace_for_message(job))
                continue;

            if (message_imminent(job) && !stop_running(job))
                continue;
        }

        now = now_us();

        /* An IO_END due at the same instant as a timer must be printed first. */
        if (n_pending == 0 && !tie_waited && timer_due(now) && io_end_imminent(now)) {
            struct pollfd pfd = { msg_pipe, POLLIN, 0 };

            tie_waited = 1;
            poll(&pfd, 1, IO_TIE_WAIT_MS);
            continue;
        }

        int acted = 0;

        if (n_pending > 0) {
            handle_events();
            acted = 1;
        }

        now = now_us();

        if (fire_due_timers(now)) {
            acted = 1;
            tie_waited = 0;
        }

        if (acted) {
            dispatch();
            continue;
        }

        long long deadline = next_deadline(now);
        int timeout_ms;

        if (deadline == NO_DEADLINE) {
            timeout_ms = -1;
        } else if (deadline <= now) {
            continue;
        } else {
            long long delta_us = deadline - now;
            long long timeout = (delta_us + 999LL) / 1000LL;

            if (timeout > INT_MAX)
                timeout_ms = INT_MAX;
            else
                timeout_ms = (int)timeout;
        }

        struct pollfd pfd;
        pfd.fd = msg_pipe;
        pfd.events = POLLIN | POLLHUP | POLLERR;
        pfd.revents = 0;

        int ret = poll(&pfd, 1, timeout_ms);

        if (ret < 0) {
            if (errno == EINTR)
                continue;

            perror("poll");
            exit(1);
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <input-file>\n", argv[0]);
        return 1;
    }

    setvbuf(stdout, NULL, _IOLBF, 0);

    parse_input(argv[1]);
    spawn_all();
    clock_gettime(CLOCK_MONOTONIC, &clock_start);
    run_loop();

    return 0;
}