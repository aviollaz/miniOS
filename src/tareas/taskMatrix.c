#include "task_lib.h"

#define WIDTH TASK_VIEWPORT_WIDTH
#define HEIGHT TASK_VIEWPORT_HEIGHT
#define SHARED_CONTINUOUS_MATRIX (PAGE_ON_DEMAND_BASE_VADDR + 0xA00)


typedef struct {
	uint8_t onoff;
	uint8_t isfirstchar;
	uint8_t duration;
} status ;

typedef status colstatus[WIDTH];


/**
 * Armo un nuevo estado inicial
 */
void randomize(colstatus gol, uint32_t on_time, uint32_t off_time) {
	uint32_t random_state = ENVIRONMENT->tick_count + ENVIRONMENT->task_id * 7;
	for (int j = 0; j < WIDTH; j++) {
		if (gol[j].duration == 0){
			gol[j].onoff = !gol[j].onoff;
			gol[j].duration = task_random(&random_state) % (gol[j].onoff ? on_time : off_time);
			gol[j].isfirstchar = 1;
		} else {
			gol[j].duration -= 1;
			gol[j].isfirstchar = 0;
		}
	}
}

uint8_t random_char(uint32_t* random_state){
	uint8_t res = task_random(random_state) % 256;
	while (res == 0x00 || res == 0x20 || res == 0x5F || res == 0xFF){
		res = task_random(random_state) % 256;
	}
	return res;
}

void task() {
	screen pantalla;
	colstatus gol = {0};

	int sleep_time = 15;
	int on_time = HEIGHT;
	int off_time = HEIGHT;
	randomize(gol, on_time, off_time);
	uint8_t mode = 0;
	


	while (!ENVIRONMENT->keyboard.escape) {

		if (mode){
			for (int i = HEIGHT-1; i > 0; i--) {
				for (int j = WIDTH-1; j >=0; j--) {
					if (pantalla[i-1][j].c == ' ') {
						pantalla[i][j] = pantalla[i-1][j];
					} else if (pantalla[i-1][j].a == (C_BG_BLACK | C_FG_LIGHT_GREEN)) {
						
					}
				}
			}
		} else {
			for (int i = HEIGHT-1; i > 0; i--) {
				for (int j = WIDTH-1; j >=0; j--) {
					pantalla[i][j] = pantalla[i-1][j];
				}
			}

			randomize(gol, on_time, off_time);
			uint32_t random_state = ENVIRONMENT->tick_count + ENVIRONMENT->task_id * 7;
			for (int j = 0; j < WIDTH; j++) {
				if (gol[j].onoff) {
					pantalla[0][j] =(ca){.c = 'a',
						.c = random_char(&random_state),  
						.a = C_BG_BLACK | (gol[j].isfirstchar ? C_FG_LIGHT_GREEN : C_FG_GREEN) };
				} else {
					pantalla[0][j] =(ca){.c = ' ', .a = C_BG_BLACK};
				}
			}

			/*if (ENVIRONMENT->task_id < 2){
				for (int j = 0; j < WIDTH; j++) {
					int entrySize = WIDTH*2+1;
					uint8_t* is_matrix_ptr = SHARED_CONTINUOUS_MATRIX + ENVIRONMENT->task_id*entrySize;
				}
			}*/
		}
		
		syscall_draw(pantalla);
		task_sleep(sleep_time);

		if (ENVIRONMENT->keyboard.up) {
			randomize(gol, on_time, off_time);
		} else if (ENVIRONMENT->keyboard.left && 50 <= sleep_time) {
			sleep_time++;
		} else if (ENVIRONMENT->keyboard.right && sleep_time > 50) {
			sleep_time--;
		}

		if (ENVIRONMENT->keyboard.spacebar) {
			mode = !mode;
		}
	}
}
