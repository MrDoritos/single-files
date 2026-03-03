#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <tuple>

bool op_and(bool a, bool b) {
	return a && b;
}

bool op_or(bool a, bool b) {
	return a || b;
}

const char *bool_text(const bool &v) {
	return v ? "True" : "False";
}

std::string get_bar(const int &length) {
	return std::string(length, '-');
}

template<typename T>
int get_width(const T &pair) {
	return pair.first.size() + 6;
}

using func = bool(bool,bool);

int main() {
	std::vector<std::pair<std::string, func*>> ops = {
		{"and", op_and},
		{"or", op_or}
	};

	std::vector<std::string> props = {"p", "q"};

	int rows = 1 << props.size();

	std::cout << "   " << props[0] << "   |   " << props[1] << "   | " << props[0] << " " << ops[0].first << " "
		  << props[1] << " | " << props[0] << " " << ops[1].first << " " << props[1] << std::endl;

	std::cout << get_bar(7) << "|" << get_bar(7) << "|" << get_bar(get_width(ops[0])) << "|" << get_bar(get_width(ops[1])) << std::endl;

	bool vals[props.size()];

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < props.size(); j++) {
			const auto &prop = props[j];
			vals[j] = (rows-1-i)&(props.size()-j);
			printf("%7s|", bool_text(vals[j]));
		}
		for (int j = 0; j < ops.size(); j++) {
			const auto &op = ops[j].second;
			printf("%*s%s", get_width(ops[j]), bool_text(op(vals[0], vals[1])), j + 1 < ops.size() ? "|" : "");
		}
		
		printf("\n");
	}


	return 0;
}

