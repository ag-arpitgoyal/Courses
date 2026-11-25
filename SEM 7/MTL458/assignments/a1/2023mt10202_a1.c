#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

char history[2048][2050];
int history_count = 0;
int history_start = 0;
char home_directory[2050] = "";
char previous_directory[2050] = "";
int should_exit = 0;

// Check whether a token is one of the redirection operators.
int is_redirect(char *s)
{
    if (strcmp(s, "<") == 0 || strcmp(s, ">") == 0 || strcmp(s, ">>") == 0) {
        return 1;
    }
    return 0;
}

// Separate words and shell operators into tokens.
int split(char *line, char *parsed, char *args[])
{
    int i = 0, j = 0, count = 0;

    while (line[i] != '\0') {
        char c = line[i];

        if (c == '<' || c == '>' || c == '&' || c == ';' || c == '|') {
            parsed[j++] = ' ';
            parsed[j++] = c;
            if (line[i + 1] == c && c != '<' && c != ';') {
                i++;
                parsed[j++] = c;
            }
            parsed[j++] = ' ';
        } else {
            parsed[j++] = c;
        }
        i++;
    }
    parsed[j] = '\0';

    char *word = strtok(parsed, " \t");
    while (word != NULL) {
        args[count++] = word;
        word = strtok(NULL, " \t");
    }
    args[count] = NULL;
    return count;
}

// Store commands in a circular history of size 2048.
void add_history(char *line)
{
    int index = (history_start + history_count) % 2048;
    strcpy(history[index], line);

    if (history_count < 2048) {
        history_count++;
    } else {
        history_start = (history_start + 1) % 2048;
    }
}

// Print all history entries or only the last requested entries.
int show_history(char *args[], int nargs)
{
    int first = 0;

    if (nargs == 2) {
        for (int i = 0; args[1][i] != '\0'; i++) {
            if (args[1][i] < '0' || args[1][i] > '9') {
                printf("Invalid Command\n");
                return 1;
            }
        }

        int last = atoi(args[1]);
        if (last <= 0) {
            printf("Invalid Command\n");
            return 1;
        }

        if (last < history_count) {
            first = history_count - last;
        }
    } else if (nargs != 1) {
        printf("Invalid Command\n");
        return 1;
    }

    for (int i = first; i < history_count; i++) {
        int index = (history_start + i) % 2048;
        printf("%s\n", history[index]);
    }

    return 0;
}

// Handle all supported forms of the cd built-in.
int change_directory(char *args[], int nargs)
{
    char current[2050];
    char *target;
    int print_path = 0;

    if (nargs > 2) {
        printf("Invalid Command\n");
        return 1;
    }

    if (nargs == 1 || strcmp(args[1], "~") == 0) {
        target = home_directory;
    } else if (strcmp(args[1], "-") == 0) {
        if (previous_directory[0] == '\0') {
            printf("Invalid Command\n");
            return 1;
        }
        target = previous_directory;
        print_path = 1;
    } else {
        target = args[1];
    }

    if (target[0] == '\0') {
        printf("Invalid Command\n");
        return 1;
    }

    if (getcwd(current, sizeof(current)) == NULL) {
        printf("Invalid Command\n");
        return 1;
    }

    if (chdir(target) != 0) {
        printf("Invalid Command\n");
        return 1;
    }

    strcpy(previous_directory, current);

    if (print_path) {
        if (getcwd(current, sizeof(current)) != NULL) {
            printf("%s\n", current);
        }
    }

    return 0;
}

