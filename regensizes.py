### this script regenerates the size constants in "gensizes.h"

import re

endpat = re.compile("}.*")

header: str = """
#ifndef __GENSIZES_H__
#define __GENSIZES_H__ 1
/*
THIS FILE IS GENERATED AUTOMATICALLY
run the python3 script "regensizes.py" to regenerate this file
*/
"""

content: str = ""

footer: str = "\n#endif"

lines: list[str] = None

with open("os/kernel/filesystems/fsdefs.h", "rt") as f:
    lines = f.readlines()

# print(''.join(lines))

sizings: dict[str,int] = {
    "u8":1,
    "s8":1,
    "char":1,
    "u16":2,
    "s16":2,
    "u32":4,
    "s32":4,
    "u64":8,
    "s64":8,
}
keys = sizings.keys()

def parsePygen(gens: list[str]):
    global content
    l = len(gens)
    typedef: bool = False
    comment: str = None
    name: str = None
    start = 0
    for i in range(l):
        line = gens[i].strip()
        if line == "*/":
            start = i+1
            break
        if line == "pygen-typedef":
            start = i+1
            typedef = True
            break
        parts = line.split(":", 1)
        for j in range(len(parts)):
            parts[j] = parts[j].strip()
        if parts[0] == "comment":
            comment = parts[1]
        if parts[0] == "name":
            name = parts[1]
    if name == None:
        raise ValueError("UNNAMED PYGEN")
    sizesum = 0
    for i in range(start, l):
        if typedef and gens[i].strip() == "pygen-end":
            sizings[name] = sizesum
            return
        if endpat.match(gens[i]) != None:
            sizings[gens[i].strip()[1:-1].strip()] = sizesum
            break
        line = gens[i].strip().split()
        if len(line) == 0:
            continue
        if line[0] == "unsigned":
            line.pop(0)
        if line[0].endswith("*"):
            line[0] = line[0][:-1]
        if line[0] not in keys:
            continue
        if "[" in line[1]:
            x = line[1]
            sizesum += sizings[line[0]] * int(x[x.index("[")+1:x.index("]")])
        else:
            sizesum += sizings[line[0]]
    if comment:
        content += f"\n// {comment}\n"
    content += f"#define {name} {sizesum}"

l = len(lines)
for i in range(l):
    if lines[i].strip() == "PYGENSTART":
        parsePygen(lines[i:])

content += "\n"

with open("os/kernel/filesystems/gensizes.h", "wt") as f:
    f.truncate()
    f.write(header+content+footer)