import sys
import os

class BFSyntaxError(Exception):
    pass

# Cell memory object, implements operations on memory, including I/O
class MemState:
    def __init__(self):
        self.mem = []
        self.memOffset = 0
        self.pointer = 0
        self.mem_init()
    def mem_init(self):
        if self.pointer+self.memOffset < 0:
            self.mem.insert(0, 0)
            self.memOffset += 1
        try:
            self.get()
        except IndexError:
            self.mem.append(0)
    def get(self):
        return self.mem[self.pointer+self.memOffset]
    def ml(self):
        self.pointer -= 1
        self.mem_init()
    def mr(self):
        self.pointer += 1
        self.mem_init()
    def add(self):
        self.mem[self.pointer+self.memOffset] += 1
    def sub(self):
        self.mem[self.pointer+self.memOffset] -= 1
    def out(self):
        print(chr(self.get()), end='')
        return -1
    def inp(self):
        return input()
    def print_mem(self):
        print(self.mem)

# Interpret BF string code, with a given memory state
def interpret_bf(s, state=MemState()):
    i = 0               # Interpreter index in BF string
    printed = False     # Will be set to True if a character is ever printed during execution (whether to print newline after execution or not)
    brace_map = {}      # Set of index->index values linking corresponding braces' indexes
    op_count = 0        # Number of BF operations executed (loop cycles, excluding non-BF characters)

    # Build brace_map
    if s.count('[') != s.count(']'):
        raise BFSyntaxError('Non-closed loop')
    index_list = []
    for k, c in enumerate(s):
        if c == '[':
            index_list.append(k)
        elif c == ']':
            try:
                v = index_list.pop()
                brace_map[k] = v
                brace_map[v] = k
            except IndexError:
                raise BFSyntaxError('Non-matching loop characters')
    del index_list

    # Program flow operations
    def ls():
        if state.get() == 0:
            return brace_map[i] + 1
        else:
            return None
    def le():
        if state.get() == 0:
            return None
        else:
            return brace_map[i] + 1

    # Main interpreter loop
    while i < len(s):
        try:
            r = {'<': state.ml, '>': state.mr, '+': state.add, '-': state.sub, '.': state.out, ',': state.inp, '[': ls, ']': le}[s[i]]()
            if r == -1:
                printed = True
                i += 1
            elif r:
                i = r
            else:
                i += 1
            op_count += 1
        except KeyError:
            i += 1
    if printed:
        print()
    return op_count

def main():
    interactive = False
    try:
        arg = sys.argv[1]
    except IndexError:
        interactive = True
    if interactive:
        state = MemState()
        while True:
            s = input('~#')
            if s == '!':
                break
            elif s == '$':
                state.print_mem()
                continue
            elif s == '@':
                state = MemState()
            while s.count('[') > s.count(']'):
                s += input('~~')
            try:
                interpret_bf(s, state)
            except BFSyntaxError:
                print('Invalid syntax')
    else:
        f = None
        try:
            f = open(arg, 'r')
            op_count = 0
            if os.path.getsize(arg) <= 1024:     # Read whole file if its size is less than or equal to 1KB
                op_count = interpret_bf(f.read())
            else:                                # If size of file is higher than 1KB, read file by chunks of 1KB + [necessary to include all matching brackets]
                state = MemState()
                s = f.read(1024)
                while s:
                    while s.count('[') > s.count(']'):
                        c = f.read(1)
                        s += c
                        if not c:
                            break
                    op_count += interpret_bf(s, state)
            print('Executed', op_count, 'operations.')
            f.close()
        except FileNotFoundError:
            print('File not found')
        

if __name__ == '__main__':
    main()