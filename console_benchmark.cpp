#include <math.h>

#include "console.h"
#include "printbenchmark.h"

static int seed = 0;
static int width, height;

inline void xychrandom() {
    int x = rand() % width;
    int y = rand() % height;
    char ch = (rand() % (0x7E - 0x33)) + 0x33;

    console::write(x, y, ch, FWHITE|BBLACK);
}

inline void xycorandom() {
    int x = rand() % width;
    int y = rand() % height;
    color_t color = rand() % 16;

    console::write(x, y, 'X', color);
}

inline void xychcorandom() {
    int x = rand() % width;
    int y = rand() % height;
    char ch = (rand() % (0x7E - 0x33)) + 0x33;
    color_t color = rand() % 16;

    console::write(x, y, ch, color);
}

inline void bufferrandom() {
    int length = width * height;
    char fb[length];
    color_t cb[length];
    for (int i = 0; i < length; i++) {
        fb[i] = (rand() % (0x7E - 0x33)) + 0x33;
        cb[i] = rand() % 16;
    }

    console::write(fb, cb, length);
}

int main() {
    width = console::getConsoleWidth(), height = console::getConsoleHeight();
    srand(seed);

    using XYCHRANDOM = MakeFunc<xychrandom>;
    using XYCORANDOM = MakeFunc<xycorandom>;
    using XYCHCORANDOM = MakeFunc<xychcorandom>;
    using BUFFERRANDOM = MakeFunc<bufferrandom>;

    printBench<BUFFERRANDOM, 5>("bufferrandom", stderr);

    printBench<XYCHRANDOM, 100>("xychrandom", stderr);
    printBench<XYCORANDOM, 100>("xycorandom", stderr);
    printBench<XYCHCORANDOM, 100>("xychcorandom", stderr);
    printBench<BUFFERRANDOM, 10>("bufferrandom", stderr);

    printBench<XYCHRANDOM, 1000>("xychrandom", stderr);
    printBench<XYCORANDOM, 1000>("xycorandom", stderr);
    printBench<XYCHCORANDOM, 1000>("xychcorandom", stderr);
    printBench<BUFFERRANDOM, 100>("bufferrandom", stderr);

    printBench<XYCHRANDOM, 10000>("xychrandom", stderr);
    printBench<XYCORANDOM, 10000>("xycorandom", stderr);
    printBench<XYCHCORANDOM, 10000>("xychcorandom", stderr);
    printBench<BUFFERRANDOM, 200>("bufferrandom", stderr);

    printBench<XYCHRANDOM, 100000>("xychrandom", stderr);
    printBench<XYCORANDOM, 100000>("xycorandom", stderr);
    printBench<XYCHCORANDOM, 100000>("xychcorandom", stderr);
    printBench<BUFFERRANDOM, 300>("bufferrandom", stderr);


    return 0;
}