// Run one external command, with optional file redirection.
int run_external(char *args[], int nargs)
{
    char *input_file = NULL;
    char *output_file = NULL;
    int append = 0;
    int j = 0;

    for (int i = 0; i < nargs; i++) {
        if (strcmp(args[i], "<") == 0) {
            if (input_file != NULL || i + 1 == nargs || is_redirect(args[i + 1])) {
                printf("Invalid Command\n");
                return 1;
            }
            input_file = args[++i];
        } else if (strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0) {
            if (output_file != NULL || i + 1 == nargs || is_redirect(args[i + 1])) {
                printf("Invalid Command\n");
                return 1;
            }
            if (strcmp(args[i], ">>") == 0) {
                append = 1;
            }
            output_file = args[++i];
        } else {
            args[j++] = args[i];
        }
    }
    args[j] = NULL;

    if (j == 0) {
        printf("Invalid Command\n");
        return 1;
    }

    int pid = fork();
    int status;

    if (pid < 0) {
        printf("Invalid Command\n");
        return 1;
    }

    if (pid == 0) {
        if (input_file != NULL) {
            int fd = open(input_file, O_RDONLY);
            if (fd < 0) {
                _exit(127);
            }
            if (dup2(fd, STDIN_FILENO) < 0) {
                _exit(127);
            }
            close(fd);
        }

        if (output_file != NULL) {
            int flags = O_WRONLY | O_CREAT;
            if (append) {
                flags = flags | O_APPEND;
            } else {
                flags = flags | O_TRUNC;
            }

            int fd = open(output_file, flags, 0644);
            if (fd < 0) {
                _exit(127);
            }
            if (dup2(fd, STDOUT_FILENO) < 0) {
                _exit(127);
            }
            close(fd);
        }

        signal(SIGPIPE, SIG_DFL);
        execvp(args[0], args);
        _exit(127);
    }

    if (waitpid(pid, &status, 0) == -1) {
        return 1;
    }

    if (WIFEXITED(status)) {
        int result = WEXITSTATUS(status);
        if (result == 127) {
            printf("Invalid Command\n");
        }
        return result;
    }

    return 1;
}

