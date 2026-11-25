#define _DEFAULT_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "myalloc.h"

#define REGION_SIZE 65536
#define HEADER 32
#define MIN_BLOCK 128
#define MAX_REQUEST ((size_t)1 << 30)
#define MAGIC 0xA110CA7Eu
#define MAX_REGIONS (1 << 20)
#define MAX_DELAY 64
#define BUDDY_SIZE 1048576
#define BUDDY_HEADER 16

#define ALLOCATED 1
#define FREE 2
#define DELAYED 3

typedef struct {
    uint32_t magic;
    uint32_t state;
    uint32_t size;
    uint32_t req;
    uint32_t prev_size;
    uint32_t region;
    uint64_t unused;
} block_t;

// a free block keeps two of these in its data part, one for each tree
typedef struct node {
    struct node *left, *right;
    block_t *block;
    int height;
    uint32_t max;
} node_t;

typedef struct {
    uint32_t size;
    uint32_t req;
    uint32_t state;
    uint32_t unused;
} buddy_t;

_Static_assert(sizeof(block_t) == HEADER, "block header must be 32 bytes");
_Static_assert(sizeof(buddy_t) == BUDDY_HEADER, "buddy header must be 16 bytes");
_Static_assert(HEADER + 2 * sizeof(node_t) <= MIN_BLOCK, "tree nodes must fit in a free block");

static struct {
    char *base[MAX_REGIONS];
    size_t size[MAX_REGIONS];
    int sorted[MAX_REGIONS];
} *reg;
static int nregions;

static int policy = FIRST_FIT;
static unsigned last_region;
static size_t last_offset;

// free blocks sorted by size, and sorted by heap order
static node_t *by_size;
static node_t *by_addr;

static block_t *delay_queue[MAX_DELAY + 1];
static int queue_len;
static int delay_k;

static char *buddy;
// free_bits[k] has one bit for each place a free block of size 2^k could be
static uint64_t (*free_bits)[512];

static void *get_memory(size_t size) {
    void *p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) return NULL;
    return p;
}

static size_t block_size(size_t n) {
    size_t size = (HEADER + n + 8 + 15) / 16 * 16;
    if (size < MIN_BLOCK) size = MIN_BLOCK;
    return size;
}

static char *data(block_t *b) { return (char *)b + HEADER; }
static node_t *size_node(block_t *b) { return (node_t *)data(b); }
static node_t *addr_node(block_t *b) { return (node_t *)data(b) + 1; }
static size_t offset(block_t *b) { return (char *)b - reg->base[b->region]; }

static block_t *next_block(block_t *b) {
    if (offset(b) + b->size >= reg->size[b->region]) return NULL;
    return (block_t *)((char *)b + b->size);
}

static block_t *prev_block(block_t *b) {
    if (b->prev_size == 0) return NULL;
    return (block_t *)((char *)b - b->prev_size);
}

static int comes_before(block_t *a, block_t *b) {
    if (a->region != b->region) return a->region < b->region;
    return a < b;
}

static void init_block(block_t *b, size_t size, size_t prev_size, unsigned region, int state) {
    b->magic = MAGIC;
    b->state = state;
    b->size = size;
    b->req = 0;
    b->prev_size = prev_size;
    b->region = region;
}

static int ends_after_cursor(block_t *b) {
    if (b->region != last_region) return b->region > last_region;
    return offset(b) + b->size > last_offset;
}

// AVL trees of free blocks. max is the biggest block size in the subtree.

static int height(node_t *n) { return n ? n->height : 0; }
static uint32_t max_size(node_t *n) { return n ? n->max : 0; }

static void update(node_t *n) {
    int l = height(n->left), r = height(n->right);
    n->height = (l > r ? l : r) + 1;
    n->max = n->block->size;
    if (max_size(n->left) > n->max) n->max = max_size(n->left);
    if (max_size(n->right) > n->max) n->max = max_size(n->right);
}

