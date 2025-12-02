#include <iostream>

using namespace std;

int main() {
	int arraySize;
	double *dArray;
	cin >> arraySize;
	dArray = new double[arraySize];

	for (int i = 0; i < arraySize*2000; i++) {
		dArray[i] = i + 1;
	}

	return 0;
}
