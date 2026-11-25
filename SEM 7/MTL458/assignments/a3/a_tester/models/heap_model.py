"""A deliberately simple reference model of the assignment's allocator.

It uses plain Python lists and linear scans, written straight from the PDF,
so it is easy to check by reading. generate() makes a random sequence of
operations together with the exact output a correct a3.c must print for it.
The operations are run by c/driver.c.

Operations (one per line):
    m <id> <n>        p[id] = my_malloc(n)
    c <id> <a> <b>    p[id] = my_calloc(a, b)       (driver checks zeroing)
    r <id> <n>        p[id] = my_realloc(p[id], n)  (driver checks data kept)
    f <id>            my_free(p[id])                (p[id] may be stale)
    x <id>            my_free(p[id] + 16)           (never a block start)
    s                 my_free(<stack address>)
    o <id>            write into a guard byte of p[id]
    w <id>            write into the data of delayed block p[id]
    p <k> / k <k>     my_set_policy(k) / my_set_delay(k)
    d / l             my_heap_dump() / my_leak_report()

Points the PDF leaves open are kept out of the generated sequences, so any
mismatch is a real disagreement with the spec:
  * the first operation is always a small valid malloc (region 0 size and
    whether failed requests create it are open questions);
  * realloc(p, n > 2^30) is only issued while p's guard bytes are intact;
  * with allow_realloc=False no realloc happens at all, which is used for
    sequences that include next fit (whether an in-place realloc moves the
    next-fit position is an open question);
  * delays stay within 0..64.
"""
import random

MAX_REQUEST = 1 << 30


