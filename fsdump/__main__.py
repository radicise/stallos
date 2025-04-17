"""
modified hexdump
"""
import os
import sys
import subprocess

from .decoders import RootBlock
from .utils import pprint

assert len(sys.argv) >= 2

class DataFormatter():
    def check_data(block: int, data: bytes) -> bool:
        raise NotImplementedError()
    def format(block: int, data: bytes) -> str:
        raise NotImplementedError()

class ZeroFormatter(DataFormatter):
    def check_data(block: int, data: bytes) -> bool:
        return data.count(0) == 1024
    def format(block: int, data: bytes) -> str:
        return ""
        # return "*0*"

class DefaultFormatter(DataFormatter):
    """
    fallback for when data doesn't match any other format
    """
    def check_data(block: int, data: bytes) -> bool:
        return True
    def format(block: int, data: bytes) -> str:
        lines = [f"<{'  '.join([' '.join([hex(data[b])[2:].rjust(2, '0') for b in range(i+j, i+j+8)]) for j in range(0, 16, 8)])}>" for i in range(0, len(data), 16)]
        i = 0
        count = 0
        remove = False
        while i < len(lines)-1:
            if lines[i] == lines[i-1]:
                lines.pop(i)
                remove = True
                count += 1
                continue
            if remove:
                remove = False
                lines.insert(i, f"*{count}")
                count = 0
            i += 1
        if remove:
            lines.insert(i, f"*{count}")
        return f"!c{hex(block)[2:].rjust(8, '0')}:\n"+'\n'.join(lines)

class RootBlockFormatter(DataFormatter):
    def check_data(block: int, data: bytes) -> bool:
        return block == 0
    def format(block: int, data: bytes) -> str:
        b = RootBlock(data)

# raw = subprocess.run(f"hexdump -C {sys.argv[1]}", shell=True, capture_output=True, text=True, encoding="ascii", errors="replace").stdout.splitlines()

# def calcrep(loc):
#     num = int(loc, base=16)
#     # print(num)
#     if num % 1024 == 0:
#         return "Z"+hex(num // 1024)[2:].rjust(7, "0")
#     else:
#         return loc

# for i in range(len(raw)):
#     line = raw[i]
#     if len(line) > 1:
#         line = line.replace(line[:8], calcrep(line[:8]), 1)
#     raw[i] = line

# for line in raw:
#     print(line, flush=False)
# sys.stdout.flush()

formatters: list[DataFormatter] = []
for name in dir():
    if name in ("DataFormatter", "DefaultFormatter", "ZeroFormatter") or eval(f"repr({name}.__class__) != repr(DataFormatter.__class__)"):
        continue
    if eval(f"issubclass({name}, DataFormatter)"):
        eval(f"formatters.append({name})")

formatters.append(ZeroFormatter)
formatters.append(DefaultFormatter)

print(formatters)

with open(sys.argv[1], "rb") as f:
    block = 0
    while True:
        data = f.read(1024)
        if len(data) == 0: # break when all data has been read
            break
        for p in formatters:
            if p.check_data(block, data):
                r = p.format(block, data)
                if len(r) > 0:
                    print()
                    pprint(r)
                break
        block += 1

# print(subprocess.run(f"-C {sys.argv[1]}", executable="hexdump").stdout)
# print(subprocess.check_output(f"-C {sys.argv[1]}", executable="hexdump"))
