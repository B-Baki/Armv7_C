"use strict";

const PB = 0xc8000000;
const PS2 = 0xff200100;
const WIDTH = 320;
const HEIGHT = 240;
const STRIDE = 1024;

const BY_CODE = {
  KeyW: 0x1d, ArrowUp: 0x1d,
  KeyA: 0x1c, ArrowLeft: 0x1c,
  KeyS: 0x1b, ArrowDown: 0x1b,
  KeyD: 0x23, ArrowRight: 0x23,
  Space: 0x29,
  KeyN: 0x31,
};

const BY_KEY = {
  w: 0x1d, arrowup: 0x1d,
  a: 0x1c, arrowleft: 0x1c,
  s: 0x1b, arrowdown: 0x1b,
  d: 0x23, arrowright: 0x23,
  " ": 0x29,
  n: 0x31,
};

const statusEl = document.getElementById("status");
const rateEl = document.getElementById("rate");
const sourceEl = document.getElementById("source");
const canvas = document.getElementById("screen");
const ctx = canvas.getContext("2d", { alpha: false });
const image = ctx.createImageData(WIDTH, HEIGHT);
const pauseBtn = document.getElementById("pause");
const resetBtn = document.getElementById("reset");

let uc = null;
let engine = null;
let entry = 0;
let segments = [];

let fifo = [];
let staged = false;
let running = true;
let booted = false;
let seenCursor = false;
let fault = null;
let executed = 0;
let raf = 0;
let redMinX = 0;
let redMaxX = 0;
let redMinY = 0;
let redMaxY = 0;
let generation = 0;
let rateMarkT = 0;
let rateMarkN = 0;

function u32(x) {
  if (typeof x === "bigint") return Number(BigInt.asUintN(32, x));
  return x >>> 0;
}

function setStatus(text) {
  if (statusEl.textContent !== text) statusEl.textContent = text;
}

function loadElf(buf) {
  const bytes = new Uint8Array(buf);
  const view = new DataView(buf);
  if (bytes[0] !== 0x7f || bytes[1] !== 0x45) throw new Error("part3.elf is not an ELF file");
  const elfEntry = view.getUint32(0x18, true);
  const phoff = view.getUint32(0x1c, true);
  const phentsize = view.getUint16(0x2a, true);
  const phnum = view.getUint16(0x2c, true);
  const loaded = [];
  for (let i = 0; i < phnum; i++) {
    const off = phoff + i * phentsize;
    if (view.getUint32(off, true) !== 1) continue;
    const offset = view.getUint32(off + 4, true);
    const vaddr = view.getUint32(off + 8, true);
    const filesz = view.getUint32(off + 16, true);
    loaded.push({ vaddr, bytes: bytes.slice(offset, offset + filesz) });
  }
  return { entry: elfEntry, segments: loaded };
}

function writeReg(word) {
  engine.mem_write(PS2, new Uint8Array([
    word & 255,
    (word >>> 8) & 255,
    (word >>> 16) & 255,
    (word >>> 24) & 255,
  ]));
}

function stageNext() {
  if (fifo.length === 0) {
    writeReg(0);
    staged = false;
  } else {
    writeReg((fifo[0] & 255) | 0x8000);
    staged = true;
  }
}

function enqueue(byte) {
  if (!engine || fault) return;
  fifo.push(byte & 255);
  if (byte === 0x31) generation += 1;
  if (!staged) stageNext();
  if (!running) {
    running = true;
    pauseBtn.textContent = "Pause";
    kick();
  }
}

function onPs2(_handle, _type, address) {
  try {
    if (u32(address) !== PS2 || !staged) return;
    fifo.shift();
    staged = false;
    stageNext();
  } catch (err) {
    fault = err;
  }
}

function zeroRange(addr, len) {
  const chunk = new Uint8Array(4096);
  for (let off = 0; off < len; off += 4096) {
    const n = Math.min(4096, len - off);
    engine.mem_write(addr + off, n === 4096 ? chunk : chunk.subarray(0, n));
  }
}

function loadImage() {
  for (const seg of segments) engine.mem_write(seg.vaddr, seg.bytes);
  zeroRange(PB, 0x40000);
  zeroRange(0xc9000000, 0x10000);
  writeReg(0);
  fifo = [];
  staged = false;
  booted = false;
  seenCursor = false;
  generation = 0;
  fault = null;
  executed = 0;
  rateMarkN = 0;
  rateMarkT = performance.now();
}

function mapMachine() {
  engine.mem_map(0x00000000, 0x200000, uc.PROT_ALL);
  engine.mem_map(PB, 0x40000, uc.PROT_ALL);
  engine.mem_map(0xc9000000, 0x10000, uc.PROT_ALL);
  engine.mem_map(0xff200000, 0x1000, uc.PROT_ALL);
  engine.hook_add(uc.HOOK_MEM_READ_AFTER, onPs2, {}, PS2, PS2 + 3);
}

