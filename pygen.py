### this script regenerates the size constants in "gensizes.h"

from sys import argv

if "help" in argv:
    helptopics = """
please choose a topic:
    using-help
    howto
    typedef
    macro
    readwrite
    helperformat
    misc
"""
    helpbadtopic = "sorry, that topic isn't recognized"
    helpprimer = """
WARNING: pygen only operates on one file, ensure that all pygen needed is in THAT FILE
pygen will begin processing after each line containing only "PYGENSTART"
pygen processes in two stages, the first stage is looking for pygen directives, the second is calculating sizes and generating code (note, some directive will cause different behavior)
pygen defaults to creating a macro that resolves to the size of the pygen'd struct as determined by summing the size of its fields
any struct pygen runs on will be recorded as a type within pygen, allowing fields that are other structs
for this to function, pygen has one required directive, and one optional
required is "name" in the format of "name: [value]", where value is the name that should be given to the generated macro
optional is "comment", where the value is any text and will be placed in a one-line comment and should describe the generated macro's purpose
pygen will then reach the end of the comment "*/", and begin the process of calculating the size of the struct
pygen goes through all fields, disregarding any lines that do not describe a field
NOTE: pygen DOES NOT ignore comments unless they cause a line to not be a valid field declaration, so the following:
{
    u16   hi;
    /*
    u64   bye;
    */
}
will be recorded as 10 bytes, not 2, this can be used to inflate the sum, ensure all inflation is intentional, as this
would be hard to debug
pygen stops its search upon reaching a line that begins with a closing curly bracket, pygen then extracts the name of the struct and uses that
NOTE: pygen is not capable of handling structs declared as follows:
struct T {};
instead use the following:
typedef struct {} T;
"""
    helptypedef = """
typedef is a convienient way to get the size of some format totaled, without having to actually declare a struct that will never be used
the directive "pygen-typedef" will cause pygen to begin part two of processing within the comment, note that the "name" directive will now be interpreted as the struct name
it is important to add "pygen-end" to the end of the declaration, otherwise pygen will continue to process lines
"""
    helpmacdef = """
sometimes, a struct's size is dependent on a macro, say: char version[VERSION_LENGTH]
IMPORTANT NOTE: the usage of macro values is restricted to defining the length of an array
ALSO: pygen DOES support arrays sized with 'magic' numbers
however, VERSION_LENGTH is: a) not a type, it's a number, and b) pygen does not know what VERSION_LENGTH expands to
how to fix this:
go to wherever VERSION_LENGTH is defined, and add the following above it:
/*
PYGENSTART
pygen-macro
*/

pygen will now search for the next #define, and record it, all integer formats are supported (eg. 10, 0x1a, etc.)
note that this is still not a type, and cannot be used in place of a type, to create a type of custom size, use a typedef
"""
    helpreadwrite = """
lots of things pygen is used on are transmitted through streams, to make this easier, helper read/write functions may be used
however, maintaining these yourself is a recipe for disaster, if you change the definition of the struct, you HAVE to
remember, on your own, to update the helper functions, luckily you can tell pygen to generate the helpers for you
simply add the following directive: "pygen-mk-rw: [name]", where helpers will be outputted according to the following:
void [pygen-rw-pre][read|write]_[name](<[objname]* obj, >?[classname]* [name]) {...}
see the "misc" section of help for what "pygen-rw-pre" does
[classname] is extracted from the struct definition
within the function body, the lines will be according to the following (note, the following is for a write helper, read helpers are only of marginal difference):
unsigned char x[macro-name];
unsigned char* xp = (unsigned char*)x;
<
awrite_u[16|32|64]be(xp+{offset}, [name]->[fieldname]);|
awrite_buf(xp+{offset}, [name]->[fieldname], [fieldsize]);|
x[{offset}] = [name]->[fieldname];
>+

pygen will insert the following line upon reaching the first field with the word "checksum" in it
NOTE: this field is assumed to represent the checksum of the current contents of array "x" at this point
[name]->[fieldname] = hash_sized(xp, {offset});
pygen will then resume the previous pattern all fields have been written, pygen then finishes with the following
write_buf(<obj, >?xp, [size]);

where [size] is the recorded size of the struct
when [objname] is not provided, all <>? are not included

pygen will also create a hash function, this will create a hash of the buffer representation of the struct until the first field containing "checksum"
"""
    helpmisc = """
this is for all directives that don't need a dedicated topic

pygen-rw-pre -- sets the [pygen-rw-pre] variable, altering the behavior of generating helper functions, format:
"pygen-rw-pre: [value]", WARNING: ensure that [value] is valid as the start of a C function

pygen-end -- stops pygen interpretation in stage one, will only stop stage two if stage two was started due to a typedef

pygen-fake -- format of "pygen-fake [value]", where value is some integer (can be negative), that will be directly added to
the calculated size, bypassing everything else

pygen-hf-use -- uses the specified helper format, format: "pygen-hf-use: [format-name]"

pygen-force-* -- has variants corresponding to those of "default-mk-*" (see "helperformat"), and will be used instead if
present

pygen-include -- will cause all lines until "pygen-end" to be interpreted as the arguments for "#include" directives, and
these directives will be inserted into the generated file

pygen-align -- "_: [align]" will cause the size to be immediately padded to the specified alignment (recommend use only at end of struct definition)

pygen-over -- "_: [fieldname, ...] [size-type] [expression]
[expression]: [code | name, ...] -> [name]: "`fieldname`", [code]: valid C code
"""
    helprwfmt = """
pygen-mk-rw is very useful, but outwardly highly inflexible, only able to be used for one case
using the directive "pygen-hf-fmt" allows for extreme flexibility

/*
PYGENSTART
pygen-hf-fmt
name: [fmtname|$default]
objname: [classname]?
readerpre: [prefix|$none]
writerpre: [prefix|$none]
sbufread: {see buffer reader/writer specifier format}
sbufwrite: {see buffer reader/writer specifier format}
bufcpy: [name]
hashf: [name]
default-mk-rw: [bool]?
default-mk-hash: [bool]?
pygen-end
*/

[classname] is the name of the struct that the read of write function uses, if left blank, it specifies that a struct is not required

[bufcpy] is the name of a function that has signature: void [bufcpy](void* dst, void* src, size_t size);

[hashf] is the name of a hash function that has signature: unsigned long long [hashf](void* data, off_t size);

[default-mk-*]  directives specify what to generate by default, any omitted variants will themselves default to "true"
NOTE: "default-mk-all" will specify the option for all variants

all "$" values have special meaning, $none means there is no prefix, $default means this is the default helper fmt that should be used unless otherwise specified
NOTE: "name" may be specified multiple times, each "name" will be set to the defined format

buffer reader/writer specifier format:
"[head]{size}[tail]?" (NOTE: "{size}" here is meant to be included literally)
while [tail] is optional, [head] is not, as it should distinguish between reading and writing, and {size} will be a number, and therefore not valid for the begining of a symbol name

to specify a format to use, add the following to your pygen:
pygen-fmt-use: [fmtname]

you can also specify a "copy" directive, to make a copy of a previously defined format:
"copy: [fmtname]"
you can use this to preserve things such as the default format when overriding it
NOTE: convention when copying to a temporary name is to use "$tmp" as the temporary name
"""
    helphelp = """
this page describes how to interpret other help pages

syntax: (NOTE: this only applies to code snippets for pygen, C snippets will, of course, conform to C syntax)
    within code snippets, you will sometimes find various brackets
    these brackets each have their own meaning, as defined below, in the event that these brackets are meant as literal brackets, and not escapes, a (NOTE:) will immediately follow the snippet to clarify this
    
    "[]": square brackets are the most common, they define some placeholder for an option that should be specified
        square brackets will contain 1) a variable name, this name will usually describe what should be put in the brackets (eg. [name] should be specified as a name), and may be reffered to by the help page later; or 2) a type (see "types"), this is used in cases where a literal value is expected (eg. [bool])
        square brackets may be followed by a "?", which denotes an optional parameter
    
    "{}": curly brackets are used to represent internal variables (eg. {offset} under the "readwrite" page), and will be
        used only to describe the format in which outputted code will be generated
    
    "<>?": angle brackets with a "?" suffix are used to denote areas which may be omitted depending on configuration
        (eg. a read helper function that does not require an object passed to it will omit that parameter)
    
    "<>+": angle brackets with a "+" suffix denote areas that are repeated, they may contain one or more "|" characters,
        which denote the boundries of two unique possibilities that may be present

types:
    a type given in brackets does not necessarily follow the python 3.0 format for its literals

    [bool] - literal "true" or "false
"""
    topics: dict[str,str] = {
        "using-this":helphelp,
        "howto":helpprimer,
        "typedef":helptypedef,
        "macro":helpmacdef,
        "readwrite":helpreadwrite,
        "helperformat":helprwfmt,
        "misc":helpmisc
    }
    l = len(argv)
    if l == 2:
        print(helptopics)
    else:
        topic = argv[2].lower()
        print(topics.get(topic,helpbadtopic))
    exit()

