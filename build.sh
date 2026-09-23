#!/bin/sh
# Rebuild docs/part3.elf, docs/drivers.s, and docs/program.json.
# Needs arm-none-eabi-gcc (a local extract under ~/.local/arm-none-eabi is fine).
set -eu

cd "$(dirname "$0")"

if ! command -v arm-none-eabi-gcc >/dev/null 2>&1; then
  if [ -x "$HOME/.local/arm-none-eabi/usr/bin/arm-none-eabi-gcc" ]; then
    PATH="$HOME/.local/arm-none-eabi/usr/bin:$PATH"
    export PATH
  else
    echo "arm-none-eabi-gcc not found" >&2
    exit 1
  fi
fi

mkdir -p docs

arm-none-eabi-gcc -marm -march=armv7-a -mtune=cortex-a9 -mfloat-abi=soft \
  -ffreestanding -fno-builtin -fno-stack-protector -fno-pic \
  -O1 -Wall -nostdlib -nostartfiles \
  -Wl,-T,linker.ld -Wl,--no-warn-rwx-segments \
  startup.s part3.c -o docs/part3.elf

arm-none-eabi-nm -n docs/part3.elf > docs/part3.nm
arm-none-eabi-objdump -d docs/part3.elf > docs/part3.dis

python3 - << 'PY'
import json, re
from pathlib import Path

root = Path(".")
src = (root / "part3.c").read_text()
start = src.index("__asm__")
i = src.index("(", start) + 1
chunks = []
while True:
    while i < len(src) and src[i] != '"':
        if src.startswith(");", i):
            i = len(src)
            break
        i += 1
    if i >= len(src) or src[i] != '"':
        break
    i += 1
    chars = []
    while i < len(src):
        if src[i] == "\\":
            n = src[i + 1]
            chars.append({"n": "\n", "t": "\t", "r": "\r", "\\": "\\", '"': '"'}.get(n, n))
            i += 2
        elif src[i] == '"':
            i += 1
            break
        else:
            chars.append(src[i])
            i += 1
    chunks.append("".join(chars))

asm = "".join(chunks)
lines = []
for line in asm.split("\n"):
    if line.startswith("\t"):
        line = line[1:]
    lines.append(line.rstrip())
while lines and lines[0] == "":
    lines.pop(0)
while lines and lines[-1] == "":
    lines.pop()
drivers = "\n".join(lines) + "\n"
(root / "docs" / "drivers.s").write_text(drivers)
labels = set(re.findall(r"^([A-Za-z_][A-Za-z0-9_]*):", drivers, re.M))

symbols = []
seen = {}
for line in (root / "docs" / "part3.nm").read_text().splitlines():
    parts = line.split()
    if len(parts) < 3:
        continue
    addr_s, kind, name = parts[0], parts[1], parts[2]
    if kind.lower() != "t":
        continue
    addr = int(addr_s, 16)
    hand = name in labels
    prev = seen.get(addr)
    if prev is None or (prev.startswith("__") and not name.startswith("__")):
        seen[addr] = name
        if prev is not None:
            symbols = [s for s in symbols if s["addr"] != addr]
        symbols.append({"addr": addr, "name": name, "hand": hand})

symbols.sort(key=lambda s: s["addr"])
hand_addrs = [s["addr"] for s in symbols if s["hand"]]
if not hand_addrs:
    raise SystemExit("no handwritten symbols found in the ELF")
hand_lo = min(hand_addrs)
last_hand = max(hand_addrs)
hand_hi = None
for s in symbols:
    if s["addr"] > last_hand:
        hand_hi = s["addr"]
        break
if hand_hi is None:
    hand_hi = last_hand + 4

insns = []
line_re = re.compile(r"^\s+([0-9a-f]+):\s+([0-9a-f]{8})\s+(.*?)\s*$")
for line in (root / "docs" / "part3.dis").read_text().splitlines():
    m = line_re.match(line)
    if not m:
        continue
    text = re.sub(r"\s+", " ", m.group(3)).strip()
    insns.append({
        "addr": int(m.group(1), 16),
        "hex": m.group(2),
        "text": text,
    })

elf = (root / "docs" / "part3.elf").read_bytes()
if elf[:4] != b"\x7fELF":
    raise SystemExit("part3.elf is not an ELF")
entry = int.from_bytes(elf[0x18:0x1c], "little")

prog = {
    "entry": entry,
    "handLo": hand_lo,
    "handHi": hand_hi,
    "symbols": symbols,
    "insns": insns,
    "source": drivers,
}
(root / "docs" / "program.json").write_text(json.dumps(prog, separators=(",", ":")))
print(f"entry 0x{entry:x}  insns {len(insns)}  hand 0x{hand_lo:x}-0x{hand_hi:x}")
PY

cp part3.c docs/part3.c
rm -f docs/part3.nm docs/part3.dis part3.elf
echo "built docs/part3.elf"
