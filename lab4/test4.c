#include <minix/sysutil.h>
#include <minix/syslib.h>
#include <minix/drivers.h>
#include "i8042.h"
#include "i8254.h"
#include "test4.h"
#include "timer.h"

unsigned int hookidmouse = 2;

int mouse_subscribe_int() {
	int temp = hookidmouse; //hookid temporario para não alterar o valor original
	if(sys_irqsetpolicy(12 ,IRQ_REENABLE | IRQ_EXCLUSIVE, &hookidmouse)!=OK){
		return 1;
	}
	if(sys_irqenable(&hookidmouse)!=OK){
		return 1;
	}
	return BIT(temp); //return dos bits da interrupt mask
}

int mouse_unsubscribe_int() {
	int temp = hookidmouse;
	if(sys_irqdisable(&temp)!=OK){
				return 2;
		}
	if(sys_irqrmpolicy(&temp)!=OK){
			return 1;
		}
	return 0;
}

unsigned short func(){
	unsigned long data;
	unsigned long stat;
	int a =0;
	while( a<4 ) {//tenta no minimo 4 vezes
		sys_inb(STAT_REG, &stat);
		if( stat & OBF ) {
			if(sys_inb(OUT_BUF, &data)!=OK){
				return 1;
			}
			return data;
		}
		tickdelay(micros_to_ticks(DELAY_US));
		a++;
	}
	return -1;
}

int test_packet(unsigned short cnt){
	printf("\n\nentraste no test_packet\n\n");
	int l = 0;
	long array[3];
	int counter = 0;
	int ipc_status;
	message msg;
	int a = mouse_subscribe_int();
	if(a == 1){
		printf("\n\nERRO NO SUBSCRIBE INT\n\n");
		return 0;
	}
	unsigned long irq_set = a;
	printf("\n\nIRQ_SET: %lu\n\n",irq_set);
	//ATIVAR O RATO (FALTAM OS ERROS)
	if(sys_outb(STAT_REG, 0xA8)!=OK) printf("\nERRO na primeira escrita\n");
	if(sys_outb(STAT_REG, 0xD4)!=OK) printf("\nERRO na segunda escrita\n");
	if(sys_outb(IN_BUF, 0xF4)!=OK) printf("\nERRO na terceira escrita\n");

	while(l <= cnt) {
		int r = driver_receive(ANY, &msg, &ipc_status);
		if ( r != 0 ) {
			printf("driver_receive failed with: %d", r);
			continue;
		}

		if (is_ipc_notify(ipc_status)) { //recebeu uma notificação
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				if (msg.NOTIFY_ARG & irq_set) { //a notificação é do rato;
					array[counter] = func();
					if((array[0] & BIT(3)) != 0){
						if(counter !=2){
							counter++;
						}else{
							printf("%d iteration\t",l);
							printf("B1=%02x ",array[0]);
							printf("B2=%02x ",array[1]);
							printf("B3=%02x ",array[2]);
							printf("LB=%u ",array[0] & BIT(0));
							printf("MB=%u ",array[0] & BIT(2));
							printf("RB=%u ",array[0] & BIT(1));
							printf("XOVF=%u ",array[0] & BIT(6));
							printf("YOVF=%u ",array[0] & BIT(7));
							if(BIT(7) & array[1]){ array[1] = array[1]^255; array[1] = -array[1];}
							if(BIT(7) & array[2]){ array[2] = array[2]^255; array[2] = -array[2];}
							printf("X=%ld ",array[1]);
							printf("Y=%ld\n",array[2]);
							l++;
							counter = 0;
						}
					}else{
						printf("\nnot a valid first byte\n");
					}
				}
				break;
			default:
				break;
			}
		}
	}
	printf("\n\nExiting..\n\n");
	//está tudo bem
	if(sys_outb(STAT_REG, 0xD4)!=OK) printf("ERROR on writing to 0x64");
	if(sys_outb(IN_BUF,0xF5)!=OK) printf("ERROR on writing to 0x60");
	int i = mouse_unsubscribe_int();
	unsigned long stat;
	sys_inb(OUT_BUF,&stat);
	if(i != 0){
	printf("\n\nEsta a dar erro no unsubscribe %d\n\n",i);
	return 1; //em caso de erro
	}
	return 0;//em caso de não dar erro
}

