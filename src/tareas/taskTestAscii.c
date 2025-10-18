#include "task_lib.h"

#define WIDTH TASK_VIEWPORT_WIDTH
#define HEIGHT TASK_VIEWPORT_HEIGHT

typedef bool gol_grid[HEIGHT][WIDTH];

int miii = 0;


/**
 * Implementación a simplista del juego de la vida de Conway
 */


 static void task_print_hex_len(screen output, uint32_t numero, uint32_t size,
	uint32_t x, uint32_t y, uint8_t attr) {
	uint8_t letras[16] = "0123456789ABCDEF";

	for (uint32_t i = 0; i < size; i++) {
		uint32_t resto = numero % 16;
		numero = numero / 16;
		output[y][x + size - i - 1] = (ca) {
		.c = letras[resto],
		.a = attr
	};
	}
}

void task() {
	screen pantalla;

	int iters=0;
	for (int j = 0; j < WIDTH && iters<256; j++) {
		for (int i = 0; i < HEIGHT && iters<256; i++) {
			// print hex no te permite indicar longitud. 
			task_print_hex_len(pantalla, iters, 2, j*3, i, C_FG_GREEN);
			char print[] = {iters,'\0'};
			task_print(pantalla, print, j*3+2, i, C_FG_WHITE);
			iters++;
		}
	}

	syscall_draw(pantalla);
}

// los carcateres de control, salvo 00, tienen interpretacion grafica
// 0x00, 0x20, 0x5F, 0xFF son los unicos caracteres vacios 
