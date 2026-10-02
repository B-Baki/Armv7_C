# ARMv7 Game of Life on the DE1-SoC

Freestanding **ARMv7-A** C with **handwritten assembly drivers** for the Terasic DE1-SoC: Conway's Game of Life on a VGA grid, controlled over PS/2 — plus a browser demo that runs the same ELF in [Unicorn](https://www.unicorn-engine.org/).

**[Live browser demo](https://b-baki.github.io/Armv7_C/)** · [McGill DE1-SoC simulator](https://ecse324.ece.mcgill.ca/simulator/?sys=arm-de1soc)

---

## What it is

A bare-metal Game of Life for the DE1-SoC FPGA board's HPS/ARM side:

| Piece | Detail |
| --- | --- |
| Board | 12×16 cells, 20×20 px each, drawn into the VGA pixel buffer |
| Controls | **WASD** move cursor · **SPACE** toggle cell · **N** next generation |
| Graphics | Pixel buffer (`0xC8000000`) + character buffer (`0xC9000000`) |
| Input | PS/2 data register (`0xFF200100`), scancode handling including break codes |
| Runtime | Freestanding: custom `startup.s`, linker script, no libc |

The main program is `part3.c` (name kept for the build flow). VGA draw/clear, line drawing, and PS/2 read are implemented as inline ARMv7 assembly in that file; `startup.s` provides `_start`, BSS clear, and minimal `memcpy`/`memset` Aeabi helpers so GCC can link without a C library.

---

## Tech stack

![ARM](https://img.shields.io/badge/ISA-ARMv7--A-blue)
![C](https://img.shields.io/badge/Language-C%20%2B%20ASM-informational)
![Target](https://img.shields.io/badge/Target-DE1--SoC-green)
![Tool](https://img.shields.io/badge/Toolchain-arm--none--eabi--gcc-lightgrey)
![Demo](https://img.shields.io/badge/Demo-Unicorn.js-orange)

---

## Architecture

```text
startup.s          freestanding entry, BSS, Aeabi helpers
      │
part3.c            GoL logic + cursor/keyboard loop (C)
      │
inline ASM         VGA pixel/char drivers, draw_line, read_PS2
      │
DE1-SoC MMIO       pixel buff · char buff · PS/2
```

`build.sh` cross-compiles with `arm-none-eabi-gcc` (`-ffreestanding -nostdlib`), writes `docs/part3.elf`, extracts the handwritten driver listing, and regenerates `docs/program.json` for the Unicorn demo.

---

## Try it

### Browser (no hardware)

Open the [GitHub Pages demo](https://b-baki.github.io/Armv7_C/) — or open `docs/index.html` locally after a build. The page loads the ELF via Unicorn and mirrors the VGA framebuffers.

### Rebuild the ELF / demo assets

```bash
# Needs arm-none-eabi-gcc on PATH (or under ~/.local/arm-none-eabi/usr/bin)
./build.sh
```

Produces `docs/part3.elf`, `docs/drivers.s`, `docs/program.json`, and a copy of `part3.c` under `docs/`.

### On the DE1-SoC / McGill simulator

1. Build with `./build.sh`.
2. Load `docs/part3.elf` in the [McGill arm-de1soc simulator](https://ecse324.ece.mcgill.ca/simulator/?sys=arm-de1soc) (URL also in the `site` file), or on board hardware that maps the same MMIO.

---

## Repository layout

```text
part3.c       Game of Life + ASM drivers (main program)
startup.s     Reset entry, BSS, memcpy/memset helpers
linker.ld     Memory map for freestanding link
build.sh      Cross-build + docs/program.json regeneration
docs/         GitHub Pages demo (index.html, Unicorn, ELF)
site          Link to the McGill DE1-SoC simulator
```
