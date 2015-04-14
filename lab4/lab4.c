#include <minix/drivers.h>
#include "test4.h"

static int proc_args(int argc, char *argv[]);

static unsigned long parse_ulong(char *str, int base);

static long parse_long(char *str, int base);

static void print_usage(char *argv[]);

int main(int argc, char *argv[]) {
	/* Initialize service */
	  sef_startup();
	  //test_packet(60);
	  //test_async(10);
	  //test_config();

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
	 "\t service run %s -args \"packet <iterations>\" \n"
	 "\t service run %s -args \"async <seconds>\" \n"
	 "\t service run %s -args \"config\" \n"
	 "\t service run %s -args \"gesture <length tolerance>\" \n",
	 argv[0], argv[0], argv[0]);
}

static int proc_args(int argc, char *argv[]) {

  long  it, seconds, x, y;
  char *str;
  long num;

  /*check the function to test: if the first characters match, accept it */
  if (strncmp(argv[1], "packet", strlen("packet")) == 0) {
	  if( argc != 3 ) {
		  printf("wrong no of arguments for test of test_packet() \n");
		  return 1;
	  }
	  if( (it = parse_ulong(argv[2], 10)) == ULONG_MAX )
	  		  return 1;
	  printf("test_packet(%lu)\n",it); /* Actually, it was already invoked */
	  test_packet(it);
	  return 0;
  } else if (strncmp(argv[1], "async", strlen("async")) == 0) {
	  if( argc != 3 ) {
		  printf("wrong no of arguments for test of test_async() \n");
		  return 1;
	  }

	  if( (seconds = parse_ulong(argv[2], 10)) == ULONG_MAX )
	  		  return 1;
	  printf("test_async(%lu)\n",seconds);
	  test_async(seconds);
	  return 0;
  } else if (strncmp(argv[1], "config", strlen("config")) == 0) {
	  if( argc != 2 ) {
		  printf("wrong no of arguments for test of test_config() \n");
		  return 1;
	  }
	  printf("test_config()\n");
	  test_config();
	  return 0;
  } else if (strncmp(argv[1], "gesture", strlen("gesture")) == 0) {
	  if( argc != 4 ) {
		  printf("wrong no of arguments for test of test_gesture() \n");
		  return 1;
	  }
	  if( ( y = parse_ulong(argv[2], 10)) == ULONG_MAX )
	  	  		  return 1;
	  if( ( x = parse_ulong(argv[3], 10)) == ULONG_MAX )
	  	  	  		 return 1;
	  if(x <= 0 | y <= 0){
		  printf("\nCan't use negative numbers as argument\n");
		  return 0;
	  }
	  printf("test_gesture(%u,%u)\n",y,x);
	  test_gesture(y,x);
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
