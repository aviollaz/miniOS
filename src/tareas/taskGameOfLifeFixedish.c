#include "task_lib.h"

#define WIDTH TASK_VIEWPORT_WIDTH
#define HEIGHT TASK_VIEWPORT_HEIGHT

typedef bool gol_grid[HEIGHT][WIDTH];

int miii = 0;
int miii2 = 0;

/**
 * Avanzo la simulación un paso
 */
void step(gol_grid gol) {
	gol_grid next_gol;
	miii+=1;
	miii2+=2;

	for (int i = 0; i < HEIGHT; i++) {
		for (int j = 0; j < WIDTH; j++) {
			int izquierda = (j + WIDTH - 1) % WIDTH;
			int derecha   = (j + 1) % WIDTH;
			int arriba    = (i + HEIGHT - 1) % HEIGHT;
			int abajo     = (i + 1) % HEIGHT;
			int vecinos_vivos =
			                    gol[     i][  derecha]
			                  + gol[     i][izquierda]
			                  + gol[arriba][        j]
			                  + gol[arriba][  derecha]
			                  + gol[arriba][izquierda]
			                  + gol[ abajo][        j]
			                  + gol[ abajo][  derecha]
			                  + gol[ abajo][izquierda];
			next_gol[i][j] = (vecinos_vivos < 4 && vecinos_vivos > 1 && gol[i][j]) || vecinos_vivos == 3 ;
		}
	}

	for (int i = 0; i < HEIGHT; i++) {
		for (int j = 0; j < WIDTH; j++) {
			gol[i][j] = next_gol[i][j];
		}
	}
}

/**
 * Armo un nuevo estado inicial
 */
void randomize(gol_grid gol) {
	int random_state = ENVIRONMENT->tick_count + ENVIRONMENT->task_id * 7;
	for (int i = 0; i < HEIGHT; i++) {
		for (int j = 0; j < WIDTH; j++) {
			gol[i][j] = task_random(&random_state) % 2;
		}
	}
}

/**
 * Implementación a simplista del juego de la vida de Conway
 */
void task() {
	screen pantalla;
	gol_grid gol;

	randomize(gol);
	int sleep_time = 50;//1;

	while (!ENVIRONMENT->keyboard.escape) {
		syscall_draw(pantalla);

		task_print(pantalla, "test shared", 1,2,C_FG_CYAN);
		task_print_dec(pantalla, ENVIRONMENT->tick_count,10, 1, 3, C_FG_CYAN);
		task_print_dec(pantalla, ENVIRONMENT->task_id,10, 1, 4, C_FG_CYAN);
		task_print(pantalla, "mii,&mii,mii2,&mii", 1,6,C_FG_CYAN);
		task_print_hex(pantalla, miii, 1,7,C_FG_CYAN);;
		task_print_hex(pantalla, (uint32_t)(&miii), 1,8,C_FG_CYAN);
		task_print_hex(pantalla, miii2, 1,9,C_FG_CYAN);
		task_print_hex(pantalla, (uint32_t)(&miii2), 1,10,C_FG_CYAN); // en final del stack. no se como el compilador se da cuenta de ponerlo ahi 
		task_print(pantalla, "el juego de la vida", 1,11,C_FG_CYAN);
		task_print(pantalla, "anterior tenia mal", 1,12,C_FG_CYAN);
		task_print(pantalla, "definidas las reglas", 1,13,C_FG_CYAN);
		task_print(pantalla, "????", 1,14,C_FG_CYAN);
		task_print(pantalla, "r=reset", 1,16,C_FG_CYAN);
		task_print(pantalla, "up=random", 1,17,C_FG_CYAN);
		task_print(pantalla, "down=glider", 1,18,C_FG_CYAN);
		task_print(pantalla, "left=fast", 1,19,C_FG_CYAN);
		task_print(pantalla, "right=slow", 1,20,C_FG_CYAN);
		task_sleep(sleep_time);


		for (int i = 0; i < HEIGHT; i++) {
			for (int j = 0; j < WIDTH; j++) {
				pantalla[i][j] = (ca){
					.c = pantalla[i][j].c,
					.a = gol[i][j]
					   ? ( C_BG_LIGHT_GREY | C_FG_BLUE )
					   : (C_BG_BLACK | C_FG_CYAN )
				};
			}
		}
		step(gol);
		syscall_draw(pantalla);
		
		if (ENVIRONMENT->keyboard.up) {
			randomize(gol);
		} else if (ENVIRONMENT->keyboard.left) {
			sleep_time = 1;
		} else if (ENVIRONMENT->keyboard.right) {
			sleep_time = 50;
		} else if (ENVIRONMENT->keyboard.down) {
			
			gol[17][15] = true;
			gol[17][16] = true;
			gol[17][17] = true;
			gol[16][17] = true;
			gol[15][16] = true;
		} else if (ENVIRONMENT->keyboard.r) {
				for (int i = 0; i < HEIGHT; i++) 
					for (int j = 0; j < WIDTH; j++) 
						gol[i][j] = false;

		}
	}
}
