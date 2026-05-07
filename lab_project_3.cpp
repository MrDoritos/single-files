#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <iterator>

using dt = float;
using vec = std::vector<dt>;
using pair = std::pair<dt, dt>;

vec get_set(const char *name) {
	printf("Enter set %s as comma delimited real numbers\n", name);

	std::string input, token;
	vec v;

	getline(std::cin, input);
	input += '\n';

	for (auto &ch : input) {
		switch (ch) {
			case ',':
			case '\n':
				dt conv;
				if (sscanf(token.c_str(), "%f", &conv) != 1) {
					puts("User input error");
				} else {
					v.push_back(conv);
				}
				token.clear();
				break;
			default:
				token.push_back(ch);
				break;
		}
	}

	return v;
}

void print_set(const char* name, vec &v) {
	printf("Set %s: {", name);
	for (int i = 0; i < v.size(); i++) {
		printf("%f", v[i]);
		if (i + 1 < v.size())
			printf(", ");
	}
	printf("}\n");
}

void print_pairs(const std::vector<pair> &v) {
	printf("Cartesian product AxB: {");
	for (int i = 0; i < v.size(); i++) {
		printf("(%f, %f)", v[i].first, v[i].second);
		if (i + 1 < v.size())
			printf(", ");
	}
	printf("}\n");
}

void dedup_set(vec &v) {
	vec distinct;

	for (auto &e : v) {
		if (std::find(distinct.begin(), distinct.end(), e) == distinct.end())
			distinct.push_back(e);
	}	

	v = distinct;
}

std::vector<pair> cartesian_product(const vec &a, const vec &b) {
	std::vector<pair> ret;

	for (const auto &v1 : a) {
		for (const auto &v2 : b) {
			ret.push_back(pair(v1, v2));
		}
	}

	return ret;
}

int main() {
	vec A = get_set("A");
	vec B = get_set("B");

	printf("\n");

	dedup_set(A);
	dedup_set(B);
	
	print_set("A", A);
	print_set("B", B);

	printf("\n");

	auto cp = cartesian_product(A, B);
	print_pairs(cp);

	printf("\nCardinality of cartesian product |AxB|: %li\n\n", cp.size());

	printf("The cartesian product has many uses as it creates ordered tuples of all combinations of elements of sets. ");
	printf("For example, a cartesian product can be used to create all combinations of foreground, background, and characters in the terminal. ");
	printf("The elements of that cartesian product can be analyzed for the HSV or RGB mixture of each combination, which can then be used to create a color palette. Another application is found in math, where the cartesian product of two one-dimensional spaces is a two-dimensional space, RxR=R^2. ");
	printf("Combining one-dimensional spaces is how tables work, such as in excel.\n");

	return 0;

}
