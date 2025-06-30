#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print(int i, int n) {
	char buf[n+1];

	//memset(buf, '-', n);
	for (int j = 0; j < n; j++)
		buf[j] = '-';

	buf[n]=0;

	int s = (n / 2) - i + 1;

	//memset(buf + s, '*', n-s-s);
	for (int j = 0; j < n-s-s; j++)
		buf[j+s] = '*';

	puts(buf);
}

void diamond(int n) {	
	for (int j = 0; j < n; j++) {
		int i = (j > n / 2) ? n-j : j+1;
		print(i, n);
	}
}

int main() {
	int n;
	scanf("%i", &n);
	diamond(n*2-1);

	return 0;
}
