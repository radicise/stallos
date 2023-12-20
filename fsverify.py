import sys

if len(sys.argv) == 1:
    print("NOT ENOUGH ARGS")
    exit(0)

if not sys.argv[1].isdigit():
    print("BAD INPUT")
    exit(0)

numbytes = int(sys.argv[1])

if numbytes > 1024:
    print("TOO LONG")
    exit(0)

p = 0

def dhash(ba: bytearray) -> int:
    print(ba.hex(" "))
    h = 0
    for c in ba:
        h = c + (h << 6) + (h << 16) - h
        h = h & 0xffffffffffffffff
    return h

with open("FSMOCKFILE.mock", "rb") as f:
    b = f.read(numbytes)
    ba = bytearray(b)
    print(hex(dhash(ba[0:-int(sys.argv[2])])))
    for s in sys.argv[3:]:
        l = int(s)
        bs = ba[p:p+l]
        p += l
        print(bs.hex(" "))
        # print(list(bs))