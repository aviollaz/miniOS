/* ** por compatibilidad se omiten tildes **
================================================================================
 TALLER System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================

  Rutinas del controlador de interrupciones.
*/
#include "pic.h"

#define PIC1_PORT 0x20
#define PIC2_PORT 0xA0

static __inline __attribute__((always_inline)) void outb(uint32_t port,
                                                         uint8_t data) {
  __asm __volatile("outb %0,%w1" : : "a"(data), "d"(port));
}
void pic_finish1(void) { outb(PIC1_PORT, 0x20); }
void pic_finish2(void) {
  outb(PIC1_PORT, 0x20);
  outb(PIC2_PORT, 0x20);
}

// implementar pic_reset()
void pic_reset() {
  outb(PIC1_PORT, 0x11);    // inicializar en modo cascada
  outb(PIC1_PORT + 1, 0x20);// empezar a mappear a partir de 32
  outb(PIC1_PORT + 1, 0x4); // pic2 es tu esclavo
  outb(PIC1_PORT + 1, 0x1); // settear en modo 8086 (no buffered)
  //outb(PIC1_PORT + 1, 0xFF);//empezar con el teclado no enmascarado
  
  outb(PIC2_PORT, 0x11);    // inicializar en modo casacada
  outb(PIC2_PORT + 1, 0x28);// empieza a mappear a partir de 40
  outb(PIC2_PORT + 1, 0x2); // puerto 2 es master
  outb(PIC2_PORT + 1, 0x1); // settear en modo 8086 (no buffered)

}

void pic_enable() {
  outb(PIC1_PORT + 1, 0x00);
  outb(PIC2_PORT + 1, 0x00);
}

void pic_disable() {
  outb(PIC1_PORT + 1, 0xFF);
  outb(PIC2_PORT + 1, 0xFF);
}
