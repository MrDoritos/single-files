#include <iostream>

using namespace std;

int main() {
	int x = 10;
	int *y = &x;

	cout << "Value of x: " << x << endl;
	cout << "Address of x: " << &x << endl;
	
	return 0;
}