import re

endpat = re.compile("}.*")

header: str = """
#ifndef __GENSIZES_H__
#define __GENSIZES_H__ 1
/*
THIS FILE IS GENERATED AUTOMATICALLY
run the python3 script "pygen.py" to regenerate this file
*/
"""

content: str = ""
secondarycontent: str = ""

# prefix for generated read-write functions
rwpre: str = ""

helperfmts: dict[str,tuple[str, str, str, str, str, str, str, bool, bool]] = {}

footer: str = "\n#endif"

lines: list[str] = None

with open("os/kernel/filesystems/fsdefs.h", "rt") as f:
    lines = f.readlines()

spacetab = "    "

macdefs: dict[str, int] = {}

sizings: dict[str,int] = {
    "u8":1,
    "s8":1,
    "char":1,
    "u16":2,
    "s16":2,
    "u32":4,
    "s32":4,
    "u48":6,
    "s48":6,
    "u64":8,
    "s64":8,
}
keys = sizings.keys()

GenFlags = tuple[bool, bool]

def gen_rw_helpers(fmt: str, classname: str, name: str, rwname: str, fieldnames: list[str], fieldsizes: list[int], flags: GenFlags, overd: dict[str, int], overl: list[tuple[str, int, dict[str, str]]]):
    global secondarycontent
    global rwpre
    objname = helperfmts[fmt][0]
    readpre = helperfmts[fmt][1]
    writepre = helperfmts[fmt][2]
    sbr = helperfmts[fmt][3]
    sbw = helperfmts[fmt][4]
    sbc = helperfmts[fmt][5]
    hfunc = helperfmts[fmt][6]
    def mksbop(f: str, s: int) -> str:
        return f.replace("{size}", str(s))
    cpos = 0
    objdats = ("","") if objname == None else (objname+"* ", "obj, ")
    wbuild = f"""
/*
GENERATED BY PYGEN
writes a [{classname}]
*/
void {rwpre}write_{rwname}({objdats[0]}{objdats[1]}{classname}* {rwname}) {{
{spacetab}unsigned char x[{name}];
{spacetab}unsigned char* xp = (unsigned char*)x;"""
    rbuild = f"""
/*
GENERATED BY PYGEN
reads a [{classname}]
*/
void {rwpre}read_{rwname}({objdats[0]}{objdats[1]}{classname}* {rwname}) {{
{spacetab}unsigned char x[{name}];
{spacetab}unsigned char* xp = (unsigned char*)x;
{spacetab}{readpre}read_buf({objdats[1]}xp, {name});"""
    hbuild = f"""
/*
GENERATED BY PYGEN
hashes a [{classname}]
*/
u64 {rwpre}hash_{rwname}({classname}* {rwname}) {{
{spacetab}unsigned char hbuf[%%BUFSIZE];
{spacetab}unsigned char* bufp = (unsigned char*)hbuf;"""
    checksumhandled = True
    hsize: int = 0
    tmpno: int = 0
    for odsc in overl:
        odsc = list(odsc)
        s1 = odsc[2].pop("write")
        used = list(map(str.strip, odsc[0].split(",")))
        for un in used:
            s1 = s1.replace(f"`{un}`", f"{rwname}->{un}")
        for s2 in odsc[2].keys():
            odsc[2][s2] = odsc[2][s2].replace("`$TMP`", f"tmpval{tmpno}")
        s = odsc[1]*8
        if odsc[1] == 1:
            rbuild += f"\n{spacetab}u8 tmpval{tmpno} = x[{cpos}];"
            wbuild += f"\n{spacetab}x[{cpos}] = {s1};"
            hbuild += f"\n{spacetab}hbuf[{cpos}] = {s1};"
        else:
            rbuild += f"\n{spacetab}u{s} tmpval{tmpno} = {mksbop(sbr, s)}(xp+{cpos});"
            wbuild += f"\n{spacetab}{mksbop(sbw, s)}(xp+{cpos}, {s1});"
            hbuild += f"\n{spacetab}{mksbop(sbw, s)}(bufp+{cpos}, {s1});"
        for fn in odsc[2].keys():
            rbuild += f"\n{spacetab}{rwname}->{fn} = {odsc[2][fn]};"
        cpos += odsc[1]
        tmpno += 1
    for i in range(len(fieldnames)):
        fn = fieldnames[i]
        fs = fieldsizes[i]
        if fs == 0:
            continue
        if checksumhandled and "checksum" in fn:
            checksumhandled = False
            wbuild += f"\n{spacetab}{rwname}->{fn} = {hfunc}(xp, {cpos});"
            hsize = cpos
        if fn == "-align":
            cpos += fs
            continue
        elif fs < 0:
            fs = -fs
            wbuild += f"\n{spacetab}{sbc}(xp+{cpos}, {rwname}->{fn}, {fs});"
            rbuild += f"\n{spacetab}{sbc}({rwname}->{fn}, xp+{cpos}, {fs});"
            hbuild += f"\n{spacetab}{sbc}(bufp+{cpos}, {rwname}->{fn}, {fs});"
        else:
            if fs == 1:
                wbuild += f"\n{spacetab}x[{cpos}] = {rwname}->{fn};"
                rbuild += f"\n{spacetab}{rwname}->{fn} = x[{cpos}];"
                hbuild += f"\n{spacetab}hbuf[{cpos}] = {rwname}->{fn};"
            else:
                s = fs * 8
                wbuild += f"\n{spacetab}{mksbop(sbw, s)}(xp+{cpos}, {rwname}->{fn});"
                rbuild += f"\n{spacetab}{rwname}->{fn} = {mksbop(sbr, s)}(xp+{cpos});"
                hbuild += f"\n{spacetab}{mksbop(sbw, s)}(bufp+{cpos}, {rwname}->{fn});"
        cpos += fs
    wbuild += f"\n{spacetab}{writepre}write_buf({objdats[1]}xp, {name});"
    wbuild += "\n}"
    rbuild += "\n}"
    hbuild += f"\n{spacetab}return {hfunc}(bufp, {hsize});"
    hbuild += "\n}"
    if flags[0]:
        secondarycontent += wbuild
        secondarycontent += rbuild
    if flags[1]:
        secondarycontent += hbuild.replace("%%BUFSIZE", str(hsize), 1)

