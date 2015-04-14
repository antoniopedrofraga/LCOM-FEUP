#include <minix/syslib.h>
#include <minix/drivers.h>
#include <machine/int86.h>
#include <sys/mman.h>
#include <sys/types.h>
#include "video_card.h"
#include "test5.h"
#include "timer.h"
#include "test3.h"
#include "pixmap.h"
#include "read_xpm.h"
#include "video_gr.h"
#include "i8042.h"
#include "lmlib.h"
#include "video_card.h"
#include "vbe.h"



void *test_init(unsigned short mode, unsigned short delay) {
	vg_init(mode);
	timer_test_int(delay);
	vg_exit();
	printf("\nVRAM ADRESS: 0x%02x\n",vg_adress());
	return vg_adress();
}


int test_square(unsigned short x, unsigned short y, unsigned short size, unsigned long color) {
	vg_init(SQUARE_MODE);
	int i;
	unsigned short x_max = x + size;
	unsigned short y_max = y + size;
	while(y <= y_max){
		for(i = x; i < x_max; i++){
			if(color_pixel(i, y, color)==1){
				vg_exit();
				printf("\n\nERROR\n\nYou are writing in non-existing pixels on mode 0x105\n\n");
				return 1;
			}
		}
		y++;
	}
	kbd_test_scan(0);
	vg_exit();
	return 0;
}

int test_line(unsigned short xi, unsigned short yi, 
		unsigned short xf, unsigned short yf, unsigned long color) {
	vg_init(LINE_MODE);
	unsigned short current_x = xi;
	int current_y;
	int last_y;
	float dx = xf - xi;
	float dy = yf - yi;
	if(dx != 0){ //when slope is not divisible by 0 (m), horizontal lines and others
		float m = dy / dx;
		float b = yf - m * xf;
		while(current_x < xf){   //positive (xf-xi)
			current_y = m * current_x + b;
			int i;
			if(current_x != xi){  //avoid non-initialized last_y
				for(i = last_y; i < current_y;i++){  //cases with high m values
					if(color_pixel(current_x ,i ,color)==1){
						vg_exit();
						printf("\n\nERROR\n\nYou are writing in non-existing pixels on mode 0x105\n\n");
						return 1;
					}
				}
			}
			if(color_pixel(current_x ,current_y ,color)==1){
				vg_exit();
				printf("\n\nERROR\n\nYou are writing in non-existing pixels on mode 0x105\n\n");
				return 1;
			}
			current_x++;
			last_y = current_y;
		}
		while(current_x > xf){	 //negative (xf-xi)
			current_y = m * current_x + b;
			int i;
			if(current_x!=xi) //avoid non-initialized last_y
				for(i = last_y; i > current_y;i--){  //cases with high m values
					if(color_pixel(current_x ,i ,color)==1){
						vg_exit();
						printf("\n\nERROR\n\nYou are writing in non-existing pixels on mode 0x105\n\n");
						return 1;
					}
				}
			if(color_pixel(current_x ,current_y ,color)==1){
				vg_exit();
				printf("\n\nERROR\n\nYou are writing in non-existing pixels on mode 0x105\n\n");
				return 1;
			}
			current_x--;
			last_y = current_y;
		}
	}else{	//when slope is divisible by 0 (m), dots or vertical lines
		current_y = yi;
		while(current_y <= yf){   //first case and positive (yf-yi)
			if(color_pixel(current_x,current_y,color)==1){
				vg_exit();
				printf("\n\nERROR\n\nYou are writing in non-existing pixels on mode 0x105\n\n");
				return 1;
			}
			current_y++;
		}
		while(current_y > yf){   //negative (yf-yi)
			if(color_pixel(current_x,current_y,color)==1){
				vg_exit();
				printf("\n\nERROR\n\nYou are writing in non-existing pixels on mode 0x105\n\n");
				return 1;
			}
			current_y--;
		}
	}
	kbd_test_scan(0);
	vg_exit();
	return 0;
}

int print_xpm(short x_min, short y_min, short x_max, short y_max, char*map){
	unsigned short current_y;
	unsigned short current_x;
	int i;
	int map_pos = 0;
	current_x = x_min;
	current_y = y_min;
	short h_res = get_h_res();
	short v_res = get_v_res();
	if(x_max < h_res && y_max < v_res && x_min >= 0 && y_min >= 0){
		vg_blank();
		while(current_y <= y_max){
			for(i = current_x; i < x_max; i++){
				if(color_pixel(i, current_y, map[map_pos])==1){
					vg_exit();
					printf("\n\nERROR\n\nYou are writing in non-existing pixels on mode 0x105\n\n");
					return 1;
				}
				map_pos++;
			}
			current_y++;
		}
	}
	return 0;
}


int test_xpm(unsigned short xi, unsigned short yi, char *xpm[]) {
	vg_init(XPM_MODE);
	int h;
	int w;
	int i;
	int map_pos = 0;
	char *map = read_xpm(xpm,&w,&h);
	unsigned short x_max = xi + w;
	unsigned short y_max = yi + h;
	if(print_xpm(xi,yi,x_max,y_max,map)==1){
		return 1;
	}
	kbd_test_scan(0);
	vg_exit();
	return 0;
}	


void exit_move(){
	unsigned long stat;
	timer_unsubscribe_int();
	kb_unsubscribe_int();
	sys_inb(IN_BUF,&stat);
	vg_exit();
}


int test_move(unsigned short xi, unsigned short yi, char *xpm[], 
		unsigned short hor, short delta, unsigned short time) {
	vg_init(XPM_MODE);

	//test stuff
	int x_test = 0;
	int y_test = 0;

	//kb stuff
	unsigned long l;
	unsigned long irq_setkb = kb_subscribe_int();
	//---------------------------------
	double velocity = delta / time; //pixel per second
	int counter = 0;
	int ipc_status;
	unsigned long irq_set = timer_subscribe_int();
	message msg;
	short x_min = xi;
	short y_min = yi;
	unsigned short limit = xi + delta;
	//---------------------------------
	int h;
	int w;
	char *map = read_xpm(xpm,&w,&h);
	//----------------------------------
	while(counter < (time*60) ) {
		int r =driver_receive(ANY, &msg, &ipc_status);
		if ( r != 0 ) {
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) { /* received notification */
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE: /* hardware interrupt notification */
				if (msg.NOTIFY_ARG & irq_set) { /* subscribed interrupt */
					counter++;
					if(counter%2==0){ //30 frames per second
						if(hor == 1) {
							x_min = x_min + velocity / 30;
						}else if (hor == 0){
							y_min = y_min + velocity / 30;
						}
						//-------------------------------------
						unsigned short x_max = x_min + w;
						unsigned short y_max = y_min + h;
						//--------------------------------------
						if(print_xpm(x_min,y_min,x_max,y_max,map)==1){
							return 1;
						}

					}
				}

				if (msg.NOTIFY_ARG & irq_setkb) {
					l = func();
					if(l==ESC){
						exit_move();
						return 0;
					}else{
						x_test++;
						y_test++;
						color_pixel(x_test, y_test, 3);
					}
				}
				break;
			default:
				break; /* no other notifications expected: do nothing */
			}
		} else { /* received a standard message, not a notification */
			//no standard messages expected to do nothing
		}
	}
	exit_move();
	return 0;
}



