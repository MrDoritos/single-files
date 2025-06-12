#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int get_utf8_codepoint(void *buf, int &cn) {
    char *utf8 = (char*)buf;
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
        ret =  utf8[0];
        ret <<= 6;
        ret |= utf8[1] & 0b111111;
        return ret;
    } else if (b7 && b6 && b5 && !b4) {
        cn = 3;
        ret =  utf8[0] & 0b1111;
        ret <<= 6;
        ret |= utf8[1] & 0b111111;
        ret <<= 6;
        ret |= utf8[2] & 0b111111;
        return ret;
    } else if (b7 && b6 && b5 && b4 && !b3) {
        cn = 4;
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

std::string get_hex(const void *_buf, const int &len) {
    const unsigned char *bytes = (const unsigned char*)_buf;
    std::string ret = "";
    for (int i = 0; i < len; i++) {
        const int buflen = 8;
        char buf[buflen];
        memset(buf, 0, buflen);
        printf("%i ", int(bytes[i]));
        int l = snprintf(buf, buflen, "%02X", bytes[i]);
        ret += std::string(buf);
        if (i + 1 < len)
            ret += std::string(" ");
    }
    return ret;
}

int main() {
    const int buflen = 256;
    char buf[256];

    init();

    while (1) {
        poll(&pfd, 1, -1);
        memset(buf, 0, buflen);
        ssize_t c = read(ifd, buf, buflen-1);
        buf[c] = '\0';

        if (c == -1)
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            else if (errno == EINTR)
                break;

        if (c == 0)//█▓
            break;

        int n;
        int cp = get_utf8_codepoint(buf, n);
        n = n > 2 ? n - 1 : n; 
        int *pcp = &cp;
        std::string a = get_hex(buf, c);
        std::string b = get_hex(pcp, n);
        std::string bits_a = get_bits(buf, c);
        std::string bits_b = get_bits(pcp, n);

        printf("bytes: %i\ninput %s\ninput %i\ninput %s\ninput %s\ncodepoint %i\ncodepoint %s\ncodepoint %s\n",
            int(c),
            buf,
            int(*(int*)buf) & ((1 << (c*8))-1),
            a.c_str(),
            bits_a.c_str(),
            cp,
            b.c_str(),
            bits_b.c_str()
        );
        
        
    }

    destroy();
}