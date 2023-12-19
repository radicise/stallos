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

with open("FSMOCKFILE.mock", "rb") as f:
    b = f.read(numbytes)
    ba = bytearray(b)
    print(ba.hex(" "))
    print(list(ba))