int test_async(unsigned short idle_time) {
		int b = 0;
		int l = 0;
	//mouse stuff
		int counter = 0;
		unsigned long array[3];
		int a = mouse_subscribe_int();
			if(a == 1){
				printf("\n\nERRO NO SUBSCRIBE INT\n\n");
				return 0;
			}
			unsigned long irq_setmouse = a;
			printf("\n\nIRQ_SET: %lu\n\n",irq_setmouse);
			//ATIVAR O RATO (FALTAM OS ERROS)
			if(sys_outb(STAT_REG, 0xA8)!=OK) printf("\nERRO na primeira escrita\n");
			if(sys_outb(STAT_REG, 0xD4)!=OK) printf("\nERRO na segunda escrita\n");
			if(sys_outb(IN_BUF, 0xF4)!=OK) printf("\nERRO na terceira escrita\n");
		//timer stuff
		unsigned long counterb = 0;
		int ipc_status;
		unsigned long irq_set = timer_subscribe_int();
		message msg;
		while(counterb < idle_time*60 ) {
			int r =driver_receive(ANY, &msg, &ipc_status);
			if ( r != 0 ) {
				printf("driver_receive failed with: %d", r);
				continue;
			}
			if (is_ipc_notify(ipc_status)) {
				if (msg.NOTIFY_ARG & irq_set)counterb++;

				switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE:
					if (msg.NOTIFY_ARG & irq_setmouse) {
						b = 0;
						counterb = 0;
						array[counter] = func();
						if((array[0] & BIT(3)) != 0){
							if(counter !=2){
								counter++;
							}else{
								printf("B1=%02x ",array[0]);
								printf("B2=%02x ",array[1]);
								printf("B3=%02x ",array[2]);
								printf("LB=%u ",array[0] & BIT(0));
								printf("MB=%u ",array[0] & BIT(2));
								printf("RB=%u ",array[0] & BIT(1));
								printf("XOVF=%u ",array[0] & BIT(6));
								printf("YOVF=%u ",array[0] & BIT(7));
								printf("X=%u ",array[1]);
								printf("Y=%u\n",array[2]);
								counter = 0;
							}
						}else{
							printf("\nnot a valid first byte\n");
						}

					}

					break;
				default:
					break;
				}

				if(counterb%60==0){
					if(counterb!=0){
						printf("\n%d de %u segundos\n",b,idle_time);
					}
					b++;
				}
			}
		}
		printf("\n\nExiting..\n\n");
		if(sys_outb(STAT_REG, 0xD4)!=OK) printf("ERROR on writing to 0x64");
		if(sys_outb(IN_BUF,0xF5)!=OK) printf("ERROR on writing to 0x60");
		unsigned long stat;
		sys_inb(OUT_BUF,&stat);
		if(timer_unsubscribe_int()!=0) printf("\nERROR ON TIMER_UNSUBSCRIBE\n");
		if(mouse_unsubscribe_int()!=0) printf("\nERROR ON MOUSE_UNSUBSCRIBE\n");
		return 0;
}

int test_config(void) {
		int l = 0;
		unsigned long array[3];
		int counter = 0;
		int ipc_status;
		message msg;
		unsigned long irq_set = mouse_subscribe_int();
		if(irq_set == 1){
			printf("\n\nERROR ON SUBSCRIBE INT\n\n");
			return 0;
		}
		unsigned long stat;
		//ATIVAR O RATO (FALTAM OS ERROS)
		if(sys_outb(STAT_REG, 0xA8)!=OK) printf("\nERRO na primeira escrita\n");
		if(sys_outb(STAT_REG, 0xD4)!=OK) printf("\nERRO na segunda escrita\n");
		if(sys_outb(IN_BUF, 0xF4)!=OK) printf("\nERRO na terceira escrita\n");

		do{
			if(sys_outb(IN_BUF, 0xE9)!=OK)  return 0;
			if(sys_inb(OUT_BUF,&stat)!=OK)	return 0;
		}while(stat!=ACK);
		int ib = -1; //entra no primeiro ciclo, ib precsisa de ser inicializado a -1;
		while(ib < 3){
			//DESATIVAR

			int r = driver_receive(ANY, &msg, &ipc_status);
			if ( r != 0 ) {
				printf("driver_receive failed with: %d", r);
				continue;
			}
			if (is_ipc_notify(ipc_status)) { //recebeu uma notificação
				switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE:
					if (msg.NOTIFY_ARG & irq_set) { //a notificação é do rato;
						//array[ib] = func();
						sys_inb(OUT_BUF, &stat);
						if(ib!=-1)
						array[ib] = stat;
						ib++;
					}
					break;
				default:
					break;
				}
			}
		}
		//User friendly display
		if(array[0] & BIT(6)){
			printf("\nRemote (poled) mode\n");
		}else{
			printf("\nStream mode\n");
		}
		if(array[0] & BIT(5)){
			printf("\nData reporting enabled\n");
		}else{
			printf("\nData reporting disabled\n");
		}
		if(array[0] & BIT(4)){
			printf("\nScaling is 2:1\n");
		}else{
			printf("\nScaling is 1:1\n");
		}
		if(array[0] & BIT(2)){
			printf("\nMiddle button pressed\n");
		}else{
			printf("\nMiddle button released\n");
		}
		if(array[0] & BIT(1)){
			printf("\nRight button pressed\n");
		}else{
			printf("\nRight button released\n");
		}
		if(array[0] & BIT(0)){
			printf("\nLeft button pressed\n");
		}else{
			printf("\nLeft button released\n");
		}
		int d1 = array[0];
		int d3 = array[2];
		printf("\nResolution set to: %d\n", d1 & BIT(0) & BIT(1) & BIT(2));

		printf("\nSample rate: %d\n", d3);

		printf("\n\nExiting..\n\n");
		//DESATIVAR
		if(sys_outb(STAT_REG, 0xD4)!=OK) printf("ERROR on writing to 0x64");
		if(sys_outb(IN_BUF,0xF5)!=OK) printf("ERROR on writing to 0x60");
		sys_inb(IN_BUF,&stat);
		int i = mouse_unsubscribe_int();
		if(i != 0){
			printf("\n\nEsta a dar erro no unsubscribe %d\n\n",i);
			return 1; //em caso de erro
		}
		return 0;//em caso de não dar erro
}




