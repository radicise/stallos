from typing import Union, Self
from enum import Enum
from sys import argv
from os import path
import re

argc = len(argv)

checked: list[str] = []

resalloc: list[re.Pattern] = []
resdeloc: list[re.Pattern] = []

omitpat: re.Pattern = None

Line = tuple[int, str]

class RelKind(Enum):
    UNKNOWN: int = -1 # not checked yet
    NO: int = 0
    YES: int = 1
    NA: int = 2 # not applicable for argument (eg: type 'int')

class BlockType(Enum):
    FILE: int = -1
    IF: int = 0
    ELIF: int = 1
    ELSE: int = 2
    FOR: int = 3
    WHILE: int = 4
    SWITCH: int = 5
    CASE: int = 6
    DEFAULT: int = 7
    FUNC: int = 8

class BlockHeader():
    def __init__(self, k: BlockType, l: Line, s: str, scope: list[Self]) -> None:
        self.kind: BlockType = k # type of block
        self.line: Line = l # originating line
        self.src: str = s # originating file
        self._name = f"{k.name}@{l[0]}" if k is not BlockType.FUNC else f"{extract_func_name(l)}"
        self._fullname = f"{{{s}}}::"
        if len(scope) > 1:
            self._fullname += f"{extract_func_name(scope[1].line)}/"
            for i in range(2, len(scope)):
                self._fullname += f"{scope[i]._name}/"
        self._fullname += self._name
    def name(self) -> str:
        return self._name
    def fullname(self, scope: list[Self]) -> str:
        return self._fullname

Tree = list[tuple[BlockHeader, list[Union[str, list]]]]
Scope = list[BlockHeader]

class Violation():
    def __init__(self, l: Line, m: str, s: Scope) -> None:
        self.line = l
        self.msg = m
        self.scope = s

class FuncAttrs():
    def __init__(self, l: Line) -> None:
        l1 = l[1]
        self.lnum = l[0]
        self.name = l1[:l1.index('(')].strip()
        self.name = self.name.split()[-1]
        self.args = list(map(str.strip, l1[l1.index('(')+1:l1.rindex(')')].split(',')))
        self.rels = {(n,None) for n in self.args}

FUNCS: dict[str, FuncAttrs] = {}

def extract_func_name(line: Line) -> str:
    return FuncAttrs(line).name

def extract_func_args(line: Line) -> list[str]:
    return FuncAttrs(line).args


def make_tree(lines: list[Line]) -> Tree:
    f: Tree = []
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