def parsePygen(gens: list[str]):
    global header
    global content
    global rwpre
    l = len(gens)
    typedef: bool = False
    macdef: bool = False
    fmtdef: bool = False
    incl: bool = False
    comment: str = None
    name: str = None
    rwname: str = None
    gflags: list[bool] = [None] * 2
    fmt: str = "$default"
    override: dict[str, int] = {}
    overrideto: list[tuple[str, int, dict[str, str]]] = []
    overkeys = override.keys()
    sizesum = 0
    start = 0
    for i in range(l):
        line = gens[i].strip()
        if line == "pygen-end":
            return
        elif incl:
            header += f"#include {line}\n"
        elif line == "*/":
            start = i+1
            break
        elif line == "pygen-typedef":
            start = i+1
            typedef = True
            break
        elif line == "pygen-hf-fmt":
            start = i+1
            fmtdef = True
            break
        elif line == "pygen-macro":
            start = i+1
            macdef = True
            name = ""
            break
        elif line == "pygen-include":
            incl = True
            header += "\n"
        parts = line.split(":", 1)
        for j in range(len(parts)):
            parts[j] = parts[j].strip()
        if parts[0] == "pygen-mk-rw":
            rwname = parts[1]
        elif parts[0] == "pygen-rw-pre":
            rwpre = parts[1]
        elif parts[0] == "pygen-hf-use":
            fmt = parts[1]
        elif parts[0] == "comment":
            comment = parts[1]
        elif parts[0] == "name":
            name = parts[1]
        elif parts[0] == "pygen-over":
            ix = parts[1][1:].split("] ", 1)
            iy = ix[1].split(" ", 1)
            sv = int(iy[0])
            d: dict[str, str] = {}
            i += 1
            line = gens[i].strip()
            while line != "}":
                d[line.split(":", 1)[0]] = line.split(":", 1)[1].strip()
                i += 1
                line = gens[i].strip()
            overrideto.append((ix[0], sv, d))
            sizesum += sv
            for n in ix[0].split(","):
                override[n.strip()] = 0
        elif parts[0].startswith("pygen-force-"):
            opt = parts[0][12:]
            v = False if parts[1] == "false" else True
            if opt == "rw":
                gflags[0] = v
            elif opt == "hash":
                gflags[1] = v
            elif opt == "all":
                for j in range(len(gflags)):
                    gflags[j] = v
    if incl:
        return
    if fmtdef:
        fmtnames: list[str] = []
        vals: list[str] = [None] * 7 + [True] * 2
        for i in range(start, l):
            line = gens[i].strip()
            if line == "pygen-end":
                if vals[1] == "$none":
                    vals[1] = ""
                if vals[2] == "$none":
                    vals[2] = ""
                vals = tuple(vals)
                for fmtname in fmtnames:
                    helperfmts[fmtname] = vals
                return
            parts = line.split(":", 1)
            for j in range(len(parts)):
                parts[j] = parts[j].strip()
            if parts[0] == "name":
                fmtnames.append(parts[1])
            elif parts[0] == "readerpre":
                vals[1] = parts[1]
            elif parts[0] == "writerpre":
                vals[2] = parts[1]
            elif parts[0] == "objname":
                vals[0] = parts[1]
            elif parts[0] == "sbufread":
                vals[3] = parts[1]
            elif parts[0] == "sbufwrite":
                vals[4] = parts[1]
            elif parts[0] == "bufcpy":
                vals[5] = parts[1]
            elif parts[0] == "hashf":
                vals[6] = parts[1]
            elif parts[0] == "copy":
                vals = list(helperfmts[parts[1]])
            elif parts[0].startswith("default-mk-"):
                opt = parts[0][11:]
                v = False if parts[1] == "false" else True
                if opt == "rw":
                    vals[7] = v
                elif opt == "hash":
                    vals[8] = v
                elif opt == "all":
                    for j in range(7, len(vals)):
                        vals[j] = v
        raise SyntaxError("pygen helper format def not ended")
    if name == None:
        raise ValueError("UNNAMED PYGEN")
    fieldnames: list[str] = []
    fieldsizes: list[int] = []
    classname: str = None
    for i in range(start, l):
        if typedef and gens[i].strip() == "pygen-end":
            sizings[name] = sizesum
            return
        if endpat.match(gens[i]) != None:
            classname = gens[i].strip()[1:-1].strip()
            sizings[classname] = sizesum
            break
        line = gens[i].strip().split()
        if len(line) == 0:
            continue
        if macdef and line[0] == "#define":
            macdefs[line[1]] = int(line[2], base=0)
            return
        if line[0] == "unsigned":
            line.pop(0)
        if line[0].endswith("*"):
            line[0] = line[0][:-1]
        if line[0] not in keys:
            continue
        if line[1].endswith(';'):
            line[1] = line[1][:-1]
        sv = 0
        if "[" in line[1]:
            x = line[1]
            s = x[x.index("[")+1:x.index("]")]
            line[1] = line[1][:line[1].index("[")]
            if s in macdefs:
                sv = sizings[line[0]] * macdefs[s]
            else:
                sv = sizings[line[0]] * int(s)
            # make array type fields have negative size to remember to use awrite_buf when generating rw functions
            sv *= -1
        elif line[0] == "pygen-fake":
            sizesum += int(line[1])
            continue
        elif line[0] == "pygen-align":
            sa = (8 - (sizesum % 8))
            sizesum += sa
            fieldnames.append("-align")
            fieldsizes.append(sa)
            continue
        else:
            sv = sizings[line[0]]
        if line[1] in overkeys:
            override[line[1]] = sv
            continue
        fieldnames.append(line[1])
        fieldsizes.append(sv)
        sizesum += abs(sv)
    if comment:
        content += f"\n// {comment}"
    content += f"\n#define {name} {sizesum}"
    if rwname != None:
        for i in range(len(gflags)):
            if gflags[i] == None:
                gflags[i] = helperfmts[fmt][i+7]
        gen_rw_helpers(fmt, classname, name, rwname, fieldnames, fieldsizes, gflags, override, overrideto)

l = len(lines)
for i in range(l):
    if lines[i].strip() == "PYGENSTART":
        parsePygen(lines[i:])

content += "\n"

if "-nowrite" not in argv:
    with open("os/kernel/filesystems/gensizes.h", "wt") as f:
        f.truncate()
        f.write(header+content+secondarycontent+footer)

if "-dbgout" in argv:
    print(header, content, secondarycontent, footer, sep='')