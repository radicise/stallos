from typing import Self

def sizedop(f):
    def wf(a, b):
        if isinstance(b, SizedNum):
            if a._smaller(b):
                return NotImplemented
            return f(a, b._)
        elif isinstance(b, bytearray) or isinstance(b, bytes):
            return f(a, b)
        elif isinstance(b, int):
            return f(a, b.to_bytes(a.__class__.SIZE))
        else:
            raise TypeError("unsupported type")
    return wf

class SizedNum:
    SIZE = None
    _Other = Self | int | bytes | bytearray
    def __call__(self):
        o = type(self).__new__(type(self))
        o.__init__()
        return o
    def __init__(self, value=None):
        self._: bytearray = bytearray(self.__class__.SIZE)
        if value != None:
            self += value
    def copy(self) -> Self:
        n: Self = self()
        n._ = self._.copy()
        return n
    def __int__(self) -> int:
        return int.from_bytes(self._)
    def __index__(self) -> int:
        return int(self)
    def __repr__(self) -> str:
        return repr(int(self))
    def _smaller(self, other):
        if not isinstance(other, SizedNum):
            raise TypeError()
        return self.__class__.SIZE < other.__class__.SIZE
    def __add__(self, other: _Other) -> Self:
        return self.copy().__iadd__(other)
    def __sub__(self, other: _Other) -> Self:
        return self.copy().__isub__(other)
    def __mul__(self, other: _Other) -> Self:
        return self.copy().__imul__(other)
    def __floordiv__(self, other: _Other) -> Self:
        return self.copy().__ifloordiv__(other)
    def __truediv__(self, other: _Other) -> Self:
        return self // other
    def __mod__(self, other: _Other) -> Self:
        return self.copy().__imod__(other)
    def __lshift__(self, other: _Other) -> Self:
        return self.copy().__ilshift__(other)
    def __rshift__(self, other: _Other) -> Self:
        return self.copy().__irshift__(other)
    def __radd__(self, other: _Other) -> Self:
        return self + other
    def __rsub__(self, other: _Other) -> Self:
        return self - other
    def __rmul__(self, other: _Other) -> Self:
        return self * other
    def __rfloordiv__(self, other: _Other) -> Self:
        return self // other
    def __rtruediv__(self, other: _Other) -> Self:
        return self // other
    def __rmod__(self, other: _Other) -> Self:
        return self % other
    def __itruediv__(self, other: _Other) -> Self:
        return self.copy().__ifloordiv__(other)
    @sizedop
    def __iadd__(self, other: _Other) -> Self:
        ol = len(other)
        ld = len(self._) - ol
        carry = 0
        for i in range(ol):
            c = self._[ld+i]
            c += carry + other[i]
            self._[ld+i] = c % 256
            carry = c // 256
        return self
    @sizedop
    def __isub__(self, other: _Other) -> Self:
        ol = len(other)
        ld = len(self._) - ol
        carry = 0
        for i in range(ol):
            c = self._[ld+i]
            c -= carry + other[i]
            self._[ld+i] = c % 256
            carry = +c // 256
        return self
    @sizedop
    def __imul__(self, other: _Other) -> Self:
        ol = len(other)
        ld = len(self._) - ol
        carry = 0
        for i in range(ol):
            c = self._[ld+i]
            c *= other[i]
            c += carry
            self._[ld+i] = c % 256
            carry = c // 256
        return self
    @sizedop
    def __divmod__(self, other: _Other) -> tuple[int, int]:
        q = 0
        r = 0
        i = self.SIZE - 1
        d = int.from_bytes(other)
        while i >= 0:
            r <<= 1
            q <<= 1
            r |= (self._[i//8]>>(i%8))&1
            if r >= d:
                r -= d
                q |= 1
            i -= 1
        return (q, r)
    @sizedop
    def __ifloordiv__(self, other: _Other) -> Self:
        v = self.__divmod__(other)[0]
        self._ = v.to_bytes(self.SIZE)
    @sizedop
    def __imod__(self, other: _Other) -> Self:
        v = self.__divmod__(other)[1]
        self._ = v.to_bytes(self.SIZE)
    def __ilshift__(self, other: int | Self) -> Self:
        other = int(other)
        acc = 0
        for i in range(self.SIZE):
            acc |= (self._[self.SIZE-1-i] << other)
            self._[self.SIZE-1-i] = acc & 0xff
            acc >>= 8
        return self
    def __irshift__(self, other: int | Self) -> Self:
        other = int(other)
        acc = 0
        for i in range(self.SIZE):
            acc |= ((self._[i] << (i*8)) >> other)
            self._[i] = (acc>>(i*8)) & 0xff
            acc >>= 8
        return self

class u8(SizedNum):
    SIZE = 1
class u16(SizedNum):
    SIZE = 2
class u32(SizedNum):
    SIZE = 4
class u64(SizedNum):
    SIZE = 8
