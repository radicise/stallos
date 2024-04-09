"""
checks for leaks
"""

from typing import Union, Self

from sys import argv
# import sys
# argv = sys.argv
from os import path
import re
import traceback

argc = len(argv)

checked: list[str] = []

resalloc: list[re.Pattern] = []
resdeloc: list[re.Pattern] = []

omitpat: re.Pattern = None

Line = tuple[int, str]

class BlockHeader():
    IF: int = 0
    ELIF: int = 1
    ELSE: int = 2
    FOR: int = 3
    WHILE: int = 4
    SWITCH: int = 5
    CASE: int = 6
    DEFAULT: int = 7
    FUNC: int = 8
    def __init__(self, k: int, l: Line, s: str) -> None:
        self.kind = k # type of block
        self.line = l # originating line
        self.src = s # originating file

Tree = list[Union[Line, list]]
Scope = list[Line]

StackFrame = tuple[str, int, Scope]

Violation = tuple[Line, str, Scope]

BLOCKS = tuple("for,while,if,else,else if,switch".split(","))

FuncStats = list[bool]
FuncReg = tuple[bool, Tree, FuncStats]

FUNCS: dict[str, FuncReg] = {}

class Tracked():
    ALIAS: int = 0 # eg: void* a = b;
    RESOR: int = 1
    def __init__(self, k: int, d: Line, n: str, v: Scope) -> None:
        self.kind: int = k # what kind of resource
        self.decl: Line = d # where it was declared
        self.name: str = n # name of resource
        self.vald: Scope = v # scope the resource is valid in
    def resolveFull(self) -> Self:
        """
        returns whatever instance of kind [RESOR] this object refers to
        """
        c = self
        while c.kind == Tracked.ALIAS:
            c = FUNCS[...]
    def isSame(self, other: Self) -> bool:
        ...

Tracking: dict[str, Tracked]

def make_tree(lines: list[Line]) -> list[Line,list]:
    f = []
    ca = [f]
    ig = 0
    i = 0
    le = len(lines)
    while i < le:
        line = lines[i][1].strip()
        if "//" in line:
            line = line[:line.index("//")].strip()
            if len(line) == 0:
                i += 1
                continue
        if "/*" in line:
            if "*/" in line:
                line = (line[:line.index("/*")] + line[line.rindex("*/")+2:]).strip()
                if len(line) == 0:
                    i += 1
                    continue
            else:
                while "*/" not in lines[i][1]:
                    i += 1
                line = lines[i][1][lines[i][1].rindex("*/")+2:].strip()
                if len(line) == 0:
                    i += 1
                    continue
        if (line.endswith("{")):
            if ("struct" not in line and "=" not in line):
                l = [(lines[i][0],line)]
                ca[-1].append(l)
                ca.append(l)
            else:
                ig += 1
        elif (len(ca) > 1 and line.endswith("}")):
            ca[-1].append((lines[i][0],line))
            if ig > 0:
                ig -= 1
            else:
                ca.pop()
        else:
            ca[-1].append((lines[i][0],line))
        i += 1
    return f

def fmtl(l: list) -> str:
    sb = ""
    for i in l:
        sb += repr(i) + "\n"
    return sb
    # return l.replace('), (','),\n(').replace('], (','],\n(').replace('), [','),\n[').replace('], [','],\n[')

def fmts(ls: list[Line]) -> str:
    # print(ls)
    sb = f"{ls[0][1]}"
    for l in ls[1:]:
        lt = l[1]
        if lt[0] == "}":
            lt = lt[1:].strip()
        if lt.startswith(("switch", "while", "for", "if", "else")):
            if lt.startswith("else if"):
                sb += f"/elseif@{{{l[0]}}}"
                continue
            c = lt.find(' ')
            cc = lt.find('(')
            ccc = lt.find('{')
            if c < 0:
                c = 100000
            if cc < 0:
                c = 100000
            if ccc < 0:
                c = 100000
            c = min(c, min(cc, ccc))
            if c == 100000:
                c = 0
            if lt.startswith("else"):
                c -= 1
            sb += f"/{lt[:c]}@{{{l[0]}}}"
        else:
            # print(lt)
            # sb += f"/{lt[lt.index(' ')+1:lt.rindex(')')+1]}"
            try:
                sb += f"/{lt[lt.index(' ')+1:lt.index('(')]}"
            except Exception as E:
                print(f"ERROR ON {{\"{lt}\"}}")
                print(*ls, sep='\n')
                raise E
    return sb

