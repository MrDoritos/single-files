//#include <iostream>
//#include <math.h>
#include <cstring>
#include <cstdio>
//#include <stdlib.h>
//main(){int i;std::cin>>i;printf("%i\n",1+(uint)log10(i));}

//main(){int i;std::cin>>i;std::cout<<1+(uint)log10(i)<<'\n';}
//main(){std::string s;std::cin>>s;printf("%i\n",s.size());}
main(){long i;printf("%i\n",strlen(gets(i)));}

/*
int DigitCount(int num) {
    if (num < 10)
        return 1;
    return 1 + DigitCount(num / 10);
}

int main(int argc, char **argv) {
    int i;
    if (argc < 2) return -1;
    sscanf(argv[1], "%i", &i);
    printf("%i\n", DigitCount(i));
    return 0;
}
*/