// Run up to three external commands connected by pipes.
int run_pipeline(char *args[], int nargs)
{
    char **commands[3];
    int pipes[2][2];
    int pids[3] = {-1, -1, -1};
    int count = 1;

    commands[0] = args;

    for (int i = 0; i < nargs; i++) {
        if (strcmp(args[i], "|") == 0) {
            if (count == 3) {
                printf("Invalid Command\n");
                return 1;
            }
            args[i] = NULL;
            commands[count++] = args + i + 1;
        }
    }

    for (int i = 0; i < count; i++) {
        if (commands[i][0] == NULL) {
            printf("Invalid Command\n");
            return 1;
        }
    }

    for (int i = 0; i < count - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            printf("Invalid Command\n");
            for (int j = 0; j < i; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            return 1;
        }
    }

    for (int i = 0; i < count; i++) {
        pids[i] = fork();

        if (pids[i] < 0) {
            printf("Invalid Command\n");
            continue;
        }

        if (pids[i] == 0) {
            int shell_stdout = dup(STDOUT_FILENO);
            fcntl(shell_stdout, F_SETFD, FD_CLOEXEC);

            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }
            if (i < count - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            for (int j = 0; j < count - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            signal(SIGPIPE, SIG_DFL);
            execvp(commands[i][0], commands[i]);
            if (write(shell_stdout, "Invalid Command\n", 16) < 0) {
                _exit(127);
            }
            _exit(127);
        }
    }

    for (int i = 0; i < count - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    int result = 1;

    for (int i = 0; i < count; i++) {
        int status;

        if (pids[i] > 0 && waitpid(pids[i], &status, 0) > 0 && WIFEXITED(status)) {
            if (i == count - 1) {
                result = WEXITSTATUS(status);
            }
        }
    }

    return result;
}

// Run cd, history, or exit in the shell process itself.
int run_builtin(char *args[], int nargs)
{
    char *input_file = NULL;
    char *output_file = NULL;
    int append = 0;
    int j = 0;

    for (int i = 0; i < nargs; i++) {
        if (strcmp(args[i], "<") == 0) {
            if (input_file != NULL || i + 1 == nargs || is_redirect(args[i + 1])) {
                printf("Invalid Command\n");
                return 1;
            }
            input_file = args[++i];
        } else if (strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0) {
            if (output_file != NULL || i + 1 == nargs || is_redirect(args[i + 1])) {
                printf("Invalid Command\n");
                return 1;
            }
            if (strcmp(args[i], ">>") == 0) {
                append = 1;
            }
            output_file = args[++i];
        } else {
            args[j++] = args[i];
        }
    }
    args[j] = NULL;

    int old_input = -1;
    int old_output = -1;
    int fd;

    fflush(stdout);

    if (input_file != NULL) {
        old_input = dup(STDIN_FILENO);
        fd = open(input_file, O_RDONLY);

        if (fd < 0) {
            printf("Invalid Command\n");
            if (old_input >= 0) {
                close(old_input);
            }
            return 1;
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    if (output_file != NULL) {
        int flags = O_WRONLY | O_CREAT;
        if (append) {
            flags = flags | O_APPEND;
        } else {
            flags = flags | O_TRUNC;
        }

        old_output = dup(STDOUT_FILENO);
        fd = open(output_file, flags, 0644);

        if (fd < 0) {
            printf("Invalid Command\n");
            if (old_output >= 0) {
                close(old_output);
            }
            if (old_input >= 0) {
                dup2(old_input, STDIN_FILENO);
                close(old_input);
            }
            return 1;
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    int status;

    if (strcmp(args[0], "exit") == 0) {
        should_exit = 1;
        status = 0;
    } else if (strcmp(args[0], "history") == 0) {
        status = show_history(args, j);
    } else {
        status = change_directory(args, j);
    }

    fflush(stdout);

    if (old_input >= 0) {
        dup2(old_input, STDIN_FILENO);
        close(old_input);
    }
    if (old_output >= 0) {
        dup2(old_output, STDOUT_FILENO);
        close(old_output);
    }

    return status;
}

// Choose between a pipeline, built-in, and external command.
int execute_command(char *args[], int nargs)
{
    for (int i = 0; i < nargs; i++) {
        if (strcmp(args[i], "|") == 0)
            return run_pipeline(args, nargs);
    }

    if (strcmp(args[0], "exit") == 0 || strcmp(args[0], "history") == 0 || strcmp(args[0], "cd") == 0)
        return run_builtin(args, nargs);

    return run_external(args, nargs);
}

// Execute commands joined by ;, &&, and || from left to right.
void execute_line(char *args[], int nargs)
{
    int start = 0;
    int status = 0;
    char *previous_operator = NULL;

    for (int i = 0; i <= nargs; i++) {
        if (i == nargs || strcmp(args[i], ";") == 0 || strcmp(args[i], "&&") == 0 || strcmp(args[i], "||") == 0) {
            char *next_operator = args[i];
            args[i] = NULL;

            int run = previous_operator == NULL || strcmp(previous_operator, ";") == 0 || (strcmp(previous_operator, "&&") == 0 && status == 0) || (strcmp(previous_operator, "||") == 0 && status != 0);

            if (start == i) {
                if (i != nargs || previous_operator == NULL || strcmp(previous_operator, ";") != 0) {
                    printf("Invalid Command\n");
                    fflush(stdout);
                    status = 1;
                }
            } else if (run) {
                status = execute_command(args + start, i - start);
                fflush(stdout);
            }

            if (should_exit)
                return;

            previous_operator = next_operator;
            start = i + 1;
        }
    }
}

int main(void)
{
    char line[2050];
    char raw[2050];
    char parsed[6150];
    char *args[2050];

    if (getcwd(home_directory, sizeof(home_directory)) == NULL) {
        home_directory[0] = '\0';
    }

    while (1) {
        printf("MTL458 > ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL)
            break;

        int n = strlen(line);
        if (n > 0 && line[n - 1] == '\n')
            line[--n] = '\0';

        if (n == 0)
            continue;

        strcpy(raw, line);
        int nargs = split(line, parsed, args);
        if (nargs == 0)
            continue;

        add_history(raw);

        execute_line(args, nargs);

        if (should_exit)
            break;
    }

    return 0;
}