static node_t *rotate_right(node_t *n) {
    node_t *l = n->left;
    n->left = l->right;
    l->right = n;
    update(n);
    update(l);
    return l;
}

static node_t *rotate_left(node_t *n) {
    node_t *r = n->right;
    n->right = r->left;
    r->left = n;
    update(n);
    update(r);
    return r;
}

static node_t *balance(node_t *n) {
    update(n);
    int diff = height(n->left) - height(n->right);
    if (diff > 1) {
        if (height(n->left->right) > height(n->left->left)) n->left = rotate_left(n->left);
        return rotate_right(n);
    }
    if (diff < -1) {
        if (height(n->right->left) > height(n->right->right)) n->right = rotate_right(n->right);
        return rotate_left(n);
    }
    return n;
}

// the size tree is sorted by size then heap order, the address tree only by heap order
static int less(node_t *a, node_t *b, int sized) {
    if (sized && a->block->size != b->block->size) return a->block->size < b->block->size;
    return comes_before(a->block, b->block);
}

static node_t *tree_insert(node_t *t, node_t *n, int sized) {
    if (!t) {
        n->left = n->right = NULL;
        update(n);
        return n;
    }
    if (less(n, t, sized)) t->left = tree_insert(t->left, n, sized);
    else t->right = tree_insert(t->right, n, sized);
    return balance(t);
}

static node_t *remove_min(node_t *t, node_t **min) {
    if (!t->left) {
        *min = t;
        return t->right;
    }
    t->left = remove_min(t->left, min);
    return balance(t);
}

static node_t *tree_remove(node_t *t, node_t *n, int sized) {
    if (t == n) {
        if (!t->right) return t->left;
        node_t *min;
        node_t *right = remove_min(t->right, &min);
        min->left = t->left;
        min->right = right;
        return balance(min);
    }
    if (less(n, t, sized)) t->left = tree_remove(t->left, n, sized);
    else t->right = tree_remove(t->right, n, sized);
    return balance(t);
}

// every free block is in both trees. don't change b->size while it is in them.
static void add_free(block_t *b) {
    size_node(b)->block = b;
    addr_node(b)->block = b;
    by_size = tree_insert(by_size, size_node(b), 1);
    by_addr = tree_insert(by_addr, addr_node(b), 0);
}

static void remove_free(block_t *b) {
    by_size = tree_remove(by_size, size_node(b), 1);
    by_addr = tree_remove(by_addr, addr_node(b), 0);
}

// smallest block with size >= need, earliest one if there is a tie
static block_t *best_fit(size_t need) {
    node_t *t = by_size;
    block_t *found = NULL;
    while (t) {
        if (t->block->size >= need) {
            found = t->block;
            t = t->left;
        } else {
            t = t->right;
        }
    }
    return found;
}

// earliest block in the address tree t with size >= need
static block_t *first_fit(node_t *t, size_t need) {
    while (t && t->max >= need) {
        if (max_size(t->left) >= need) t = t->left;
        else if (t->block->size >= need) return t->block;
        else t = t->right;
    }
    return NULL;
}

// same as first_fit but only blocks that end after the next-fit position
static block_t *next_fit(node_t *t, size_t need) {
    if (!t || t->max < need) return NULL;
    if (!ends_after_cursor(t->block)) return next_fit(t->right, need);
    block_t *b = next_fit(t->left, need);
    if (b) return b;
    if (t->block->size >= need) return t->block;
    return first_fit(t->right, need);
}

