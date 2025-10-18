#include "task_lib.h"

#define WIDTH TASK_VIEWPORT_WIDTH
#define HEIGHT TASK_VIEWPORT_HEIGHT

#define SHARED_SCORE_BASE_VADDR (PAGE_ON_DEMAND_BASE_VADDR + 0xF00)
#define CANT_PONGS 3


void task(void) {
	screen pantalla;
	// ¿Una tarea debe terminar en nuestro sistema?
	while (true)
	{
	// - Pueden definir funciones auxiliares para imprimir en pantalla
	// - Pueden usar `task_print`, `task_print_dec`, etc. 

		for (uint8_t i = 0; i < CANT_PONGS; i++){
			uint32_t* score_p1_ptr = (uint32_t*) (SHARED_SCORE_BASE_VADDR + ((uint32_t) i * sizeof(uint32_t)*2));
			uint32_t score_p1 = *score_p1_ptr;
			uint32_t* score_p2_ptr = (uint32_t*) (SHARED_SCORE_BASE_VADDR + ((uint32_t) i * sizeof(uint32_t)*2) + 4);
			uint32_t score_p2 = *score_p2_ptr;

			task_print(pantalla, "Partida: ", 13, 4 * i + 6, 15);
			task_print_dec(pantalla, i + 1, 1, 23,  4 * i + 6, 15);
			task_print(pantalla, "Score P1: ", 13, 4 * i + 7, 15);
			task_print_dec(pantalla, score_p1, 2, 23,  4 * i + 7, 15);
			task_print(pantalla, "Score P2: ", 13, 4 * i + 8, 15);
			task_print_dec(pantalla, score_p2, 2, 23,  4 * i + 8, 15);
		}
		
		syscall_draw(pantalla);
	}
}
