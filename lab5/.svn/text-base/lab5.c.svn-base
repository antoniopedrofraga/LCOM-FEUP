#include <minix/drivers.h>
#include "test5.h"
#include "pixmap.h"
#include "lmlib.h"
static int proc_args(int argc, char *argv[]);

static unsigned long parse_ulong(char *str, int base);

static long parse_long(char *str, int base);

static void print_usage(char *argv[]);

int main(int argc, char *argv[]) {
	/* Initialize service */
	sef_startup();

	if ( argc == 1 ) {
		print_usage(argv);
		return 0;
	} else {
		sys_enable_iop(SELF);
		lm_init();
		proc_args(argc, argv);
	}


	return 0;
}


static void print_usage(char *argv[]) {
  printf("Usage: one of the following:\n"
	 " service run %s -args \"init <mode>\" \n"
	 " service run %s -args \"square <xi> <yi> <size> <color>\" \n"
	 " service run %s -args \"line <xi> <yi> <xf> <yf> <color>\" \n"
	 " service run %s -args \"xpm <xi> <yi> <xpm>\" \n"
	 " service run %s -args \"move <xi> <yi> <xpm> <hor> <delta> <time>\" \n",
	 argv[0], argv[0], argv[0], argv[0], argv[0]);
}

static int proc_args(int argc, char *argv[]) {

	unsigned short  xi , yi , size , color , xf , yf , delay, mode, hor, time;
	short delta;
	char *str;
	long num;
	/*check the function to test: if the first characters match, accept it */
	if (strncmp(argv[1], "init", strlen("init")) == 0) {
		if( argc != 4 ) {
			printf("wrong no of arguments for test of test_init() \n");
			return 1;
		}
		mode = strtol(argv[2], NULL, 16);
		if( (delay = parse_ulong(argv[3], 10)) == ULONG_MAX )
			return 1;

		printf("test_init(0x%lu , %lu)\n", mode , delay);
		test_init(mode , delay);
		return 0;
	} else if (strncmp(argv[1], "square", strlen("square")) == 0) {
		if( argc != 6 ) {
			printf("wrong no of arguments for test of test_square() \n");
			return 1;
		}
		if( (xi = parse_ulong(argv[2], 10)) == ULONG_MAX )
			return 1;
		if( (yi = parse_ulong(argv[3], 10)) == ULONG_MAX )
			return 1;
		if( (size = parse_ulong(argv[4], 10)) == ULONG_MAX )
			return 1;
		if( (color = parse_ulong(argv[5], 10)) == ULONG_MAX )
			return 1;

		printf("test_square(%lu , %lu , %lu , %lu)\n", xi , yi , size , color);
		test_square(xi,yi,size,color);
		return 0;
	} else if (strncmp(argv[1], "move", strlen("move")) == 0) {
		if( argc != 8 ) {
			printf("wrong no of arguments for test of test_move() \n");
			return 1;
		}
		if( (xi = parse_ulong(argv[2], 10)) == ULONG_MAX )
			return 1;
		if( (yi = parse_ulong(argv[3], 10)) == ULONG_MAX )
			return 1;
		if( (hor = parse_ulong(argv[5], 10)) == ULONG_MAX )
			return 1;
		if( (delta = parse_ulong(argv[6], 10)) == ULONG_MAX )
			return 1;
		if( (time = parse_ulong(argv[7], 10)) == ULONG_MAX )
			return 1;
		if((strncmp(argv[4], "pic1", strlen("pic1")) == 0)) test_move(xi,yi,pic1,hor,delta,time);
		else if((strncmp(argv[4], "pic2", strlen("pic2")) == 0)) test_move(xi,yi,pic2,hor,delta,time);
		else if((strncmp(argv[4], "pic3", strlen("pic3")) == 0)) test_move(xi,yi,pic3,hor,delta,time);
		else if((strncmp(argv[4], "cross", strlen("cross")) == 0)) test_move(xi,yi,cross,hor,delta,time);
		else if((strncmp(argv[4], "penguin", strlen("penguin")) == 0)) test_move(xi,yi,penguin,hor,delta,time);
		else{
			printf("\n\nThere is not such xpm, %s, unable to call test_move()\n\n",argv[4]);
			return 1;
		}
		printf("test_move(%lu , %lu , %s , %lu , %ld , %lu)\n", xi , yi, argv[4], hor, delta, time);
		return 0;
	} else if (strncmp(argv[1], "line", strlen("line")) == 0) {
		if( argc != 7 ) {
			printf("wrong no of arguments for test of test_line() \n");
			return 1;
		}
		if( (xi = parse_ulong(argv[2], 10)) == ULONG_MAX )
			return 1;
		if( (yi = parse_ulong(argv[3], 10)) == ULONG_MAX )
			return 1;
		if( (xf = parse_ulong(argv[4], 10)) == ULONG_MAX )
			return 1;
		if( (yf = parse_ulong(argv[5], 10)) == ULONG_MAX )
			return 1;
		if( (color = parse_ulong(argv[6], 10)) == ULONG_MAX )
			return 1;
		printf("test_line(%lu , %lu , %lu , %lu , %lu)\n", xi , yi , xf , yf , color);
		test_line(xi,yi,xf,yf,color);
		return 0;
	} else if (strncmp(argv[1], "xpm", strlen("xpm")) == 0) {
		if( argc != 5 ) {
			printf("wrong no of arguments for test of test_xpm() \n");
			return 1;
		}
		if( (xi = parse_ulong(argv[2], 10)) == ULONG_MAX )
			return 1;
		if( (yi = parse_ulong(argv[3], 10)) == ULONG_MAX )
			return 1;
		if((strncmp(argv[4], "pic1", strlen("pic1")) == 0)) test_xpm(xi,yi,pic1);
		else if((strncmp(argv[4], "pic2", strlen("pic2")) == 0)) test_xpm(xi,yi,pic2);
		else if((strncmp(argv[4], "pic3", strlen("pic3")) == 0)) test_xpm(xi,yi,pic3);
		else if((strncmp(argv[4], "cross", strlen("cross")) == 0)) test_xpm(xi,yi,cross);
		else if((strncmp(argv[4], "penguin", strlen("penguin")) == 0)) test_xpm(xi,yi,penguin);
		else{
			printf("\n\nThere is not such xpm, %s\n\n",argv[4]);
			return 1;
		}
		printf("test_xpm(%lu , %lu , %s)\n", xi , yi, argv[4]);
		return 0;
	} else {
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

