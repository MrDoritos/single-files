#include <iostream>
main(){char b,h,i=1,w,bw=5,hw=4;std::cin>>b>>h;for(;i<hw*2&&(w=(i>hw?hw-(i%hw):i));i++)std::cout<<std::string(bw,w>2?b:32)<<std::string(w,h)<<'\n';}
