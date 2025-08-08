#include <stdio.h>
main(){
char*k[]={"Fizz","Buzz","FizzBuzz"},n=101,i=0,m;
for(;i<n;i++)(m=(1*!(i%3))+(2*!(i%5)))?puts(k[m-1]):printf("%i\n",i);
}