def block_size(n):
    return max(128, (32 + n + 8 + 15) // 16 * 16)


class Heap:
    def __init__(self):
        self.regions = []          # each: {'size': int, 'blocks': [block dicts in offset order]}
        self.policy = 0
        self.next_pos = (0, 0)     # (region, offset)
        self.delay = 0
        self.queue = []            # (region, offset), oldest first
        self.out = []

    # ---- blocks ----
    def find(self, r, off):
        if r >= len(self.regions):
            return None, None
        for i, b in enumerate(self.regions[r]['blocks']):
            if b['off'] == off:
                return i, b
        return None, None

    def all_blocks(self):
        for r, reg in enumerate(self.regions):
            for b in reg['blocks']:
                yield r, b

    def coalesce(self, r, i):
        bl = self.regions[r]['blocks']
        if i + 1 < len(bl) and bl[i + 1]['st'] == 'F':
            bl[i]['size'] += bl[i + 1]['size']
            del bl[i + 1]
        if i > 0 and bl[i - 1]['st'] == 'F':
            bl[i - 1]['size'] += bl[i]['size']
            del bl[i]

    def make_free(self, r, off):
        i, b = self.find(r, off)
        b['st'] = 'F'
        b['n'] = 0
        self.coalesce(r, i)

    def split(self, r, i, keep):
        bl = self.regions[r]['blocks']
        b = bl[i]
        rest = {'off': b['off'] + keep, 'size': b['size'] - keep, 'st': 'F', 'n': 0,
                'guard_bad': False, 'written': False}
        b['size'] = keep
        bl.insert(i + 1, rest)
        self.coalesce(r, i + 1)

    def new_region(self, size):
        self.regions.append({'size': size, 'blocks': [
            {'off': 32, 'size': size - 32, 'st': 'F', 'n': 0, 'guard_bad': False, 'written': False}]})

    # ---- allocation ----
    def choose(self, need):
        fits = [(r, b) for r, b in self.all_blocks() if b['st'] == 'F' and b['size'] >= need]
        if not fits:
            return None
        if self.policy == 0:                                   # first fit
            return fits[0]
        if self.policy == 1:                                   # next fit
            after = [(r, b) for r, b in fits if (r, b['off'] + b['size']) > self.next_pos]
            return after[0] if after else fits[0]
        if self.policy == 2:                                   # best fit
            smallest = min(b['size'] for _, b in fits)
            return next(c for c in fits if c[1]['size'] == smallest)
        free = [(r, b) for r, b in self.all_blocks() if b['st'] == 'F']   # worst fit
        largest = max(b['size'] for _, b in free)
        return next(c for c in free if c[1]['size'] == largest) if largest >= need else None

    def allocate(self, n):
        need = block_size(n)
        if not self.regions:
            self.new_region(65536)
        pick = self.choose(need)
        if pick is None:
            self.new_region((need + 32 + 65535) // 65536 * 65536)
            pick = (len(self.regions) - 1, self.regions[-1]['blocks'][0])
        r, b = pick
        b.update(st='A', n=n, guard_bad=False, written=False)
        i, _ = self.find(r, b['off'])
        if b['size'] - need >= 128:
            self.split(r, i, need)
        self.next_pos = (r, b['off'] + b['size'])
        return r, b['off']

    # ---- freeing ----
    def lookup(self, addr):
        """The allocated block at addr, or None after printing the error."""
        r, off = addr
        _, b = self.find(r, off)
        if b is None:
            self.out.append('error: invalid pointer')
            return None
        if b['st'] != 'A':
            self.out.append(f'error: double free (block {r}:{off})')
            return None
        return b

    def check_guard(self, r, b):
        if b['guard_bad']:
            self.out.append(f"error: overflow detected (block {r}:{b['off']})")

    def evict(self):
        r, off = self.queue.pop(0)
        _, b = self.find(r, off)
        if b['written']:
            self.out.append(f'error: write after free (block {r}:{off})')
        self.make_free(r, off)

    def release(self, r, b):
        if self.delay == 0:
            self.make_free(r, b['off'])
            return
        b['st'] = 'D'
        b['written'] = False
        self.queue.append((r, b['off']))
        while len(self.queue) > self.delay:
            self.evict()

    def set_delay(self, k):
        self.delay = k
        while len(self.queue) > self.delay:
            self.evict()

    # ---- output ----
    def dump(self):
        for r, reg in enumerate(self.regions):
            self.out.append(f"region {r} {reg['size']}")
            for b in reg['blocks']:
                if b['st'] == 'F':
                    self.out.append(f"  {b['off']} F {b['size']}")
                else:
                    self.out.append(f"  {b['off']} {b['st']} {b['size']} {b['n']}")

    def leak_report(self):
        count = total = 0
        for r, b in self.all_blocks():
            if b['st'] == 'A':
                self.out.append(f"leak: block {r}:{b['off']} ({b['n']} bytes)")
                count += 1
                total += b['n']
        self.out.append(f'leak summary: {count} blocks, {total} bytes')


def generate(seed, nops=400, allow_next_fit=True, allow_realloc=True):
    """Return (ops, expected_output_lines) for one random test."""
    rng = random.Random(seed)
    h = Heap()
    ops = []
    addr = {}          # id -> (region, offset) the driver's pointer refers to
    alive = set()      # ids whose block the driver owns
    next_id = 0
    policies = [0, 1, 2, 3] if allow_next_fit else [0, 2, 3]

    def rand_n():
        x = rng.random()
        if x < 0.6:
            return rng.randint(1, 300)
        if x < 0.9:
            return rng.randint(300, 5000)
        if x < 0.98:
            return rng.randint(5000, 70000)
        return rng.randint(70000, 300000)

    def aliased(i):
        """A stale id whose address now holds someone else's allocated block."""
        if i in alive:
            return False
        _, b = h.find(*addr[i])
        return b is not None and b['st'] == 'A'

    # first operation: a small valid malloc (see module docstring)
    ops.append(f'm {next_id} 10')
    addr[next_id] = h.allocate(10)
    alive.add(next_id)
    next_id += 1

    for _ in range(nops):
        x = rng.random()
        ids = list(addr)
        live = sorted(alive)
        if x < 0.30:
            n = rand_n()
            if rng.random() < 0.02:
                n = rng.choice([0, MAX_REQUEST + 1])
            i = next_id
            next_id += 1
            ops.append(f'm {i} {n}')
            if n == 0 or n > MAX_REQUEST:
                h.out.append('NULL')
            else:
                addr[i] = h.allocate(n)
                alive.add(i)
        elif x < 0.34:
            a, b = rng.randint(0, 40), rng.randint(0, 40)
            i = next_id
            next_id += 1
            ops.append(f'c {i} {a} {b}')
            if a * b == 0:
                h.out.append('NULL')
            else:
                addr[i] = h.allocate(a * b)
                alive.add(i)
        elif x < 0.58 and live:
            i = rng.choice(live)
            ops.append(f'f {i}')
            r = addr[i][0]
            b = h.lookup(addr[i])
            h.check_guard(r, b)
            h.release(r, b)
            alive.discard(i)
        elif x < 0.75 and allow_realloc:
            i = rng.choice(live) if live and rng.random() < 0.9 else rng.choice(ids)
            if aliased(i):
                continue
            n = rand_n()
            if rng.random() < 0.05:
                n = 0
            if rng.random() < 0.02 and i in alive:
                _, blk = h.find(*addr[i])
                if not blk['guard_bad']:
                    n = MAX_REQUEST + 5
            ops.append(f'r {i} {n}')
            b = h.lookup(addr[i])
            if b is None:
                h.out.append('NULL')
                continue
            r = addr[i][0]
            h.check_guard(r, b)
            if n == 0:
                h.release(r, b)
                alive.discard(i)
                h.out.append('NULL')
                continue
            if n > MAX_REQUEST:
                h.out.append('NULL')
                continue
            need, size = block_size(n), b['size']
            idx, _ = h.find(r, b['off'])
            bl = h.regions[r]['blocks']
            if need <= size:                                            # shrink
                if size - need >= 128:
                    h.split(r, idx, need)
            elif idx + 1 < len(bl) and bl[idx + 1]['st'] == 'F' and size + bl[idx + 1]['size'] >= need:
                b['size'] += bl[idx + 1]['size']                        # grow
                del bl[idx + 1]
                if b['size'] - need >= 128:
                    h.split(r, idx, need)
            else:                                                       # move
                new = h.allocate(n)
                h.release(r, b)
                addr[i] = new
                continue
            b['n'] = n
            b['guard_bad'] = False
        elif x < 0.80 and ids:
            i = rng.choice(ids)
            if aliased(i):
                continue
            ops.append(f'f {i}')
            r = addr[i][0]
            b = h.lookup(addr[i])
            if b:
                h.check_guard(r, b)
                h.release(r, b)
            alive.discard(i)
        elif x < 0.82 and ids:
            i = rng.choice(ids)
            r, off = addr[i]
            if h.find(r, off + 16)[1] is not None:
                continue
            ops.append(f'x {i}')
            h.out.append('error: invalid pointer')
        elif x < 0.83:
            ops.append('s')
            h.out.append('error: invalid pointer')
        elif x < 0.86 and live:
            i = rng.choice(live)
            ops.append(f'o {i}')
            h.find(*addr[i])[1]['guard_bad'] = True
        elif x < 0.88 and h.queue:
            r, off = rng.choice(h.queue)
            owners = [i for i in ids if addr[i] == (r, off) and i not in alive]
            if owners:
                ops.append(f'w {owners[0]}')
                h.find(r, off)[1]['written'] = True
        elif x < 0.92:
            h.policy = rng.choice(policies)
            ops.append(f'p {h.policy}')
        elif x < 0.94:
            k = rng.choice([0, 0, 1, 2, 3, 8, 64])
            ops.append(f'k {k}')
            h.set_delay(k)
        elif x < 0.97:
            ops.append('d')
            h.dump()
        else:
            ops.append('l')
            h.leak_report()
    ops.append('d')
    h.dump()
    ops.append('l')
    h.leak_report()
    return ops, h.out


def generate_buddy(seed, nops=600):
    """Random buddy-allocator test: (ops, expected_output_lines)."""
    rng = random.Random(seed)
    ops, out = ['p 4'], []
    blocks = None                      # [offset, size, state, n] in offset order
    addr, alive, next_id = {}, set(), 0

    def find(off):
        for i, b in enumerate(blocks):
            if b[0] == off:
                return i
        return None

    def dump():
        if blocks:
            out.append('buddy 1048576')
            for b in blocks:
                out.append(f'  {b[0]} F {b[1]}' if b[2] == 'F' else f'  {b[0]} A {b[1]} {b[3]}')

    for _ in range(nops):
        x = rng.random()
        if x < 0.55 or not alive:
            n = rng.choice([rng.randint(1, 100), rng.randint(1, 3000),
                            rng.randint(1, 100000), rng.randint(0, 1048576)])
            i = next_id
            next_id += 1
            ops.append(f'm {i} {n}')
            if n == 0 or n + 16 > 1048576:
                out.append('NULL')
                continue
            if blocks is None:
                blocks = [[0, 1048576, 'F', 0]]
            s = 32
            while s < n + 16:
                s *= 2
            fits = [b for b in blocks if b[2] == 'F' and b[1] >= s]
            if not fits:
                out.append('NULL')
                continue
            smallest = min(b[1] for b in fits)
            b = min((b for b in fits if b[1] == smallest), key=lambda b: b[0])
            k = find(b[0])
            while b[1] > s:
                half = b[1] // 2
                b[1] = half
                blocks.insert(k + 1, [b[0] + half, half, 'F', 0])
            b[2], b[3] = 'A', n
            addr[i] = b[0]
            alive.add(i)
        elif x < 0.95:
            i = rng.choice(sorted(alive))
            ops.append(f'f {i}')
            alive.discard(i)
            k = find(addr[i])
            blocks[k][2], blocks[k][3] = 'F', 0
            while blocks[k][1] < 1048576:
                off, size = blocks[k][0], blocks[k][1]
                j = find(off ^ size)
                if j is None or blocks[j][2] != 'F' or blocks[j][1] != size:
                    break
                k = min(k, j)
                blocks[k][1] = size * 2
                del blocks[k + 1]
        else:
            ops.append('d')
            dump()
    ops.append('d')
    dump()
    return ops, out
