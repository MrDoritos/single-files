#include <iostream>
#include <stdio.h>
#include <string>

using namespace std;

int main() {
	int interstate_num;
	cin >> interstate_num;


	if (interstate_num % 5 != 0) {
		std::cout << interstate_num << " is not valid" << std::endl;
		return 0;
	}

	int serving = interstate_num % 100;

	if (interstate_num < 100) {
		std::cout << "I-" << interstate_num << " is primary. ";
		if (interstate_num % 10 == 5) {
			std::cout << "going east/west." << std::endl;
		} else {
			std::cout << "going north/south." << std::endl;
		}
	} else {
		std::cout << "I-" << interstate_num << " is auxiliary. ";
		if (interstate_num % 10 == 5) {
			std::cout << "going east/west." << std::endl;
		} else {
			std::cout << "going north/south." << std::endl;
		}
	}

	return 0;
}
