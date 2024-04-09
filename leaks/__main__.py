from sys import argv
import re
from leaks.typedefs import *
from leaks.read import read
from leaks.cli import start as cli_start, test as cli_test

argc = len(argv)

resalloc: list[re.Pattern] = []
resdeloc: list[re.Pattern] = []

FUNCS: dict[str, FuncAttrs] = {}

def extract_func_name(line: Line) -> str:
    return FuncAttrs(line).name

def extract_func_args(line: Line) -> list[str]:
    return FuncAttrs(line).args

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

if argc > 1:
    if argv[1] == "test":
        cli_test()
        exit(0)
    # omitpat: re.Pattern = None
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
                fn = l[2].strip()
            if 'd' in argv[1]:
                dire = l[3].strip()
    else:
        resalloc = list(map(re.compile, input("alloc pattern: ").strip().split(",")))
        resdeloc = list(map(re.compile, input("deloc pattern: ").strip().split(",")))
    omitpat = re.compile(dire)
    leaf = read(fn, omitpat)
    cli_start(leaf)
    # print(leaf.print(1))
    # print(*leaf.children, sep='\n')
    # print(*leaf.children[4].children, sep='\n')
    # BlockType.test()
    # print()
    # for c in leaf.children:
    #     if c is Leaf:
    #         print(*c.children, sep=' :,,: ')