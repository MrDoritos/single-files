#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <math.h>
#include <algorithm>

template<typename T>
struct SetBase {
	using value_type = T;
	virtual void print()=0;
	virtual bool exists(const T &v)=0;
};

struct IntegerSet : public SetBase<float> {
	bool exists(const float &v) override {
		return int(v)==v;
	}

	void print() override {
		std::cout<<"\033[1mZ\033[0m";
	}
};

template<typename T>
struct FiniteSet : public SetBase<T> {
	std::vector<T> set;

	FiniteSet(const std::vector<T> &set = std::vector<T>()):set(set){}

	bool exists(const T &v) override {
		//whoops prepackaged function
		//return std::find(set.begin(), set.end(), v) != set.end();
		for(const auto&w:set)if(w==v)return true;
		return false;
	}

	void print() override {
		std::cout<<"{";
		for (std::size_t i = 0; i < set.size(); i++) {
			std::cout<<set[i];
			if (i + 1 < set.size())
				std::cout<<", ";
		}
		std::cout<<"}";
	}

	template<typename U>
	FiniteSet<T> interset(SetBase<U> &other) {
		FiniteSet<T> ret;
		for (const auto &v : set)
			if (other.exists(v))
				ret.set.push_back(v);
		return ret;
	}
};

int main() {
	std::cout << "My program also supports intersection of two finite sets as well, but I wanted to implement an infinite set." << std::endl << std::endl;

	IntegerSet Z;
	FiniteSet<float> set({1,1.2,6,M_PI,std::sqrt(2.0),9,900});

	std::cout << "Set A: ";
	Z.print();
	std::cout << std::endl;

	std::cout << "Set B: ";
	set.print();
	std::cout << std::endl;

	auto intersection = set.interset(Z);
	std::cout << std::endl;
	std::cout << "A∩B: ";
	intersection.print();
	std::cout << std::endl;

	return 0;
}
