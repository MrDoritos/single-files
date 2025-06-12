#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <queue>
#include <algorithm>

int get_utf8_codepoint(const void *buf, const ssize_t &length, int &cn) {
    cn = 0;

    if (length < 1)
        return 0;
    
    const char *utf8 = (const char*)buf;
    cn = 1;
    
    if ((utf8[0] & 0x80) == 0)
        return utf8[0];
        
    const bool b7 = 1;
    const bool b6 = (utf8[0] & 0b01000000)>0;
    const bool b5 = (utf8[0] & 0b00100000)>0;
    const bool b4 = (utf8[0] & 0b00010000)>0;
    const bool b3 = (utf8[0] & 0b00001000)>0;

    int ret = 0;
    if (b7 && b6 && !b5) {
        cn = 2;
        if (length < 2) {
            return 0;
        }
        ret =  utf8[0];
        ret <<= 6;
        ret |= utf8[1] & 0b111111;
        return ret;
    } else if (b7 && b6 && b5 && !b4) {
        cn = 3;
        if (length < 3)
            return 0;
        ret =  utf8[0] & 0b1111;
        ret <<= 6;
        ret |= utf8[1] & 0b111111;
        ret <<= 6;
        ret |= utf8[2] & 0b111111;
        return ret;
    } else if (b7 && b6 && b5 && b4 && !b3) {
        cn = 4;
        if (length < 4)
            return 0;
        ret =  utf8[0] & 0b111;
        ret <<= 6;
        ret |= utf8[1] & 0b111111;
        ret <<= 6;
        ret |= utf8[2] & 0b111111;
        ret <<= 6;
        ret |= utf8[3] & 0b111111;
        return ret;
    } else 
        return -1;
}

int ifd, c_lflag, pollflags;
pollfd pfd;
nfds_t nfds;

void init() {
    ifd = fileno(stdin);

    termios s;
    tcgetattr(ifd, &s);
    c_lflag = s.c_lflag;
    s.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr(ifd, TCSANOW, &s);

    pollflags = fcntl(ifd, F_GETFL, 0);
    pfd.fd = ifd;
    pfd.events = POLLIN;
    fcntl(ifd, F_SETFL, pollflags | O_NONBLOCK);
}

void destroy() {
    termios s;
    tcgetattr(ifd, &s);
    s.c_lflag = c_lflag;
    tcsetattr(ifd, TCSANOW, &s);

    fcntl(ifd, F_SETFL, pollflags);
}

std::string get_bits(const void *_buf, const int &len) {
    const unsigned char *bytes = (const unsigned char*)_buf;
    std::string ret = "";
    for (int i = 0; i < len; i++) {
        for (int b = 7; b > -1; b--) {
            ret += bytes[i] & (1 << b) ? '1' : '0';
        }
        if (i + 1 < len)
            ret += ' ';
    }
    return ret;
}

std::string get_hex(const void *_buf, const int &len, bool dir = false) {
    const unsigned char *bytes = (const unsigned char*)_buf;
    std::string ret = "";
    //for (int i = 0; i < len; i++) {
    int i = dir * (len-1);
    while (1) {
        if ((dir && i < 0) || (!dir && i > len))
            break;
        const int buflen = 8;
        char buf[buflen];
        memset(buf, 0, buflen);
        printf("%i ", int(bytes[i]));
        int l = snprintf(buf, buflen, "%02X", bytes[i]);
        ret += std::string(buf);
        if ((i + 1 < len && !dir) || (i > 0 && dir))
            ret += std::string(" ");
        i += dir ? -1 : 1;
    }
    return ret;
}

int printchinfo(const char *buf, const int &c) {
    int n;
    int cp = get_utf8_codepoint(buf, c, n);

    if (cp == 0 || cp == -1)
        return cp;

    int sl = n > 2 ? n - 1 : n;
    int *pcp = &cp;
    std::string a = get_hex(buf, n);
    std::string b = get_hex(pcp, sl, true);
    std::string bits_a = get_bits(buf, n);
    std::string bits_b = get_bits(pcp, sl);

    char chs[n+1];
    memcpy(chs, buf, n);
    chs[n] = '\0';

    printf("bytes: %i\ninput %s\ninput %i\ninput %s\ninput %s\ncodepoint %i\ncodepoint %s\ncodepoint %s\n",
        int(c),
        chs,
        int(*(int*)chs) & ((1 << (n*8))-1),
        a.c_str(),
        bits_a.c_str(),
        cp,
        b.c_str(),
        bits_b.c_str()
    );

    return n;
}

int main() {
    const int buflen = 256;
    char buf[256];
    memset(buf, 0, buflen);

    init();
    int bufpos = 0;

    while (1) {
        poll(&pfd, 1, -1);
        int to_read = buflen - 1 - bufpos;

        if (to_read < 1)
            break;

        ssize_t c = read(ifd, buf+bufpos, to_read);
        int total = c + bufpos;
        buf[total] = '\0';

        printf("%s\n", get_bits(buf, c).c_str());

        if (c == -1)
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            else if (errno == EINTR)
                break;

        if (c == 0)//█▓
            break;

        int n;
        
        int chs_avail = total;
        int chpos = 0;
        while (chs_avail > 0 && bufpos >= 0) {
            int used = printchinfo(buf+chpos, chs_avail);
            printf("used %i chs_avail %i chpos %i total %i bufpos %i c %i to_read %i\n", used, chs_avail, chpos, total, bufpos, c, to_read);
            if (used < 0) used = 1;
            if (used > chs_avail)
                break;
            if (bufpos - used >= 0)
                bufpos -= used;
            chs_avail -= used;
            chpos += used;
        }        
    }

    destroy();
}