from os import path
import os
from leaks.typedefs import *
import re

checked: list[str] = []
ifpat = re.compile("^}?[\\s]*(if|else|for|while|switch)")
apat = re.compile("(?<!(=|\\||\\&|<|>|\\+|-|\\*|/|!))=(?!=)")
# omitpat: re.Pattern = None

def make_tree(fname: str, lines: list[Line], ps: list[BlockHeader]) -> Leaf:
    head: BlockHeader = BlockHeader(BlockType.FILE, fname, None, ps)
    f: Leaf.Children = []
    ca = [f]
    sc = ps.copy()
    sc.append(head)
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
            # if ("struct" not in line and "=" not in line) or (ifpat.match(line)):
            # if ("typedef" not in line and not apat.match(line)) or (ifpat.match(line)):
            # if ("typedef" not in line and not apat.match(line)):
            # if ("typedef" not in line and not ''.join(line.split())[-2] == '='):
            t = ''.join(line.split())
            b = '=' in t
            if b:
                if t.startswith(("for", "while")):
                    b = False
                else:
                    # print(t)
                    find = t.index('=')-1
                    ch = t[find]
                    b = t[find+2] != '=' and (ch.isalnum() or ch == ']')
                    # print(ch, b, t[find+2])
            if ((not line.startswith("struct")) and "typedef" not in line and not b):
            # if ("=" not in line) or (ifpat.match(line)):
                l = [(lines[i][0],line)]
                try:
                    # print("T:", sc)
                    ns = BlockHeader(BlockType.of(line), fname, (lines[i][0],line), sc)
                    sc.append(ns)
                    # n = Leaf(sc[-1], [])
                    n = Leaf(ns, [])
                except Exception as e:
                    print("ERROR AT:", lines[i][0], fname)
                    print(*lines[i-3:i+3], sep='\n')
                    raise e
                ca[-1].append(n)
                ca.append(n.children)
            else:
                ig += 1
        elif line.endswith("}"):
            if len(ca) > 1:
                ca[-1].append((lines[i][0],line))
            if ig > 0:
                ig -= 1
            elif len(ca) > 1:
                ca.pop()
                sc.pop()
        else:
            ca[-1].append((lines[i][0],line))
        i += 1
    return Leaf(head, f)

def proc_lines(raw: str) -> list[str]:
    f: list[str] = []
    b = ""
    s = False
    lc = 1
    stre = ''
    lcom = False
    bcom = False
    for i, c in enumerate(raw):
        if c == '/':
            if raw[i+1] == '/':
                lcom = True
            elif raw[i+1] == '*':
                bcom = True
        if lcom:
            if c != '\n':
                continue
            lcom = False
            lc += 1
            b = ''
            continue
        if bcom:
            if c == '\n':
                lc += 1
            if c != '/':
                continue
            if raw[i-1] != '*':
                continue
            bcom = False
            b = ''
            continue
        if c == '\n':
            if len(b.strip()) > 0:
                f.append((lc, b))
                b = ""
            lc += 1
            continue
        if (c == '"' or c == "'") and raw[i-1] != '\\':
            s = not s
        if s:
            b += c
            continue
        if c == ';' and not b.strip().startswith("for"):
            b = b.strip()
            if len(b) == 0:
                continue
            if b.startswith(("if", "else if", "else")):
                j = 0
                d = 0
                while j < len(b):
                    if b[j] == '(':
                        d += 1
                    elif b[j] == ')':
                        d -= 1
                        if d == 0:
                            break
                    j += 1
                f.append((lc, b[:j+1]+'{'))
                f.append((lc, b[j+1:]))
                f.append((lc, '}'))
                b = ""
            elif raw[i+1] != '\n':
                if len(b.strip()) > 0:
                    f.append((lc, b))
                b = ""
            continue
        b += c
        # print(i, c)
        # if len(stre) > 0:
        #     if stre == "'":
        #         if c == "'" and raw[i-1] != '\\':
        #             b = ''
        #             s = False
        #     else:
        #         if c == '"' and raw[i-1] != '\\':
        #             b = ''
        #             s = False
        # else:
        #     if c == '"' or c == "'":
        #         f.append((lc,b[:-1]))
        #         b = ''
        #         s = True
        if c == '{':
            if lc == 198:
                print(b)
            if raw[i+1] != '\n':
                f.append((lc, b))
                b = ""
        elif c == '}':
            if len(b.strip()) > 0:
                f.append((lc, b[:-1]))
                b = ""
                f.append((lc, '}'))
    return f

