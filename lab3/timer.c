#include <minix/syslib.h>
#include <minix/drivers.h>
#include "timer.h"
#include "i8254.h"


unsigned long counter = 0;
int hookid = 6;
int timer_set_square(unsigned long timer, unsigned long freq) {


    unsigned char mychar;
    unsigned char last4;
    unsigned long output;
    unsigned long outputshift;
    timer_get_conf(timer, &mychar);
    output = TIMER_FREQ/freq;
    outputshift = output >> 8;
    last4 = mychar << 4; //Provavelmente é aqui que dá o warning
    if(timer==0){
        last4=(TIMER_0|TIMER_LSB_MSB|last4);
    }else if(timer==1){
        last4=(TIMER_1|TIMER_LSB_MSB|last4);
    }else if(timer==2){
        last4=(TIMER_2|TIMER_LSB_MSB|last4);
    }else{
    	return 1;
    }

    if(sys_outb(TIMER_CTRL,last4)!=OK){//pede permissão ao controlo
        return 1;
    }
    if(timer==0){//lê dos timers
    	if(sys_outb(TIMER_0,output)!=OK ||
    	sys_outb(TIMER_0,outputshift)!=OK){
    		return 1;
    	}
    }else if(timer==1){
    	if(sys_outb(TIMER_1,output)!=OK ||
    	    	sys_outb(TIMER_1,outputshift)!=OK){
    	    		return 1;
    	    	}
    }else if(timer==2){
    	if(sys_outb(TIMER_1,output)!=OK ||
    	    	sys_outb(TIMER_1,outputshift)!=OK){
    	    		return 1;
    	    	}
    }else{
    	return 1;

    }
    return 0; //em caso de sucesso

}
int timer_subscribe_int(void ) {
	int hookidtemp = hookid;
	if(sys_irqsetpolicy(0,IRQ_REENABLE,&hookidtemp)!=OK){
        return -1;
	}
	if(sys_irqenable(&hookidtemp)!=OK){
		return -1;
	}
    return BIT(hookid); //return dos bits da interrupt mask
}

int timer_unsubscribe_int() {
	int hookidtemp = hookid;
	if(sys_irqrmpolicy(&hookidtemp)!=OK){
		return 1;
	}
	if(sys_irqenable(&hookidtemp)!=OK){
		return 1;
	}
	return 0;
}

void timer_int_handler() {
    counter++;
}

int timer_get_conf(unsigned long timer, unsigned char *st) {

	unsigned long command, save;
	if(timer == 0){
		command |= TIMER_0;
	}else if(timer == 1){
		command |= TIMER_1;
	}else if(timer == 2){
		command |= TIMER_2;
	}
	command |= TIMER_RB_STATUS_;
	command |= TIMER_RB_COUNT_;
	command |= TIMER_RB_CMD;
    sys_outb(TIMER_CTRL,command);
    if(timer==0)sys_inb(TIMER_0,&save);
    if(timer==1)sys_inb(TIMER_1,&save);
    if(timer==2)sys_inb(TIMER_2,&save);
    *st=(unsigned char)save;
	return 0;
}

int timer_display_conf(unsigned char conf){
	printf("Counting Mode: \n");
	if(!(conf & BIT(0))){
		printf("Binary\n");
	}else{
		printf("BCD\n");
	}
	printf("\nOperating mode: \n");

	if((conf & BIT(1))){
	//pode ser 1 ou 3 ou 5
		if(conf & BIT(2)){
			//é 3
		    printf("3\n");
		}else{
			//pode ser 1 ou 5
			if(conf & BIT(3)){
				//é 5
				printf("5\n");
			}else{
				//é 1
				printf("1\n");
			}
		}

	}else{
		//pode ser 0 2 ou 4
		if(conf & BIT(2)){
			//é 2
			printf("2\n");
		}else{
			//pode ser 0 ou 4
			if(conf & BIT(3)){
				//é 4
				printf("4\n");
			}else{
				//é 0
				printf("0\n");
			}
		}
	} // Basicamente analisamos o BIT(0) o BIT(1) e o BIT(2)

	printf("\nType of access:\n");
	if(conf & BIT(4)){
		//pode ser o LSB ou LSB followed by MSB
		if(conf & BIT(5)){
			//é o LSB
			printf("LSB\n");
		}else{
			//só resta 1
			printf("LSB followed by MSB\n");
		}

	}else{
		//é o MSB
		printf("MSB\n");
	}
	printf("\nSelect Counter: \n");
	if(conf & BIT(6)){
		//é o 1
		printf("1\n");
	}else{
		//pode ser 0 ou 2
		if(conf & BIT(7)){
			//é o 2
			printf("2\n");
		}else{
			printf("0\n");
		}
	}
	if((conf & BIT(6)) & (conf & BIT(7))){
		return 1;
	}
	if(!(conf & BIT(4)) & !(conf & BIT(5))){
		return 1;
	}
	return 0;
}

int timer_test_square(unsigned long freq) {
	if(timer_set_square(0,freq)==1){
		printf("\n\nWARNING, AN ERROR OCURRED AT SETTING FREQ,TRY AGAIN\n\n");
		return 1;
	}else{
		return 0;
	}
}

int timer_test_int(unsigned long time) {
	 counter = 0;
	 int ipc_status;
	 int a = 1;
	 unsigned long irq_set = timer_subscribe_int();
	 message msg;
	 while(counter < time*60 ) {
		 int r =driver_receive(ANY, &msg, &ipc_status);
		 if ( r != 0 ) {
		    printf("driver_receive failed with: %d", r);
		    continue;
		 }
		 if (is_ipc_notify(ipc_status)) { /* received notification */
			 switch (_ENDPOINT_P(msg.m_source)) {
			 case HARDWARE: /* hardware interrupt notification */
				 if (msg.NOTIFY_ARG & irq_set) { /* subscribed interrupt */
					 timer_int_handler();
					 if(counter%60==0){
						 printf("\n%d of %u seconds\n",a,time);

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
	 timer_unsubscribe_int();
	 return 0;
}

int timer_test_config(unsigned long timer) {
	unsigned char a;
	if(timer_get_conf(timer, &a)==0){
		if(timer_display_conf(a)==0){
			return 0;
		}else{
			printf("\n\nWARNING, AN ERROR OCURRED AT DISPLAYING CONFIG,TRY AGAIN\n\n");
		    return 1;
		}
	}else{
		printf("\n\nWARNING, AN ERROR OCURRED AT GETTING CONFIG,TRY AGAIN\n\n");
		return 1;
	}
}
