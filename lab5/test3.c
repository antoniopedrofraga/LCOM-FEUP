#include <minix/sysutil.h>
#include <minix/syslib.h>
#include <minix/drivers.h>
#include "i8042.h"
#include "test3.h"
#include "i8254.h"
#include "timer.h"
int hookidkb = 8;
unsigned char CAPSON = 0;
unsigned char NUMLOCKON = 0;
unsigned char LOCKON =0;
unsigned long ass();

//KB FUNCTIONS
int kb_subscribe_int() {
	int temp = hookidkb; //hookid temporario para não alterar o valor original
	if(sys_irqsetpolicy(1,IRQ_REENABLE|IRQ_EXCLUSIVE,&temp)!=OK){
		return -1;
	}
	if(sys_irqenable(&temp)!=OK){
		return -1;
	}
	return BIT(hookidkb); //return dos bits da interrupt mask
}

int kb_unsubscribe_int() {
	int hookidkbtemp = hookidkb;
	if(sys_irqrmpolicy(&hookidkbtemp)!=OK){
		return 1;
	}
	if(sys_irqenable(&hookidkbtemp)!=OK){
		return 1;
	}
	return 0;
}
unsigned long func(){
	unsigned long data;
	unsigned long stat;
	int a =0;
	while( a<4 ) {//tenta no minimo 4 vezes
		sys_inb(STAT_REG, &stat);
		if( stat & OBF ) {
			if(sys_inb(OUT_BUF, &data)!=OK){
				return 1;
			}
			if ( (stat &(PAR_ERR | TO_ERR)) == 0 )
				return data;
			else
				return -1;
		}
		tickdelay(micros_to_ticks(DELAY_US));
		a++;
	}
}

int function(int a){

	unsigned long l;
	int ipc_status;
	message msg;
	unsigned long irq_set = kb_subscribe_int();
	while(l!=ESC) {//quando a tecla for ESC sai do ciclo

		int r =driver_receive(ANY, &msg, &ipc_status);
		if ( r != 0 ) {
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) { //recebeu uma notificação
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				if (msg.NOTIFY_ARG & irq_set) { //a notificação é do teclado
					if(a==0){
						l = func(); //recebe o byte da função em c
					}else if (a==1){
						l=ass();    //recebe o byte da função em assembly
					}
					if(l&BIT(7)){
						if(l==TWO_BYTES){
							printf("\nChar with 2 bytes\n");
						}else{
							printf("\nBreak Code: 0x%02x\n",l);
						}
					}else{
						printf("\nMake Code: 0x%02x\n",l);
					}
				}
				break;
			default:
				break; }
		} else { }
	}
	printf("\n\nExiting..\n\n");
	//está tudo bem
	if(timer_unsubscribe_int()==1){
		return 1; //em casp de erro
	}
	return 0;//em caso de não dar erro
}


