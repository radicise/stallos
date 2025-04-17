from typing import Self, Any
from enum import Enum
from .utils import *

class Decoder():
    """decodes disk data"""
    def __init__(self, block: int, data: bytes) -> None:
        self.props: list[Property] = []
    def __getattribute__(self, name: str) -> Any:
        if name == "props":
            return super().__getattribute__(name)
        for p in self.props:
            if p.name == name:
                return p
        return super().__getattribute__(name)
    def __repr__(self) -> str:
        f = ""
        f += type(self).__name__
        maxs = [0, 0, 0, 0, 0]
        for p in self.props:
            parts = p.reprparts()
            maxs[0] = max(maxs[0], alignlen(parts[0]))
            maxs[1] = max(maxs[1], alignlen(parts[1]))
            maxs[2] = max(maxs[2], alignlen(parts[2]))
            maxs[3] = max(maxs[3], alignlen(parts[3]))
            maxs[4] = max(maxs[4], alignlen(parts[4]))
        # print(maxs)
        for p in self.props:
            f += f"\n    {p.alignrepr(maxs)}"
        return f
    def __str__(self) -> str:
        return repr(self)
    def _checkval(self, ch: int) -> None:
        if self.checksum.value == ch:
            self.checksum.valid = Validity.VALID
        else:
            self.checksum.valid = Validity.INVALID
            self.checksum.comment = f"calculated as !M{hex(ch)[1:]}!n"
    def crosscheck(self) -> None:
        pass

crosschecks: dict[int, Decoder] = {}

def checkStructure(f):
    def wf(s):
        if s.disk_loc.value >= crosschecks[0].usedleft.value:
            s.disk_loc.valid = Validity.INVALID
            s.disk_loc.comment = "outside structure region"
        else:
            s.disk_loc.valid = Validity.VALID
        f(s)
    return wf

class RootBlock(Decoder):
    """decodes a root block"""
    def __init__(self, block: int, data: bytes) -> None:
        self.props = [
            Property("disk_loc", block, DataType.DLOC, Validity.VALID),
            Property("breakver", int.from_bytes(data[:2]), DataType.U16, Validity.VALID),
            Property("partition_size", int.from_bytes(data[2:6]), DataType.SIZE),
            Property("creation_time", int.from_bytes(data[6:14]), DataType.TIME, Validity.PLAUSIBLE),
            Property("version", data[14:30], DataType.CSTR, Validity.VALID),
            Property("top_dir", int.from_bytes(data[30:34]), DataType.DLOC),
            Property("usedleft", int.from_bytes(data[34:38]), DataType.DLOC),
            Property("usedright", int.from_bytes(data[38:42]), DataType.DLOC),
            Property("!kusedhalfleft!n", int.from_bytes(data[42:46]), DataType.DLOC, comment="deprecated"),
            Property("!kusedhalfright!n", int.from_bytes(data[46:50]), DataType.DLOC, comment="deprecated"),
            Property("checksum", int.from_bytes(data[50:58]), DataType.HASH)
        ]
        self.partition_size.valid = Validity.VALID if self.partition_size.value <= MAX_PSIZE else Validity.INVALID
        self.top_dir.valid = Validity.VALID if self.top_dir.value == 7 else Validity.INVALID
        self.usedleft.valid = Validity.VALID if self.usedleft.value >= CORE_LPROTECT and self.usedleft.value < self.usedright.value else Validity.INVALID
        self.usedright.valid = Validity.VALID if self.usedright.value > self.usedleft.value and self.usedright.value < self.partition_size.value else Validity.INVALID
        ch = hashsized(data[:50])
        self._checkval(ch)
        crosschecks[0] = self