class Resource():
    def __init__(self, enscope):
        self.enscope: list[Line] = enscope
        self.stack: list[StackFrame] = []
        self.errbuildup = []
        self.log: list[tuple[int, Line, StackFrame]] = []
    def branch(self, l):
        self.stack.append([l,0,[]])
    def release(self, l):
        self.stack[-1][1] += 1
        self.stack[-1][2].append(l)
    def escape(self, l):
        self.log.append((0, l, self.stack.pop()))
    def join(self, l):
        n = self.stack.pop()
        self.stack[-1][1] += n[1]
        self.log.append((1, l, n))
    def __repr__(self) -> str:
        return f"RESOURCE @ {{{fmts(self.enscope)}}}\n{fmtl(self.log)}"

def checkalloc(l: str) -> bool:
    global resalloc
    for p in resalloc:
        if p.search(l) != None:
            return True
    return False

def checkdeloc(l: str) -> bool:
    global resdeloc
    for p in resdeloc:
        if p.search(l) != None:
            return True
    return False

def generate_resource_name(scope: list[Line], name: str) -> str:
    return f"{fmts([scope[0], scope[-1]])}::{name}"

def mk_full_name(scope: list[Line], name: str) -> str:
    return f"{fmts(scope)}::{name}"

def extract_name(l: str) -> str:
    return l.strip().split()[1].strip().split("=")[0]

def resolve_resource_name(scope: list[Line], track: dict[str, Resource], name: str) -> str:
    k = track.keys()
    for i in range(1, len(scope)-1):
        # s = generate_resource_name(scope[:-i], name)
        s = mk_full_name(scope, name)
        if not scope[-i][1].startswith(BLOCKS):
            s = generate_resource_name(scope[:-i], name)
        if s in k:
            return s
    return None

def extract_resource_names(scope: list[Line], track: dict[str, Resource], line: str) -> list[str]:
    line = line[line.index("(")+1:line.rindex(")")] # grab args
    l = list(map(str.strip, line.split(",")))
    r = []
    for i in l:
        i = i.strip()
        s = resolve_resource_name(scope, track, i)
        if s != None:
            r.append(s)
    return r

def obtain_tracking(line: str, track: dict[str, Resource], scope: list[Line]) -> list[str]:
    if line.startswith(("if", "else", "for", "while", "switch")):
        return []
    # print("LINE", line, not line.startswith(BLOCKS))
    line = line[line.index("(")+1:line.rindex(")")] # grab args
    l = list(map(str.strip, line.split(",")))
    r = []
    for j, i in enumerate(l):
        s = i.rsplit(" ", 1)
        s[0] = s[0].strip()
        s[1] = s[1].strip()
        if s[0][-1] == '*':
            res = generate_resource_name(scope, s[1])
            track[res] = Resource(scope)
            track[res].branch(scope[-1])
            r.append(res)
    return r

def check_block(lines: list[Line], track: dict[str, Resource], scope: list[Line]):
    scope.append(lines[0])
    try:
        rest = obtain_tracking(lines[0][1], track, scope)
    except Exception as E:
        print("\x1b[38;2;255;0;0mERROR SOURCE:\x1b[0m", lines[0][1])
        raise E
    # print(rest)
    for i in range(1, len(lines)):
        if type(lines[i]) == list:
            try:
                check_block(lines[i], track, scope)
            except Exception as E:
                print(track, sep='\n')
                # print(lines[max(0,i-4):i])
                # print(lines[i][0])
                # print(fmts(scope))
                print(*scope, sep='\n')
                traceback.print_exc()
                exit(1)
        else:
            l = lines[i][1]
            if checkalloc(l):
                # print(lines[i])
                # sc = fmts(scope)
                sc = mk_full_name(scope, extract_name(l))
                track[sc] = Resource(scope.copy())
                track[sc].branch(lines[i])
            elif checkdeloc(l):
                # print(lines[i])
                try:
                    nl = extract_resource_names(scope, track, l)
                    # fstats: FuncStats = get_stats(extract_func_name(scope, l))
                    # for i, n in enumerate(nl):
                except Exception as E:
                    print("ESRC: " + l)
                    print(fmts(scope))
                    raise E
    scope.pop()
    for rn in rest:
        r = track.pop(rn)
        r.escape((-1,"TEST"))
        print(r)

