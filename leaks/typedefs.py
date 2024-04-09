from typing import Union, Self, Optional, NamedTuple
from enum import Enum
from os import path

# Line = tuple[int, str]

class Line(NamedTuple):
    lnum: int
    text: str

class RelKind(Enum):
    UNKNOWN: int = -1 # not checked yet
    NO: int = 0
    YES: int = 1
    NA: int = 2 # not applicable for argument (eg: type 'int')

class BlockType(Enum):
    INVALID: int = -3
    ROOT: int = -2
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
    def of(s: str) -> Self:
        s = s.strip()
        if len(s) < 2:
            return BlockType.INVALID
        if s[0] == '}':
            s = s[1:]
        if '(' not in s:
            s = s[:s.index('{')]
        else:
            s = s[:s.index('(')]
        s = s.strip().lower()
        match s:
            case "if":
                return BlockType.IF
            case "else if":
                return BlockType.ELIF
            case "else":
                return BlockType.ELSE
            case "for":
                return BlockType.FOR
            case "while":
                return BlockType.WHILE
            case "switch":
                return BlockType.SWITCH
            case "case":
                return BlockType.CASE
            case "default":
                return BlockType.DEFAULT
            case e:
                if ' ' in e:
                    return BlockType.FUNC
                raise ValueError(f"'{e}' is not a valid BlockType name")
    def toStr(self) -> str:
        # colors = ['235;0;0','135;135;135','200;135;0','100;200;0','100;200;0','100;200;0','']
        colors = ['235;0;0','135;135;135','200;135;0',*(['200;135;200']*3),*(['135;200;200']*2),'200;200;135','100;100;250','250;50;250','250;250;0']
        i = tuple(BlockType.__members__.values()).index(self)
        # print(i)
        return f"\x1b[38;2;{colors[i]}m{self.name}\x1b[0m"
    def test() -> None:
        print(*map(BlockType.toStr, BlockType.__members__.values()), sep='\n')

class BlockHeader():
    def __init__(self, k: BlockType, s: str, l: Line = None, scope: list[Self] = None) -> None:
        self.kind: BlockType = k # type of block
        self.line: Line = l # originating line
        self.src: str = s # originating file
        if scope == None:
            scope = []
        # scope = scope if scope != None else []
        # print("H:", scope)
        # print(k, l)
        self._scope = scope.copy()
        self._scope.append(self)
        if len(scope) > 0:
            s = path.abspath(s)
            s = s[len(path.commonpath((s, path.abspath(scope[-1].src)))):]
        if k == BlockType.FILE or k == BlockType.ROOT:
            self._name = f"{{{s}}}"
            self._fullname = f"{{{s}}}"
            return
        self._name = f"{k.name}@{l[0]}" if k != BlockType.FUNC else f"{FuncAttrs(l)}"
        self._fullname = f"{{{s}}}::"
        if len(scope) > 2:
            # self._fullname += f"{FuncAttrs(scope[2].line).name}/"
            for i in range(2, len(scope)):
                self._fullname += f"{scope[i]._name}/"
        self._fullname += self._name
    def scope(self) -> list[Self]:
        return self._scope
    def scope_str(self) -> str:
        f = ""
        for s in self._scope:
            f += f"{s}/"
        return f[:-1]
    def name(self) -> str:
        return self._name
    def fullname(self) -> str:
        return self._fullname
    def __repr__(self) -> str:
        # return f"{self._fullname}<-{self.kind.toStr()}"
        return f"{self.kind.toStr()}->{self._fullname}"
    DEFAULT: Self = None

BlockHeader.DEFAULT = BlockHeader(BlockType.INVALID, "!!!DEFAULT!!!", (-3, "DEFAULT"), [])

# Leaf = tuple[BlockHeader, list[Union[Line, tuple]]]
class Leaf():
    Children = list[Union[Line, Self]]
    def __init__(self, h: BlockHeader, c: Children) -> None:
        self.header = h
        self.children = c
    def get_leaf(self, ind: int) -> Optional[Self]:
        for i in range(len(self.children)):
            c = self.children[i]
            if isinstance(c, tuple):
                continue
            if ind == 0:
                return c
            ind -= 1
    def __getitem__(self, index) -> Union[Line, Self]:
        return self.children[index]
    def print(self, depth=-1, indent=-1) -> str:
        base = ""
        if indent == -1:
            base = f"{self}\n"
        else:
            base = f"{'|  '*indent}|\\ {self}\n"
        if depth == 0:
            return base
        for i in range(len(self.children)):
            if isinstance(self.children[i], Leaf):
                base += self.children[i].print(depth-1, indent + 1)
        return base
    def __str__(self) -> str:
        return f"{self.header}~[...]({len(self.children)})"

Tree = list[Leaf]
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
        try:
            self.name = l1[:l1.index('(')].strip()
            self.name = self.name.split()[-1]
            self.args = list(map(str.strip, l1[l1.index('(')+1:l1.rindex(')')].split(',')))
            self.rels = {(n,None) for n in self.args}
        except Exception as e:
            print("ERROR:", l)
            raise e
    def asKey(self) -> str:
        return
    def __str__(self) -> str:
        return f"{self.name}@{self.lnum}"

FUNCS: dict[str, FuncAttrs] = {}