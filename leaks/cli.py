from leaks.typedefs import *
import traceback
import tty
import termios
import atexit
import sys
import os
from typing import Self, Any, Callable, Optional, Union, Tuple

class IOHandle():
    """handles IO with more fine-grained control, creating an instance puts terminal in cbreak mode"""
    IN = sys.stdin
    OUT = sys.stdout
    _INIT = False
    def _tty_return(_s):
        termios.tcsetattr(1, termios.TCSANOW, _s)
    def __init__(self) -> None:
        _tty_settings = termios.tcgetattr(1)
        self.read: Callable[[int], str] = self.IN.read
        tty.setcbreak(1)
        if self._INIT == False:
            atexit.register(IOHandle._tty_return, _tty_settings)
        self._INIT = True
        ###
        self._buf = ""
        self.writehead = 0
        self._off = 0
    def _cbuf(self) -> str:
        """returns and clears the internal buffer"""
        x = self._buf
        self._buf = ""
        self.writehead = 0
        return x
    def write(self, data: str, _flush: bool = True) -> None:
        self.OUT.write(data)
        if _flush:
            self.OUT.flush()
    def _process(self, c: str):
        if c == '\x1b':
            os.set_blocking(0, False)
            ch = self.read(2)
            os.set_blocking(0, True)
            if len(ch) == 0 or ch[0] != '[':
                return
            if ch[1] == 'D':
                if self.writehead > 0:
                    self.writehead -= 1
            if ch[1] == 'C':
                if self.writehead < len(self._buf):
                    self.writehead += 1
        elif c == '\x7f':
            if self.writehead == 0:
                return
            self._buf = self._buf[:self.writehead-1] + self._buf[self.writehead:]
            self.writehead -= 1
        else:
            self._buf = self._buf[:self.writehead] + c + self._buf[self.writehead:]
            self.writehead += 1
        self.write(f"\x1b[{self._off+1}G{self._buf}\x1b[K\x1b[{self._off+self.writehead+1}G")
    def prompt(self, _prompt: str = "") -> str:
        """reads a line of input from the user with a given prompt"""
        self._off = len(_prompt)
        self.write(_prompt)
        while True:
            c = self.read(1)
            if c == '\n':
                self.write('\n')
                return self._cbuf()
            self._process(c)
    _WK_TYP = Optional[Union[str, int, Tuple[Union[str, int]]]]
    def _fin_nl_deco(_f):
        def _r(*args, **kwargs):
            r = _f(*args, **kwargs)
            print('\n', end='')
            return r
        return _r
    @_fin_nl_deco
    def wait_key(self, _prompt: str = "", keys: _WK_TYP = None) -> str:
        """waits for any key in the given set to be pressed, then returns which was pressed"""
        if type(keys) == tuple or type(keys) == str:
            if len(keys) == 0:
                keys = None
        if keys == None:
            keys = tuple()
        if type(keys) == str:
            keys = tuple(keys.split(''))
        if type(keys) == int:
            keys = (keys,)
        if type(keys) != tuple:
            raise TypeError(f"{type(keys)} is not one of None, str, int, or tuple")
        self.write(_prompt)
        while True:
            c = self.read(1)
            if c in keys or ord(c) in keys:
                return c
    @_fin_nl_deco
    def wait(self, _prompt: str = "") -> None:
        """waits for the user to press return"""
        self.write(_prompt)
        while True:
            c = self.read(1)
            if c == '\n':
                return
    def clear(self) -> None:
        """clears the terminal"""
        self.write('\x1bc')

class CLI():
    def __init__(self, tree: Leaf) -> None:
        self.tree = tree
        self.head = [tree]
        self.io = IOHandle()
    def arbmode(self) -> None:
        while True:
            com = self.io.prompt(">>> ")
            if com == 'quit':
                return
            try:
                print(eval(com))
            except Exception:
                traceback.print_exc()
    def print_head(self) -> None:
        self.io.clear()
        opts = self.head[-1].print(1).splitlines()
        ml = len(str(len(opts)-2))
        print(f"{'*'*ml} {opts[0]}")
        for i in range(1, len(opts)):
            print(f"{str(i-1).rjust(ml,'0')} {opts[i]}")
        print()
    def start(self) -> None:
        while True:
            self.print_head()
            com = self.io.prompt("> ")
            if com == 'quit':
                return
            if com == 'arb':
                self.arbmode()
            if com.startswith("enter"):
                e = int(com.split(' ', 1)[1])
                l = self.head[-1].get_leaf(e)
                if l != None:
                    self.head.append(l)
            elif com == 'done':
                self.head.pop()
                if len(self.head) == 0:
                    return
            elif com == 'scope':
                print(self.head[-1].header.scope_str())
                self.io.wait(":")

def start(tree: Leaf) -> None:
    CLI(tree).start()

def test() -> None:
    h = IOHandle()
    print(repr(h.read(1)))
    # print(h.prompt("HI>"))
    # h.wait(":")
    # h.wait(";")
    # h.clear()
    # os.set_blocking(0, False)
    # try:
    #     t = h.read(5)
    #     print(type(t))
    # finally:
    #     os.set_blocking(0, True)