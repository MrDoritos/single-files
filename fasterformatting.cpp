#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <unistd.h>
#include <limits>
#include <cassert>

inline int digit_count(const int &val, const int &base=10) {
    int i = 1, w = val / base;
    for (;w;w /= base,i++);
    return i;
}

inline char *itoa(const int &val, char *buf, const int &base=10) {
    const int sign = val < 0 ? 1 : 0;
    const int l = digit_count(val, base) + sign;
    int w = sign ? -val : val, i = l;
    buf[0] = sign ? '-' : '0';

    for (; w && i; --i, w /= base)
        buf[i-1] = "0123456789abcdef"[w % base];
    
    buf[l] = 0;
    
    return buf;
}	

namespace {
    template<typename T>
    inline void getModesR(char *buf, int &index, const int &length, T num) {
        const int sign = num < 0 ? 1 : 0;
        const int l = digit_count(num) + sign;

        if (index + l + 1 > length)
            return;

        itoa(num, buf+index);
        index += l;

        if (index + 1 > length)
            return;

        buf[index++] = ';';
    }

    template<typename T, typename ...Modes>
    inline void getModesR(char *buf, int &index, const int &length, T value, Modes... modes) {
        getModesR(buf, index, length, value);
        getModesR(buf, index, length, modes...);
    }
}

template<typename T, typename ...Modes>
inline char *getModesRE(char *buf, const int &length, T value, Modes... modes) {
    const char *escp = "[[";
    const int escpl = strlen(escp);

    if (escpl + 1 > length)
        return buf;

    strcpy(buf, escp);
    int i = escpl;
    
    getModesR(buf, i, length, value, modes...);

    if (i + 1 > length) {
        buf[length-1] = 0;
        return buf; 
    }

    buf[i-1] = 'm';
    buf[i] = 0;

    return buf;
}

template<typename ...Modes>
inline char *getModes(char *buf, const int &dest_length, Modes... modes) {
    //strcpy(buf, "\x1b[");
    //memcpy(buf, "Hello", 6);
    int off=0;
    //((off+=snprintf(buf+off, 30,"%i", modes), off+=strlen(strcpy(buf+off, ";"))), ...);
    ((
        off+=strlen(itoa(modes,buf+off)),
        off+=strlen(strcpy(buf+off,";")),
        off=(off+1>dest_length)?0:off),
     ...);

    if (sizeof...(modes)>0)
        buf[off-1]=0;
        
    return buf;
}

int main() {
    volatile int buflen = 50;
    {
        char buf[buflen];
        memset(buf, 'x', buflen);
        buf[0] = '>';
        getModesRE(buf+1, buflen-1, 1, -2, 999, 33, -80, 3432432,0,8);
        puts(buf);
    }
    return 0;
}