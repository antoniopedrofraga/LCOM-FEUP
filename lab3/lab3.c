#include <minix/drivers.h>
#include "test3.h"
static int proc_args(int argc, char *argv[]);

static unsigned long parse_ulong(char *str, int base);

static long parse_long(char *str, int base);

static void print_usage(char *argv[]);

int main(int argc, char **argv) {
  /* Initialize service */
  sef_startup();
  sys_enable_iop(SELF);
  //kbd_test_scan(0);
  //kbd_test_leds(6,array);
  //kbd_test_timed_scan(10);
   if ( argc == 1 ) {
       print_usage(argv);
       return 0;
   } else {
       proc_args(argc, argv);
   }
   return 0;

 }
static void print_usage(char *argv[]) {
  printf("Usage: one of the following:\n"
	 "\t service run %s -args \"kbscan <0 to assembly and 1 to c code>\" \n"
	 "\t service run %s -args \"leds <array with elements from 0 to 2>\" \n"
	 "\t service run %s -args \"kbtimer  <seconds>\" \n",
	 argv[0], argv[0], argv[0]);
}

static int proc_args(int argc, char *argv[]) {

  unsigned long ass, seconds, size;
  char *str;
  long num;

  /*check the function to test: if the first characters match, accept it */
  if (strncmp(argv[1], "kbscan", strlen("kbscan")) == 0) {
	  if( argc != 3 ) {
		  printf("wrong no of arguments for test of kbd_test_scan() \n");
		  return 1;
	  }
	  if( (ass = parse_ulong(argv[2], 10)) == ULONG_MAX )
	  		  return 1;
	  if(ass!=0 && ass!=1){
		  printf("you can only choose 1 or 0\n");
			  return 1;
	  }
	  printf("kbd_test_scan(%lu)\n",ass); /* Actually, it was already invoked */
	  kbd_test_scan(ass);
	  return 0;
  } else if (strncmp(argv[1], "kbtimer", strlen("kbtimer")) == 0) {
	  if( argc != 3) {
		  printf("wrong no of arguments for test of kbd_test_timed_scan() \n");
		  return 1;
	  }
	  if( (seconds = parse_ulong(argv[2], 10)) == ULONG_MAX )
	  		  return 1;
	  printf("kbd_test_timed_scan(%lu)\n",seconds);
	  kbd_test_timed_scan(seconds);
	  return 0;
  }else if (strncmp(argv[1], "leds", strlen("leds")) == 0) {
	  int a=2;
	  size=0;
	  while(argv[a]){
		  size++;
		  a++;
	  }
	  unsigned short array[size];
	  a=2;
	  int i=0;
	  while(i<size){
		  if(!argv[a]){
			  printf("Wrong size of array,there is at least one less array position\n"); //nao há argumento na posicao a;
			  return 1;
		  }
		  if( (array[i] = parse_ulong(argv[a], 10)) == ULONG_MAX )
		  	  		  return 1;

		  if(array[i]!=0 && array[i]!=1 && array[i]!=2){
			  printf("Array elements can only range from 0 to 2\n");
			  return 1;
		  }
		  a++;
		  i++;
	  }
	  if(argv[a]){
		  printf("Wrong size of array, there is at least one more array position\n"); //há um argumento a mais
		  return 1;
	  }
	  i=0;
	  printf("kbd_test_leds( %lu ,{ ",size);
	  while(i<size){
		  printf("%u",array[i]);
		  i++;
		  if(size>i){
			  printf(" , ");
		  }
	  }
	  			  printf(" }");
	  printf(" )\n");
	  kbd_test_leds(size,array);
	  return 0;
  }  else {
	  printf(" non valid function \"%s\" to test\n", argv[1]);
	  return 1;
  }
}

static unsigned long parse_ulong(char *str, int base) {
  char *endptr;
  unsigned long val;

  val = strtoul(str, &endptr, base);

  if ((errno == ERANGE && val == ULONG_MAX )
	  || (errno != 0 && val == 0)) {
	  perror("strtol");
	  return ULONG_MAX;
  }

  if (endptr == str) {
	  printf("parse_ulong: no digits were found in %s \n", str);
	  return ULONG_MAX;
  }

  /* Successful conversion */
  return val;
}
