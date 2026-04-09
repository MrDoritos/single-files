/*
	File: main.cpp
	Description: 12.1: LAB: Convert to binary - functions
	Author: Ian Moore
	Email: ianm6092@student.vvc.edu
	Course#: cis202
	Section#: 30000
	Date: 04/09/2026
*/

#include <iostream>
#include <string>
using namespace std;

string IntToReverseBinary(int integerValue) {
   string ret;
   for(;integerValue;integerValue/=2)ret+='0'+(integerValue&1);
   return ret;
}

string StringReverse(string userString) {
   return string(userString.rbegin(), userString.rend());
}

int main() {
   int num;
   cin >> num;
   cout << StringReverse(IntToReverseBinary(num)) << endl;

   return 0;
}

