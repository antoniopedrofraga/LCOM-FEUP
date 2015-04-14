#include <minix/drivers.h>
#include "timer.h"

static int proc_args(int argc, char *argv[]);

static unsigned long parse_ulong(char *str, int base);

static long parse_long(char *str, int base);

static void print_usage(char *argv[]);

int main(int argc, char **argv) {


   /* Initialize service */

  sef_startup();

  //timer_test_config(0);//Operating mode errado
  //timer_test_square(60);
  //timer_test_int(60);
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
	 "\t service run %s -args \"timer <timer from 0 to 2>\" \n"
	 "\t service run %s -args \"freq <freq>\" \n"
	 "\t service run %s -args \"chronometer <seconds>\" \n",
	 argv[0], argv[0], argv[0]);
}

static int proc_args(int argc, char *argv[]) {

  unsigned long seconds, timer, freq;
  char *str;
  long num;

  /*check the function to test: if the first characters match, accept it */
  if (strncmp(argv[1], "timer", strlen("timer")) == 0) {
	  if( argc != 3 ) {
		  printf("wrong no of arguments for test of timer_test_config() \n");
		  return 1;
	  }
	  if( (timer = parse_ulong(argv[2], 10)) == ULONG_MAX )
	  		  return 1;
	  if(timer!=0 && timer!=1 && timer!=2){
		  printf("argument timer ranges from 0 to 2\n");
			  return 1;
	  }
	  printf("timer_test_config(%lu)\n",timer); /* Actually, it was already invoked */
	  timer_test_config(timer);
	  return 0;
  } else if (strncmp(argv[1], "freq", strlen("freq")) == 0) {
	  if( argc != 3 ) {
		  printf("wrong no of arguments for test of timer_test_square() \n");
		  return 1;
	  }
	  if( (freq = parse_ulong(argv[2], 10)) == ULONG_MAX )
	  		  return 1;
	  printf("timer_test_square(%lu)\n",freq);
	  timer_test_square(freq);
	  return 0;
  } else if (strncmp(argv[1], "chronometer", strlen("chronometer")) == 0) {
	  if( argc != 3 ) {
		  printf("wrong no of arguments for test of timer_test_int() \n");
		  return 1;
	  }
	  if( (seconds = parse_ulong(argv[2], 10)) == ULONG_MAX )
	  		  return 1;
	  printf("timer_test_int(%lu)\n",seconds);
	  timer_test_int(seconds);
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