int kbd_test_scan(unsigned short ass) {//fazer nesta aula

	function(ass);
	return 0;

}
void turnitright(int cmd){ //uma maneira de alterar os bits certos
	if(cmd==2 && !CAPSON){
		printf("\nTURNING CAPSLOCK ON\n");
		CAPSON = BIT(2);
	}else  if(cmd==2 && CAPSON){
		printf("\nTURNING CAPSLOCK OFF\n");
		CAPSON = 0;
	}
	if(cmd==0 && !LOCKON){
		printf("\nTURNING LOCK ON\n");
		LOCKON = BIT(0);
	}else  if(cmd==0 && LOCKON){
		printf("\nTURNING LOCK OFF\n");
		LOCKON = 0;
	}
	if(cmd==1 && !NUMLOCKON){
		printf("\nTURNING NUMLOCK ON\n");
		NUMLOCKON = BIT(1);
	}else  if(cmd==1 && NUMLOCKON){
		printf("\nTURNING NUMLOCK OFF\n");
		NUMLOCKON = 0;
	}
	if(cmd==-1){
		printf("\nturning all LEDS off\n");
		NUMLOCKON = 0;
		LOCKON = 0;
		CAPSON = 0;
	}
}
int escreve(int cmd){
	unsigned long stat;
	unsigned long info;
	unsigned char status;
	int a = 0; // para o caso de haver resend
	do{
		turnitright(cmd);
		status = CAPSON + NUMLOCKON + LOCKON; //retorna sempre estes bits, alterados no turnit right

		if(sys_outb(IN_BUF, status)!=OK){
			return 1;
		}
		if(sys_inb(OUT_BUF, &info)!=OK){
			return 1;
		}
		if(info==ERROR){
			printf("\nERROR sending, repeting the all process again\n");
			return 1; //1 significa que vamos ter que repetir o processo
		}else if(info==RESEND){
			printf("\nresending...\n");
			turnitright(cmd);
			a++;
		}else if(info == ACK){
			a = 0;
		}
		tickdelay(micros_to_ticks(DELAY_US));
	}while(a>0);
	return 0;
}
int process(int LEDnumber){
	unsigned long info;
	int thisERROR = 0; //Se o sys_inb retornar erro 0xFC então vai voltar a repetir o processo
	do{
		do{
			if(sys_outb(IN_BUF, 0xED)!=OK){
				return 1;
			}
			if(sys_inb(OUT_BUF, &info)!=OK){
				return 1;
			}
		}while(info!=ACK);
		if(escreve(LEDnumber)==1){//quer dizer que leu ERROR no sys_inb
			thisERROR = 1;
		}else{
			thisERROR = 0;
		}
	}while(thisERROR);
}
int kbd_test_leds(unsigned short n, unsigned short *leds) {
	process(-1); //apaga todos os LEDS
	unsigned long counterb = 0;
	int a = 0;
	int ipc_status;
	unsigned long irq_set = timer_subscribe_int();
	message msg;
	while(counterb < n*60 ) {
		int r =driver_receive(ANY, &msg, &ipc_status);
		if ( r != 0 ) {
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) {
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				if (msg.NOTIFY_ARG & irq_set) {
					counterb++;

					if(counterb%60==0){
						process(leds[a]);
						a++;
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
	process(-1); //apaga todos os LEDS
	if(timer_unsubscribe_int()==1){
		return 1;
	}
	return 0;
}

int kbd_test_timed_scan(unsigned short n) {
	//kb stuff
	unsigned long l;
	unsigned long irq_setkb = kb_subscribe_int();
	//timer stuff
	unsigned long counterb = 0;
	int a = 0;
	int ipc_status;
	unsigned long irq_set = timer_subscribe_int();
	message msg;
	while(counterb < n*60 ) {
		int r =driver_receive(ANY, &msg, &ipc_status);
		if ( r != 0 ) {
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) {
			if (msg.NOTIFY_ARG & irq_set)counterb++;

			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				if (msg.NOTIFY_ARG & irq_setkb) {
					l = func();
					if(l&BIT(7)){
						if(l==TWO_BYTES){
							printf("\nChar with 2 bytes\n");
						}else{
							printf("\nBreak Code: 0x%02x\n",l);
						}
					}else{
						printf("\nMake Code: 0x%02x\n",l);
						a = 0;
						counterb = 0;
					}
				}

				break;
			default:
				break;
			}
			if(l==ESC){
				printf("\n\nExiting..\n\n");
				if(kb_unsubscribe_int()==1){
					return 1;
				}
				if(timer_unsubscribe_int()==1){
					return 1;
				}
				return 0;
			}
			if(counterb%60==0){
				if(counterb!=0){
					printf("\n%d de %u segundos\n",a,n);
				}
				a++;
			}
		}
	}
	printf("\n\nExiting..\n\n");
	timer_unsubscribe_int();
	kb_unsubscribe_int();
	return 0;
}
