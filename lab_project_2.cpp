#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <functional>
#include <vector>
#include <string>
#include <map>
#include <tuple>

using dt = float;
using vec = std::vector<dt>;
using predicate_t = bool(*)(dt&,dt&);

void get_domain(std::istream &s, vec &v) {
	puts("Enter domain set in the form {0,1,...,n}");

	std::string in, token;

	getline(s, in);

	v.clear();

	for (auto &ch:in) {
		switch (ch) {
			case ',':
			case '}':
				dt conv;
				if (sscanf(token.c_str(), "%f", &conv) != 1) {
					puts("User input error");
				} else {
					v.push_back(conv);
				}
				token.clear();
				break;
			case '{': 
				break;
			default:
				token.push_back(ch);
				break;
		}
	}
}

void get_test_element(std::istream &s, dt &v) {
	puts("Enter test element");

	s >> v;
}

bool less_than(dt &a, dt &b) {
	return a < b;
}

bool greater_than(dt &a, dt &b) {
	return a > b;
}

bool equal_to(dt &a, dt &b) {
	return a == b;
}

std::ostream &operator<<(std::ostream &s, const vec &v) {
	s << "{";

	for (int i = 0; i < v.size(); i++) {
		s << v[i];
		if (i + 1 < v.size()) 
			s << ",";
	}

	s << "}";

	return s;
}

std::string get_quantifier(predicate_t pred, vec &domain, dt &v) {
	size_t bool_sum = 0;

	for (auto &d : domain) {
		bool_sum += pred(v, d) ? 1 : 0;
	}

	size_t domain_size = domain.size();

	if (bool_sum == domain_size) {
		return "Quantifier: Universal, for all, ∀xP(x)";
	} else
        if (bool_sum > 0) {
		return "Quantifier: Existential, there exists, ∃xP(x)";
	} else {
		return "Quantifier: Inverse Universal, there exists none, ¬∀xP(x)";
	}
}

int main() {
	vec domain;
	get_domain(std::cin, domain);

	std::cout << "Domain: " << domain << std::endl << std::endl;

	dt test_element;
	get_test_element(std::cin, test_element);

	std::cout << "Test element: " << test_element << std::endl << std::endl;

	std::vector<std::tuple<predicate_t, std::string, std::string>> predicates = {
		{ less_than, "Less Than, P(x) = (x < {...})", "<" },
		{ greater_than, "Greater Than, P(x) = (x > {...})", ">" },
		{ equal_to, "Equal To, P(x) = (x == {...})", "==" }

	};

	auto &pred = predicates[2];

	std::cout << "Predicate: " << std::get<1>(pred) << std::endl;

	for (auto &d : domain) {
		printf("%.2f %s %.2f = %s\n", test_element, std::get<2>(pred).c_str(), d, std::get<0>(pred)(test_element, d) ? "True" : "False");
	}

	std::cout << std::endl << get_quantifier(std::get<0>(pred), domain, test_element) << std::endl;

	return 0;

}