static block_t *new_region(size_t need) {
    if (!reg) {
        reg = get_memory(sizeof(*reg));
        if (!reg) return NULL;
    }
    if (nregions == MAX_REGIONS) return NULL;
    size_t size = (need + 32 + REGION_SIZE - 1) / REGION_SIZE * REGION_SIZE;
    char *base = get_memory(size);
    if (!base) return NULL;

    int r = nregions++;
    reg->base[r] = base;
    reg->size[r] = size;
    // sorted keeps regions in address order so find_block can binary search
    int i = r;
    while (i > 0 && (uintptr_t)reg->base[reg->sorted[i - 1]] > (uintptr_t)base) {
        reg->sorted[i] = reg->sorted[i - 1];
        i--;
    }
    reg->sorted[i] = r;

    block_t *b = (block_t *)(base + 32);
    init_block(b, size - 32, 0, r, FREE);
    add_free(b);
    return b;
}

// shrink b to size and return the leftover as a free block (not added to the trees yet)
static block_t *split(block_t *b, size_t size) {
    if (b->size - size < MIN_BLOCK) return NULL;
    block_t *rest = (block_t *)((char *)b + size);
    init_block(rest, b->size - size, size, b->region, FREE);
    b->size = size;
    block_t *after = next_block(rest);
    if (after) after->prev_size = rest->size;
    return rest;
}

// really free a block and merge it with free neighbours
static void release(block_t *b) {
    b->state = FREE;
    b->req = 0;
    block_t *next = next_block(b);
    if (next && next->state == FREE) {
        remove_free(next);
        b->size += next->size;
        next->magic = 0;
    }
    block_t *prev = prev_block(b);
    if (prev && prev->state == FREE) {
        remove_free(prev);
        prev->size += b->size;
        b->magic = 0;
        b = prev;
    }
    next = next_block(b);
    if (next) next->prev_size = b->size;
    add_free(b);
}

static block_t *choose(size_t need) {
    if (policy == BEST_FIT) return best_fit(need);
    if (policy == WORST_FIT) {
        // the earliest block whose size is the biggest size
        if (max_size(by_addr) < need) return NULL;
        return first_fit(by_addr, by_addr->max);
    }
    if (policy == NEXT_FIT) {
        block_t *b = next_fit(by_addr, need);
        if (b) return b;
    }
    return first_fit(by_addr, need);
}

static void set_guard(block_t *b) {
    memset(data(b) + b->req, 0xCA, 8);
}

static void check_guard(block_t *b) {
    unsigned char *guard = (unsigned char *)data(b) + b->req;
    for (int i = 0; i < 8; i++) {
        if (guard[i] != 0xCA) {
            printf("error: overflow detected (block %u:%zu)\n", b->region, offset(b));
            return;
        }
    }
}

