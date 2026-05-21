/*
	File: main.cpp
	Description: 10.10: LAB: Descending selection sort with output during execution
	Author: Ian Moore
	Email: ianm6092@student.vvc.edu
	Course#: cis202
	Section#: 30000
	Date: 05/21/2026
*/

#include <iostream>
#include <algorithm>
using namespace std;

// TODO: Write a void function SelectionSortDescendTrace() that takes
//       an integer array and the number of elements in the array as arguments,
//       and sorts the array into descending order.
void SelectionSortDescendTrace(int numbers[], int numElements) {
   auto print = [&]() {
      for(int i=0; i < numElements; i++)
         cout << numbers[i] << " ";
      cout << endl;
   };

   int *begin = numbers;
   int *end = numbers + numElements;

   for (int i = 0; i < numElements - 1; i++) {
      auto it = std::max_element(begin + i, end);
      std::swap(*it, *(begin + i));
      print();
   }
}


int main() {
   //int input = 0;
   int i = 0;
   int numElements = 0;
   int numbers[11];

   // TODO: Read in a list of up to 10 positive integers; stop when
   //       -1 is read. Then call SelectionSortDescendTrace() function.

   for (;i<11;i++) {
      cin >> numbers[i];
      if (numbers[i]==-1) {
         numElements=i;
         break;
      }
   }

   SelectionSortDescendTrace(numbers, numElements);

  return 0;
}
