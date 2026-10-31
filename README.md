# miniOS

A small 32-bit x86 operating system written from scratch in C and assembly: a
bootloader-loaded kernel that switches from real mode to protected mode, sets up
segmentation, interrupts and paging, and runs **four user tasks concurrently** with a
preemptive scheduler.

Built for the System Programming workshop of *Arquitectura y Organización de
Computadoras* (Computer Science, University of Buenos Aires), on top of the course's
starter skeleton.

## What it does

- **Boot and mode switch.** `kernel.asm` enables the A20 line, loads a GDT and jumps
  from 16-bit real mode into 32-bit protected mode.
- **Segmentation.** Flat code and data segments for ring 0 (kernel) and ring 3 (user
  tasks), plus one TSS descriptor per task (`gdt.c`, `tss.c`).
- **Interrupts.** An IDT with handlers for CPU exceptions, the timer (IRQ 0) and the
  keyboard (IRQ 1), with the 8259 PIC remapped so hardware interrupts don't collide
  with exceptions (`idt.c`, `isr.asm`, `pic.c`).
- **Paging.** Identity-mapped kernel memory and a separate page directory per task,
  so each task runs in its own address space (`mmu.c`). A page-fault handler maps a
  shared memory area **on demand** the first time a task touches it.
- **Preemptive scheduling.** On every clock tick the scheduler picks the next runnable
  task and switches to it through its TSS (`sched.c`). Tasks can be paused and resumed.
- **System calls.** Tasks run in ring 3 and draw to their region of the screen through
  a software interrupt (`int 88`) instead of touching video memory directly.
- **User tasks.** The default build runs three instances of a "Matrix" rain animation
  and one Game of Life, side by side. Pong, Snake and a typing test are also in
  `src/tareas/` and can be swapped in from the `Makefile` (`TASKA`/`TASKB`).

## Project layout

| File | Responsibility |
|---|---|
| `kernel.asm` | Entry point, A20, protected-mode switch, initialization order |
| `gdt.c` / `gdt.h` | Global Descriptor Table |
| `idt.c`, `isr.asm`, `pic.c` | Interrupt table, handlers and PIC setup |
| `mmu.c` | Page directories, page mapping and the page-fault handler |
| `tss.c`, `sched.c`, `tasks.c` | Task state segments, scheduler and task creation |
| `keyboard_input.c`, `screen.c` | Keyboard decoding and text-mode output |
| `tareas/` | User-space tasks and their small runtime (`task_lib.h`, `syscall.h`) |

## Building and running

Requirements (Linux): `gcc` with 32-bit support (`gcc-multilib`), `nasm`,
`qemu-system-i386`, `gdb`, `mtools` and `bzip2`.

```bash
cd src
make gdb
```

This builds the kernel and tasks, writes them to a floppy image, starts QEMU paused
and attaches GDB to it. Type `continue` (or `c`) in GDB to boot. To rebuild only the
image without running it, use `make image`.
