from typing import Any, Callable
from enum import Enum
from time import localtime, mktime
from datetime import datetime, time, timezone
from .nums import u64

def pprint(*values, sep: str | None = " ", end: str | None = "\n", flush: bool = False):
    def colform(string: str):
        if not string.startswith("!!n"):
            string = string.replace("\n", "!n\n")
        else:
            string = string[3:]
        return string.replace("!n", "\x1b[0m").replace("!r", "\x1b[38;5;196m").replace("!y", "\x1b[38;5;214m").replace("!g", "\x1b[32m").replace("!b", "\x1b[34m").replace("!m", "\x1b[35m").replace("!c", "\x1b[36m").replace("!u", "\x1b[38;5;192m").replace("!k", "\x1b[90m").replace("!M", "\x1b[38;5;163m")
    l = list(values)
    for i in range(len(l)):
        if not isinstance(l[i], str):
            l[i] = repr(l[i])
        l[i] = colform(l[i])
    print(*l, sep=sep, end=("\x1b[0m"+end), flush=flush)

def alignlen(string: str) -> int:
    return len(string.replace("!n",'').replace("!r",'').replace("!y",'').replace("!g",'').replace("!b",'').replace("!M",'').replace("!c",'').replace("!u",'').replace("!k",'').replace("!m",''))

def aljust(string: str, width: int, fill: str = ' ') -> str:
    return string + (fill * (width - alignlen(string)))
def arjust(string: str, width: int, fill: str = ' ') -> str:
    return (fill * (width - alignlen(string))) + string

def hashsized(data: bytes, size: int = -1) -> int:
    # h = u64()
    # c = 0
    # for i in range(size):
    #     c = data[i]
    #     h = (h << 6) + (h << 16) + c - h
    # return int(h)
    h = 0
    for i in range(size if size >= 0 else len(data)):
        c = data[i]
        h = (h << 6) + (h << 16) + c - h
        h &= 0xffffffffffffffff
    return h
    ##
    # u8* data = (u8*) vdata;
    # // printf("data size: %li\n", size);
    # u64 hash = 0;
    # int c;
    # while (size) {
    #     c = *(data++);
    #     // printf("%02x ", c);
    #     hash = c + (hash << 6) + (hash << 16) - hash;
    #     size --;
    # }
    # // putchar('\n');
    # return hash;
    #00 02 00 00 00 07
    #01 f8  ba 2a 87 d8 be 09

CORE_LPROTECT: int = 9
MAX_PSIZE: int = 2147483648

class Validity(Enum):
    VALID: int = 0
    INVALID: int = 1
    UNCHECKED: int = 2
    PLAUSIBLE: int = 3

def nrepr(self):
    c = "!b"
    if self == Validity.INVALID:
        c = "!r"
    elif self == Validity.UNCHECKED:
        c = "!y"
    elif self == Validity.PLAUSIBLE:
        c = "!c"
    elif self == Validity.VALID:
        c = "!g"
    return f"{c}{self.name}!n"
type(Validity.VALID).__repr__ = nrepr

def fmtikey(x: int) -> str:
    s = bin(x)[2:].rjust(32, '0')[6:]
    return f"!c{s[:10]}|{s[10:18]}|{s[18:26]}!n"

typeformatters = [(lambda x: "!k???!n"), (lambda x: f"!u{str(x)}!n"), (lambda x: f"!y{str(x[:x.find(0)], encoding="ascii")}!n"), (lambda x: f"!M{hex(x)[1:]}!n"), (lambda x: f"!u{x}!n [!u{hex(x)[1:]}!n]"), (lambda x: f"!u{hex(x)[1:]}!n"), (lambda x: f"!yUTC {datetime.fromtimestamp(mktime(localtime(x)),timezone.utc).isoformat(' ')[:-6]}!n"), (lambda x: fmtikey(x))]
fmtselectors = [0, 1, 1, 1, 1, 2, 3, 5, 4, 6, 7]

class DataType(Enum):
    """represents datatypes"""
    UNKOWN: int = 0
    U8: int = 1
    U16: int = 2
    U32: int = 3
    U64: int = 4
    CSTR: int = 5
    HASH: int = 6
    DLOC: int = 7
    SIZE: int = 8
    TIME: int = 9
    IKEY: int = 10
    def __repr__(self) -> str:
        return "!c"+self.name+"!n"
    def __str__(self) -> str:
        return repr(self)

class Property():
    """
    a property of a disk object
    stores the name, value, validity, and optional comment
    """
    def __init__(self, name: str, value: Any, datatype: DataType = DataType.UNKOWN, valid: Validity = Validity.UNCHECKED, /, comment: str = None) -> None:
        self.name = name
        self.value = value
        self.datatype = datatype
        self.valid = valid
        self.comment = comment
    def __repr__(self) -> str:
        return f"{self.name}: {self.datatype} = {typeformatters[fmtselectors[self.datatype.value]](self.value)} ({repr(self.valid)}){(' <-- '+self.comment) if self.comment else ''}"
    def __str__(self) -> str:
        return repr(self)
    def reprparts(self) -> tuple[str, str, str, str, str]:
        # return (f"{self.name}: ", f"{self.datatype} = ", f"{typeformatters[fmtselectors[self.datatype.value]](self.value)} ({repr(self.valid)})")
        return (f"{self.name}: ", f"{self.datatype} = ", f"{typeformatters[fmtselectors[self.datatype.value]](self.value)} ", f"({repr(self.valid)})", f"{(' <-- '+self.comment) if self.comment else ''}")
    def alignrepr(self, lens: tuple[int, int, int, int, int]) -> str:
        parts = self.reprparts()
        # return f"{aljust(parts[0], lens[0])}{arjust(parts[1], lens[1])}{arjust(parts[2], lens[2])}{parts[3]}"
        return f"{aljust(parts[0], lens[0])}{arjust(parts[1], lens[1])}{arjust(parts[2], lens[2])}{aljust(parts[3], lens[3])}{aljust(parts[4], lens[4])}"
    def valcom(self, val: Validity, com: str = None) -> None:
        self.valid = val
        self.comment = com
