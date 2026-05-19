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

   auto advalpha = [&](auto &it, const auto &it_end) {
      while (it != it_end) {
         if (isalpha(*it))
            return true;
         it++;
      }
      return false;
   };

   for (;fit!=line.end()&&eit!=line.rend();fit++,eit++) {
      if (!advalpha(fit,line.end())) break;
      if (!advalpha(eit,line.rend())) break;
      if (tolower(*fit) != tolower(*eit)) {
         result = false;
      }
   }
   
   cout << (result ? "Yes" : "No") 
        << ", \"" << line << "\" is" 
        << (result ? "" : " not") 
        << " a palindrome." << endl;

   return 0;
}