function paint() {
  const fb = engine.mem_read(PB, HEIGHT * STRIDE);
  const px = image.data;
  redMinX = WIDTH;
  redMaxX = -1;
  redMinY = HEIGHT;
  redMaxY = -1;
  let o = 0;
  for (let y = 0; y < HEIGHT; y++) {
    let base = y * STRIDE;
    for (let x = 0; x < WIDTH; x++) {
      const c = fb[base] | (fb[base + 1] << 8);
      base += 2;
      if (c === 0xf800) {
        seenCursor = true;
        if (x < redMinX) redMinX = x;
        if (x > redMaxX) redMaxX = x;
        if (y < redMinY) redMinY = y;
        if (y > redMaxY) redMaxY = y;
      }
      const r = (c >>> 11) & 31;
      const g = (c >>> 5) & 63;
      const b = c & 31;
      px[o++] = (r << 3) | (r >>> 2);
      px[o++] = (g << 2) | (g >>> 4);
      px[o++] = (b << 3) | (b >>> 2);
      px[o++] = 255;
    }
  }
  ctx.putImageData(image, 0, 0);
}

function updateRate() {
  const now = performance.now();
  const dt = now - rateMarkT;
  if (dt < 200) return;
  const rate = (executed - rateMarkN) * 1000 / dt;
  rateMarkT = now;
  rateMarkN = executed;
  const text = rate >= 1e6
    ? (rate / 1e6).toFixed(1) + "M/s"
    : Math.round(rate / 1e3) + "K/s";
  if (rateEl.textContent !== text) rateEl.textContent = text;
}

function step(budgetMs) {
  const t0 = performance.now();
  let begin = booted ? u32(engine.reg_read_i32(uc.ARM_REG_PC)) : entry;
  if (begin === 0) begin = entry;
  for (let n = 0; n < 8; n++) {
    if (n > 0 && performance.now() - t0 >= budgetMs) break;
    engine.emu_start(begin, 0xffffffff, 0, 50000);
    executed += 50000;
    booted = true;
    begin = u32(engine.reg_read_i32(uc.ARM_REG_PC));
  }
}

function frame() {
  raf = 0;
  if (!running || !engine) return;
  try {
    if (!fault) step(12);
  } catch (err) {
    fault = err;
    running = false;
  }
  try {
    paint();
  } catch (err) {
    fault = fault || err;
    running = false;
  }
  if (fault) setStatus(String(fault.message || fault));
  else if (!seenCursor || redMaxX < 0) setStatus("Drawing the board.");
  else {
    const column = Math.round(((redMinX + redMaxX) / 2 - 10) / 20);
    const row = Math.round(((redMinY + redMaxY) / 2 - 9) / 20);
    setStatus("Column " + column + ", row " + row + (generation ? " · generation " + generation : ""));
  }
  updateRate();
  if (running) raf = requestAnimationFrame(frame);
  else rateEl.textContent = "paused";
}

function kick() {
  if (!raf && running && engine && !fault) raf = requestAnimationFrame(frame);
}

function resetCpu() {
  loadImage();
  running = true;
  pauseBtn.textContent = "Pause";
  setStatus("Drawing the board.");
  kick();
}

function enableControls() {
  for (const button of document.querySelectorAll("button")) button.disabled = false;
}

function scancode(event) {
  if (event.ctrlKey || event.metaKey || event.altKey) return undefined;
  if (Object.prototype.hasOwnProperty.call(BY_CODE, event.code)) return BY_CODE[event.code];
  if (event.key) return BY_KEY[event.key.toLowerCase()];
  return undefined;
}

async function main() {
  setStatus("Loading the ARM emulator.");
  const [mod, elfBuf, prog, cText] = await Promise.all([
    MUnicorn(),
    fetch("part3.elf").then((r) => {
      if (!r.ok) throw new Error("Could not load part3.elf");
      return r.arrayBuffer();
    }),
    fetch("program.json").then((r) => {
      if (!r.ok) throw new Error("Could not load program.json");
      return r.json();
    }),
    fetch("part3.c").then((r) => r.ok ? r.text() : ""),
  ]);
  uc = mod;
  const elf = loadElf(elfBuf);
  entry = elf.entry;
  segments = elf.segments;
  sourceEl.textContent = prog.source;
  document.getElementById("c-source").textContent = cText;
  engine = new uc.Unicorn(uc.ARCH_ARM, uc.MODE_ARM);
  mapMachine();
  loadImage();
  enableControls();
  setStatus("Drawing the board.");
  kick();
}

pauseBtn.addEventListener("click", () => {
  running = !running;
  pauseBtn.textContent = running ? "Pause" : "Run";
  if (running) kick();
  else {
    setStatus("Paused.");
    rateEl.textContent = "paused";
  }
});

resetBtn.addEventListener("click", () => {
  if (engine) resetCpu();
});

for (const button of document.querySelectorAll("[data-code]")) {
  button.addEventListener("click", () => {
    if (button.disabled) return;
    enqueue(parseInt(button.dataset.code, 16));
  });
}

window.addEventListener("keydown", (event) => {
  const code = scancode(event);
  if (code === undefined) return;
  event.preventDefault();
  if (event.repeat) return;
  enqueue(code);
}, true);

main().catch((err) => {
  setStatus(String(err.message || err));
});