def proc_lines(raw: str) -> list[str]:
    f: list[str] = []
    b = ""
    s = False
    lc = 1
    for i, c in enumerate(raw):
        if c == '\n':
            if len(b.strip()) > 0:
                f.append((lc, b))
                b = ""
            lc += 1
            continue
        if c == ';' and not b.strip().startswith("for"):
            if raw[i+1] != '\n':
                if len(b.strip()) > 0:
                    f.append((lc, b))
                    b = ""
                continue
            continue
        b += c
        # print(i, c)
        if (c == '"' or c == "'") and raw[i-1] != '\\':
            s = not s
        if s:
            continue
        if c == '{':
            if raw[i+1] != '\n':
                f.append((lc, b))
                b = ""
        elif c == '}':
            if len(b.strip()) > 0:
                f.append((lc, b[:-1]))
                b = ""
                f.append((lc, '}'))
    return f

def extract_func_name(ln: Line) -> str:
    l = ln[1]
    return l[:l.index("(")].split()[-1].strip()

def gen_func_name(filen: str, fn: str) -> str:
    return f"{{{filen}}}::{fn}"

def register_func(fi: str, f: list[Line]):
    f.insert(0, (0, fi))
    FUNCS[gen_func_name(fi, extract_func_name(f[1]))] = [False, f, []]


def print_func(*f):
    for k, v in f:
        print(f"{k} = ({v[0]}, [...], {tuple(v[2])})")

def check_file(filename: str):
    filename = path.normpath(filename)
    filename = filename[len(path.commonprefix((path.abspath(filename), path.curdir))):]
    print("ENCOUNTER: ", filename)
    # if filename in checked or (not omitpat.match(filename)) or len(checked) > 4:
    if filename in checked or (not omitpat.match(filename)):
        return
    print("CHECK: ", filename)
    checked.append(filename)
    raw: str = None
    with open(filename, "rt") as f:
        raw = f.read()
    tree = make_tree(proc_lines(raw))
    # print(repr(tree).replace("), (", "),\n("))
    for i in range(len(tree)):
        if type(tree[i]) == list:
            print(filename, tree[i][0])
            register_func(filename, tree[i])
            # check_block(tree[i], {}, [(0, filename)])
        else:
            if tree[i][1].strip().startswith("#include"):
                l = tree[i][1].strip().split(' ', 1)[1].strip()
                if l[0] == "<" or l in checked:
                    continue
                check_file(path.join(path.dirname(filename), l[1:-1]))

def check_branch(branch: Tree, track: dict[str, bool], scope: list[Line]) -> list[Violation]:
    dtrack: dict[str, bool] = {}
    for i in range(1, len(branch)):
        if type(branch[i]) == list:
            ...
        else:
            l = branch[i][1]

if argc > 1:
    fn = argv[1]
    dire = ".*"
    if argc > 2:
        l=[]
        with open(argv[2], "rt") as f:
            l=f.readlines()
        resalloc=list(map(re.compile, l[0].strip().split(",")))
        resdeloc=list(map(re.compile, l[1].strip().split(",")))
        if argv[1][0] == '-':
            if 'f' in argv[1]:
                fn = l[2]
            if 'd' in argv[1]:
                dire = l[3]
    else:
        resalloc = list(map(re.compile, input("alloc pattern: ").split(",")))
        resdeloc = list(map(re.compile, input("deloc pattern: ").split(",")))
    omitpat = re.compile(dire)
    check_file(fn.strip())
    print_func(*FUNCS.items())
    # print(*funcs.keys(), sep='\n')