class StructNode(Decoder):
    """decodes a structure node"""
    def __init__(self, block: int, data: bytes) -> None:
        self.props = [
            Property("disk_loc", block, DataType.DLOC),
            Property("storage_flags", data[0], DataType.U8, Validity.VALID),
            Property("parent_loc", int.from_bytes(data[1:5]), DataType.DLOC),
            Property("pnode", int.from_bytes(data[5:9]), DataType.DLOC),
            Property("ikey", int.from_bytes(data[9:13])&0x05ffffff, DataType.IKEY),
            Property("!koid!n", int.from_bytes(data[13:17]), DataType.U32, comment="not implemented"),
            Property("!kgid!n", int.from_bytes(data[17:21]), DataType.U32, comment="not implemented"),
            Property("name", data[21:276], DataType.CSTR, Validity.VALID),
            Property("checksum", int.from_bytes(data[276:284]), DataType.HASH)
        ]
        if self.pnode.value != 0:
            self.pnode.valid = Validity.VALID
        else:
            self.pnode.valid = Validity.INVALID
            self.pnode.comment = "non-root must have a parent"
        if self.storage_flags.value == 1:
            self.storage_flags.comment = "directory"
            if self.parent_loc.value == 0:
                self.parent_loc.valid = Validity.INVALID
                self.parent_loc.comment = "directory must have a child table"
            else:
                self.parent_loc.valid = Validity.PLAUSIBLE
            if self.name.value[:2] == ('/'.encode("ascii")) + bytes(1) and self.pnode.value == 0:
                self.pnode.valid = Validity.VALID
                self.pnode.comment = None
            else:
                self.pnode.valid = Validity.INVALID
                self.pnode.comment = "root cannot have a parent"
            if self.ikey.value != 0:
                self.ikey.valid = Validity.INVALID
                self.ikey.comment = "directory ikeys must be zero"
            else:
                self.ikey.valid = Validity.VALID
        else:
            if self.parent_loc.value != 0:
                self.parent_loc.valid = Validity.INVALID
                self.parent_loc.comment = "non-directory cannot have a child table"
            else:
                self.parent_loc.valid = Validity.VALID
            if self.storage_flags.value == 2:
                self.storage_flags.comment = "file"
        ch = hashsized(data[:276])
        self._checkval(ch)
        crosschecks[block] = self
    @checkStructure
    def crosscheck(self) -> None:
        if self.storage_flags.value == 1:
            sb = crosschecks.get(self.parent_loc.value)
            if not isinstance(sb, StructBlock):
                self.parent_loc.valid = Validity.INVALID
                self.parent_loc.comment = "parent_loc must point to a child table"
            else:
                if sb.flags.value != 2:
                    self.parent_loc.valcom(Validity.INVALID, "parent_loc must point to a primary table")
                else:
                    if sb.disk_ref.value != self.disk_loc.value:
                        self.parent_loc.valcom(Validity.INVALID, "parent_loc must point to the table owned by the node")
                    else:
                        self.parent_loc.valcom(Validity.VALID)
        # rb = crosschecks[0]
        # if self.disk_loc.value >= rb.usedleft.value:
        #     self.disk_loc.valid = Validity.INVALID
        #     self.disk_loc.comment = "outside structure region"
        # else:
        #     self.disk_loc.valid = Validity.VALID

class StructBlock(Decoder):
    """decodes a structure block"""
    def __init__(self, block: int, data: bytes) -> None:
        self.props = [
            Property("disk_loc", block, DataType.DLOC),
            Property("entrycount", data[0], DataType.U8, Validity.VALID),
            Property("flags", data[1], DataType.U8, Validity.VALID),
            Property("disk_ref", int.from_bytes(data[2:6]), DataType.DLOC),
            Property("checksum", int.from_bytes(data[6:14]), DataType.HASH)
        ]
        if self.entrycount.value > 72:
            self.entrycount.valid = Validity.INVALID
            self.entrycount.comment = "struct block can only have 72 children"
        if self.flags.value > 2 or self.flags.value == 0:
            self.flags.valid = Validity.INVALID
            self.flags.comment = "flags for struct block can only be 1 or 2"
        elif self.flags.value == 1:
            self.flags.comment = "extended table"
            if self.entrycount.value == 0:
                self.entrycount.valid = Validity.INVALID
                self.entrycount.comment = "extended table cannot have 0 entries"
        else:
            self.flags.comment = "primary table"
        ch = hashsized(data[:6])
        self._checkval(ch)
        crosschecks[block] = self
    @checkStructure
    def crosscheck(self) -> None:
        par = crosschecks.get(self.disk_ref.value)
        if par == None:
            self.disk_ref.valcom(Validity.INVALID, "disk_ref must point to a parent")
        elif self.flags.value == 1:
            if not isinstance(par, StructBlock):
                self.disk_ref.valid = Validity.INVALID
                self.disk_ref.comment = "extended table parent must be another struct block"
            else:
                self.disk_ref.valid = Validity.VALID
        elif self.flags.value == 2:
            if not isinstance(par, StructNode):
                self.disk_ref.valid = Validity.INVALID
                self.disk_ref.comment = "primary table parent must be a struct node"
            else:
                if par.storage_flags.value != 1:
                    self.disk_ref.valid = Validity.INVALID
                    self.disk_ref.comment = "primary table parent must be a directory node"
                else:
                    self.disk_ref.valid = Validity.VALID

def crosscheck() -> None:
    for dec in crosschecks.values():
        dec.crosscheck()

def test() -> tuple[RootBlock, StructNode, StructBlock]:
    d = []
    r = None
    s = None
    b = None
    with open("FSMOCKFILE.mock", "rb") as f:
        d = f.read(1024)
        r = RootBlock(0, bytes(d))
        f.seek(1024 * r.top_dir.value, 0)
        d = f.read(1024)
        s = StructNode(r.top_dir.value, bytes(d))
        f.seek(1024 * s.parent_loc.value, 0)
        d = f.read(1024)
        b = StructBlock(s.parent_loc.value, bytes(d))
    crosscheck()
    pprint(r)
    pprint(s)
    pprint(b)
    return (r, s, b)
r, s, b = test()
