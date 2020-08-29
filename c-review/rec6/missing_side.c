#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
	
	extern char *optarg;
	extern int optind, opterr, optopt;

	int helpFlag = 0;
	int errorFlag = argc <= 4;
	
	char arg;
	float a = -1;
	float b = -1;
	float c = -1;

	while ((arg = getopt(argc, argv, "ha:b:c:")) != -1) {
		switch (arg) {
			case 'h':
				helpFlag = 1;
				break;
			case 'a':
				a = atof(optarg);
				if (a <= 0) {
					errorFlag = 1;
				}
				break;
			case 'b':
				b = atof(optarg);
				if (b <= 0) {
					errorFlag = 1;
				}
				break;
			case 'c':
				c = atof(optarg);
				if (c <= 0) {
					errorFlag = 1;
				}
				break;
			case '?':
				helpFlag = 1;
			default:
				errorFlag = 1;
				break;
		}
	}

	if (errorFlag || helpFlag) {
		fprintf(stderr, "usage: ...");
		exit(errorFlag);
	}

	if (a == -1 && b != -1 && c != -1) {
		printf("a = %f\n", sqrt(pow(c, 2) - pow(b, 2)));
	}
	else if (a != -1 && b == -1 && c != -1) {
		printf("b = %f\n", sqrt(pow(c, 2) - pow(a, 2)));
	}
	else if (a != -1 && b != -1 && c == -1) {
		printf("c = %f\n", sqrt(pow(a, 2) + pow(b, 2)));
	}
	else if (a != -1 && b != -1 && c != -1) {
		printf("Those values %s work\n", ((pow(a, 2) + pow(b, 2)) == pow(c, 2)) ? "do" : "don't");
	}

	return 0;
}