// returns the block whose data starts at p, or NULL
static block_t *find_block(void *p) {
    if (nregions == 0) return NULL;
    uintptr_t addr = (uintptr_t)p;
    int lo = 0, hi = nregions - 1, r = -1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if ((uintptr_t)reg->base[reg->sorted[mid]] <= addr) {
            r = reg->sorted[mid];
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    if (r < 0) return NULL;
    size_t off = addr - (uintptr_t)reg->base[r];
    if (off >= reg->size[r] || off < 32 + HEADER || off % 16 != 0) return NULL;
    block_t *b = (block_t *)(reg->base[r] + off - HEADER);
    if (b->magic != MAGIC || b->region != (unsigned)r) return NULL;
    return b;
}

// prints the error and returns NULL if p is not an allocated block
static block_t *check_pointer(void *p) {
    block_t *b = find_block(p);
    if (!b) {
        printf("error: invalid pointer\n");
        return NULL;
    }
    if (b->state != ALLOCATED) {
        printf("error: double free (block %u:%zu)\n", b->region, offset(b));
        return NULL;
    }
    return b;
}

static void trim_queue(void) {
    while (queue_len > delay_k) {
        block_t *b = delay_queue[0];
        for (int i = 1; i < queue_len; i++) delay_queue[i - 1] = delay_queue[i];
        queue_len--;

        unsigned char *d = (unsigned char *)data(b);
        for (size_t i = 0; i < b->req; i++) {
            if (d[i] != 0xDE) {
                printf("error: write after free (block %u:%zu)\n", b->region, offset(b));
                break;
            }
        }
        release(b);
    }
}

static void free_block(block_t *b) {
    if (delay_k == 0) {
        release(b);
        return;
    }
    memset(data(b), 0xDE, b->req);
    b->state = DELAYED;
    delay_queue[queue_len++] = b;
    trim_queue();
}

// buddy allocator, it has its own 1 MB region

static buddy_t *bud(size_t off) { return (buddy_t *)(buddy + off); }

static int in_buddy(void *p) {
    uintptr_t addr = (uintptr_t)p;
    return buddy && addr >= (uintptr_t)buddy && addr < (uintptr_t)buddy + BUDDY_SIZE;
}

static int order_of(size_t size) {
    int k = 0;
    while (((size_t)1 << k) < size) k++;
    return k;
}

static void set_free_bit(size_t off, size_t size, int on) {
    int k = order_of(size);
    size_t i = off >> k;
    if (on) free_bits[k][i / 64] |= (uint64_t)1 << (i % 64);
    else free_bits[k][i / 64] &= ~((uint64_t)1 << (i % 64));
}

// lowest offset of a free block of size 2^k, or BUDDY_SIZE if there is none
static size_t lowest_free(int k) {
    size_t words = ((BUDDY_SIZE >> k) + 63) / 64;
    for (size_t w = 0; w < words; w++) {
        if (free_bits[k][w]) return (w * 64 + __builtin_ctzll(free_bits[k][w])) << k;
    }
    return BUDDY_SIZE;
}

static void *buddy_malloc(size_t n) {
    if (n > BUDDY_SIZE - BUDDY_HEADER) return NULL;
    if (!buddy) {
        free_bits = get_memory(21 * sizeof(*free_bits));
        if (!free_bits) return NULL;
        buddy = get_memory(BUDDY_SIZE);
        if (!buddy) return NULL;
        bud(0)->size = BUDDY_SIZE;
        bud(0)->state = FREE;
        set_free_bit(0, BUDDY_SIZE, 1);
    }
    size_t s = 32;
    while (s < n + BUDDY_HEADER) s *= 2;

    size_t off = BUDDY_SIZE;
    for (int k = order_of(s); k <= 20 && off == BUDDY_SIZE; k++) off = lowest_free(k);
    if (off == BUDDY_SIZE) return NULL;

    set_free_bit(off, bud(off)->size, 0);
    while (bud(off)->size > s) {
        uint32_t half = bud(off)->size / 2;
        bud(off)->size = half;
        bud(off + half)->size = half;
        bud(off + half)->req = 0;
        bud(off + half)->state = FREE;
        set_free_bit(off + half, half, 1);
    }
    bud(off)->state = ALLOCATED;
    bud(off)->req = n;
    return buddy + off + BUDDY_HEADER;
}

static void buddy_free(void *p) {
    size_t off = (uintptr_t)p - (uintptr_t)buddy - BUDDY_HEADER;
    if (off >= BUDDY_SIZE || off % 32 != 0 || (bud(off)->state != ALLOCATED && bud(off)->state != FREE)) {
        printf("error: invalid pointer\n");
        return;
    }
    if (bud(off)->state == FREE) {
        printf("error: double free (block 0:%zu)\n", off);
        return;
    }
    bud(off)->state = FREE;
    bud(off)->req = 0;
    size_t size = bud(off)->size;
    while (size < BUDDY_SIZE) {
        size_t other = off ^ size;
        if (bud(other)->state != FREE || bud(other)->size != size) break;
        set_free_bit(other, size, 0);
        // the higher half's header is not a block anymore
        if (other < off) {
            bud(off)->state = 0;
            off = other;
        } else {
            bud(other)->state = 0;
        }
        size *= 2;
        bud(off)->size = size;
    }
    set_free_bit(off, size, 1);
}

void *my_malloc(size_t n) {
    if (n == 0 || n > MAX_REQUEST) return NULL;
    if (policy == BUDDY) return buddy_malloc(n);

    size_t need = block_size(n);
    block_t *b = choose(need);
    if (!b) b = new_region(need);
    if (!b) return NULL;

    remove_free(b);
    block_t *rest = split(b, need);
    if (rest) add_free(rest);
    b->state = ALLOCATED;
    b->req = n;
    set_guard(b);
    last_region = b->region;
    last_offset = offset(b) + b->size;
    return data(b);
}

void my_free(void *ptr) {
    if (!ptr) return;
    if (in_buddy(ptr)) {
        buddy_free(ptr);
        return;
    }
    block_t *b = check_pointer(ptr);
    if (!b) return;
    check_guard(b);
    free_block(b);
}

void *my_calloc(size_t nmemb, size_t size) {
    if (nmemb == 0 || size == 0 || nmemb > SIZE_MAX / size) return NULL;
    size_t total = nmemb * size;
    if (total > MAX_REQUEST) return NULL;
    void *p = my_malloc(total);
    if (p) memset(p, 0, total);
    return p;
}

void *my_realloc(void *ptr, size_t n) {
    if (!ptr) return my_malloc(n);
    block_t *b = check_pointer(ptr);
    if (!b) return NULL;
    check_guard(b);
    if (n == 0) {
        free_block(b);
        return NULL;
    }
    if (n > MAX_REQUEST) return NULL;

    size_t need = block_size(n);
    if (need <= b->size) {
        block_t *rest = split(b, need);
        if (rest) release(rest);
        b->req = n;
        set_guard(b);
        return ptr;
    }

    block_t *next = next_block(b);
    if (next && next->state == FREE && b->size + next->size >= need) {
        remove_free(next);
        b->size += next->size;
        next->magic = 0;
        block_t *after = next_block(b);
        if (after) after->prev_size = b->size;
        block_t *rest = split(b, need);
        if (rest) add_free(rest);
        b->req = n;
        set_guard(b);
        return ptr;
    }

    void *p = my_malloc(n);
    if (!p) return NULL;
    memcpy(p, ptr, b->req);
    free_block(b);
    return p;
}

void my_set_policy(int p) {
    if (p >= FIRST_FIT && p <= BUDDY) policy = p;
}

void my_set_delay(int k) {
    if (k < 0) k = 0;
    if (k > MAX_DELAY) k = MAX_DELAY;
    delay_k = k;
    trim_queue();
}

static void print_block(size_t off, int state, size_t size, size_t req) {
    if (state == ALLOCATED) printf("  %zu A %zu %zu\n", off, size, req);
    else if (state == DELAYED) printf("  %zu D %zu %zu\n", off, size, req);
    else printf("  %zu F %zu\n", off, size);
}

void my_heap_dump(void) {
    for (int r = 0; r < nregions; r++) {
        printf("region %d %zu\n", r, reg->size[r]);
        for (block_t *b = (block_t *)(reg->base[r] + 32); b; b = next_block(b))
            print_block(offset(b), b->state, b->size, b->req);
    }
    if (buddy) {
        printf("buddy %d\n", BUDDY_SIZE);
        for (size_t off = 0; off < BUDDY_SIZE; off += bud(off)->size)
            print_block(off, bud(off)->state, bud(off)->size, bud(off)->req);
    }
}

void my_leak_report(void) {
    size_t count = 0, bytes = 0;
    for (int r = 0; r < nregions; r++) {
        for (block_t *b = (block_t *)(reg->base[r] + 32); b; b = next_block(b)) {
            if (b->state == ALLOCATED) {
                printf("leak: block %d:%zu (%u bytes)\n", r, offset(b), b->req);
                count++;
                bytes += b->req;
            }
        }
    }
    printf("leak summary: %zu blocks, %zu bytes\n", count, bytes);
}
