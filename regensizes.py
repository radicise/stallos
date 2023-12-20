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

sizings: dict[str,int] = {
    "u8":1,
    "s8":1,
    "char":1,
}
keys = sizings.keys()

def parsePygen(gens: list[str]):
    l = len(gens)
    comment: str = None
    name: str = None
    start = 0
    for i in range(l):
        line = gens[i].strip()
        if line == "*/":
            start = i
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
        if re.match(line) != None:
            break
        line = lines[i].strip().split()
        if line[0] == "unsigned":
            line.pop(0)
        if line[0] not in keys:
            continue
        if "[" in line[1]:
            x = line[1]
            sizesum += sizings[line[0]] * int(x[x.index("[")+1:x.index("]")])
        sizesum += sizings[line[0]]
    if comment:
        content += f"// {comment}\n"
    content += f"#define {name} {sizesum}\n"

l = len(lines)
for i in range(l):
    if lines[l] == "PYGENSTART":
        parsePygen(lines[i:])

with open("os/kernel/filesystems/gensizes.h", "wt") as f:
    f.truncate()
    f.write(header+content+footer)