int test_gesture(short length, unsigned short tolerance) {
		long horizontal = 0;
		long vertical = 0;
		int l = 0;
		long array[3];
		int counter = 0;
		int ipc_status;
		message msg;
		int a = mouse_subscribe_int();
		if(a == 1){
			printf("\n\nERRO NO SUBSCRIBE INT\n\n");
			return 0;
		}
		unsigned long irq_set = a;
		printf("\n\nIRQ_SET: %lu\n\n",irq_set);
		//ATIVAR O RATO (FALTAM OS ERROS)
		if(sys_outb(STAT_REG, 0xA8)!=OK) printf("\nERRO na primeira escrita\n");
		if(sys_outb(STAT_REG, 0xD4)!=OK) printf("\nERRO na segunda escrita\n");
		if(sys_outb(IN_BUF, 0xF4)!=OK) printf("\nERRO na terceira escrita\n");
		int gesture = 0;
		while(!gesture) {
			int r = driver_receive(ANY, &msg, &ipc_status);
			if ( r != 0 ) {
				printf("driver_receive failed with: %d", r);
				continue;
			}

			if (is_ipc_notify(ipc_status)) { //recebeu uma notificação
				switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE:
					if (msg.NOTIFY_ARG & irq_set) { //a notificação é do rato;
						array[counter] = func();
						if((array[0] & BIT(3)) != 0){
							if(counter !=2){
								counter++;
							}else{

								printf("%d iteration\t",l);
								printf("B1=%02x ",array[0]);
								printf("B2=%02x ",array[1]);
								printf("B3=%02x ",array[2]);
								printf("LB=%u ",array[0] & BIT(0));
								printf("MB=%u ",array[0] & BIT(2));
								printf("RB=%u ",array[0] & BIT(1));
								printf("XOVF=%u ",array[0] & BIT(6));
								printf("YOVF=%u ",array[0] & BIT(7));
								if(BIT(7) & array[1]){ array[1] = array[1]^255; array[1] = -array[1];}
								if(BIT(7) & array[2]){ array[2] = array[2]^255; array[2] = -array[2];}
								printf("X=%ld ",array[1]);
								printf("Y=%ld\n",array[2]);
								if(array[0] & BIT(0)){
									horizontal = horizontal + array[1];
									vertical = vertical + array[2];
								}else{
								vertical = 0;
								horizontal = 0;
								}
								if(vertical >= tolerance || vertical <= -tolerance){
								vertical = 0;
								horizontal = 0;
								}
								if(horizontal >= length || horizontal <= -length){
									gesture = 1;
								}
								printf("\n Vertical = %d  ",vertical);
								printf("  Horizontal = %d\n",horizontal);
								l++;
								counter = 0;
							}
						}else{
							printf("\nnot a valid first byte\n");
						}
					}
					break;
				default:
					break;
				}
			}
		}
		printf("\n\nExiting..\n\n");
		//está tudo bem
		if(sys_outb(STAT_REG, 0xD4)!=OK) printf("ERROR on writing to 0x64");
		if(sys_outb(IN_BUF,0xF5)!=OK) printf("ERROR on writing to 0x60");
		int i = mouse_unsubscribe_int();
		unsigned long stat;
		sys_inb(OUT_BUF,&stat);
		if(i != 0){
		printf("\n\nEsta a dar erro no unsubscribe %d\n\n",i);
		return 1; //em caso de erro
		}
		return 0;//em caso de não dar erro
	}


