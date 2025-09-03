#include <stdlib.h>
#include <stdio.h>

float Q_rsqrt(float number) {
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y = number;
	i = *(long*)&y;
	i = 0x5F3759DF - (i >> 1);
	y = *(float*)&i;
	y = y * (threehalfs - (x2 * y * y));

	return y;
}

int main(int argc, char** argv) {
	if (argc < 2) {
		puts("Invalid input");
		return 1;
	}

	float num;
	if (sscanf(argv[1], "%f", &num) != 1) {
		puts("Invalid input");
		return 2;
	}

	printf("1/sqrt(%f)=%f\n", num, Q_rsqrt(num));

	return 0;
}