# def check_file(filename: str):
#     filename = path.normpath(filename)
#     filename = filename[len(path.commonprefix((path.abspath(filename), path.curdir))):]
#     print("ENCOUNTER: ", filename)
#     # if filename in checked or (not omitpat.match(filename)) or len(checked) > 4:
#     if filename in checked or (not omitpat.match(filename)):
#         return
#     print("CHECK: ", filename)
#     checked.append(filename)
#     raw: str = None
#     with open(filename, "rt") as f:
#         raw = f.read()
#     tree = make_tree(proc_lines(raw))
#     # print(repr(tree).replace("), (", "),\n("))
#     for i in range(len(tree)):
#         if type(tree[i]) == list:
#             print(filename, tree[i][0])
#             register_func(filename, tree[i])
#             # check_block(tree[i], {}, [(0, filename)])
#         else:
#             if tree[i][1].strip().startswith("#include"):
#                 l = tree[i][1].strip().split(' ', 1)[1].strip()
#                 if l[0] == "<" or l in checked:
#                     continue
#                 check_file(path.join(path.dirname(filename), l[1:-1]))

# path.curdir = '..'
# print(path.curdir)

def read_file(fname: str, omitpat: re.Pattern, ps: list[BlockHeader]) -> Optional[tuple[Leaf, str]]:
    f: Tree = []
    fname = path.abspath(path.normpath(fname))
    # print(fname)
    # fname = fname[len(path.join(*path.split(path.commonprefix((path.abspath(fname), os.getcwd())))[:-1])):]
    fname = fname[len(path.commonprefix((path.abspath(fname), path.curdir))):]
    if fname in checked or (not omitpat.match(fname)):
        return None
    checked.append(fname)
    tocheck = []
    # if ps == None:
    #     ps = [BlockHeader(BlockType.ROOT, path.dirname(fname), (-2, 'ROOT'))]
    # print(fname)
    raw: str = None
    with open(fname, "rt") as fi:
        raw = fi.read()
    lines = proc_lines(raw)
    # if 'fsdefs' in fname:
    #     print(*lines, sep='\n')
    #     return []
    tree: Leaf = make_tree(fname[len(path.join(*path.split(path.commonprefix((path.abspath(fname), os.getcwd())))[:])):], lines, ps)
    f.append(tree)
    for i in range(len(tree.children)):
        # print(tree[i])
        if isinstance(tree[i], tuple):
            treei: Line = tree[i]
            # print(treei[1])
            if treei[1].strip().startswith("#include"):
                l = treei[1].strip().split(' ', 1)[1].strip()
                if l[0] == "<":
                    continue
                tocheck.append(path.join(path.dirname(fname), l[1:-1]))
                # r = read(path.join(path.dirname(fname), l[1:-1]), omitpat, tree.header.scope())
                # if r != None:
                #     f.extend(r.children)
                    # f.append(r)
    return (tree, tocheck)
    # return Leaf(BlockHeader(BlockType.ROOT, "ROOT", (-2, path.curdir), []), f)

def read(fname: str, omitpat: re.Pattern) -> Leaf:
    rs = BlockHeader(BlockType.ROOT, path.dirname(path.abspath(fname)))
    topleaf: Leaf = Leaf(rs, [])
    tocheck: list[str] = [fname]
    while len(tocheck) > 0:
        r = read_file(tocheck.pop(0), omitpat, [rs])
        if r != None:
            tocheck.extend(r[1])
            topleaf.children.append(r[0])
    return topleaf