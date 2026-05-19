/*
	File: main.cpp
	Description: 9.13: LAB: Palindrome (deque)
	Author: Ian Moore
	Email: ianm6092@student.vvc.edu
	Course#: cis202
	Section#: 30000
	Date: 05/19/2026
*/

#include <iostream>
#include <deque>
#include <cctype>

using namespace std;

int main() {
   string line;
   bool result = true;

   getline(cin, line);
   auto fit = line.begin();
   auto eit = line.rbegin();

   for (;fit!=line.end();fit++,eit++) {
      if (!isalpha(*fit)) {
         fit++;
         if (fit == line.end()) break;
      }
      if (!isalpha(*eit)) {
         eit++;
         if (eit == line.rend()) break;
      }
      cout << *fit << ":" << *eit << endl;
      if (tolower(*fit) != tolower(*eit)) {
         result = false;
      }
   }
   
   cout << (result ? "Yes" : "No") 
        << ", \"" << line << "\" is " 
        << (result ? "" : "not") 
        << " a palindrome." << endl;

   return 0;
}

