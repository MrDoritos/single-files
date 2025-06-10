#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cmath>
#include <vector>
#include <tuple>
#include <algorithm>
#include <numeric>
#include <functional>
#include <cassert>

#include "../console/advancedConsole.h"

extern int errno;

struct Plot {
    using data_t = double;

    void init() {
        adv::setThreadSafety(false);
        adv::setThreadState(false);
    }
    
    void destroy() {
        adv::_advancedConsoleDestruct();
        console::cons.~constructor();
    }

    std::vector<std::tuple<data_t,data_t>> data;

    void load(FILE *fp) {
        using char_t = char;
        constexpr int LINELEN = 256;
        char buf[LINELEN];
        char *pbuf = &buf[0];

        char_t delim = ',';

        while ((fgets(pbuf, LINELEN, fp) != nullptr)) {
            data_t x, y;
            if (sscanf(pbuf, "%lf,%lf", &x, &y) != 2) {
                continue;
            }
            data.push_back({x,y});
        }
    }

    template<int t>
    data_t get_min() {
        data_t min = std::numeric_limits<data_t>::max();
        for (const auto &tup : data)
            if (std::get<t>(tup) < min)
                min = std::get<t>(tup);
        return min;
    }

    template<int t>
    data_t get_max() {
        data_t max = std::numeric_limits<data_t>::min();
        for (const auto &tup : data)
            if (std::get<t>(tup) > max)
                max = std::get<t>(tup);
        return max;
    }

    template<int t>
    data_t get_range() {
        return get_max<t>() - get_min<t>();
    }

    void render() {
        data_t offsetx = get_min<0>();
        data_t offsety = get_min<1>();
        data_t rangex = get_range<0>();
        data_t rangey = get_range<1>();
        data_t midpointx = rangex * 0.5;
        data_t midpointy = rangey * 0.5;

        int c_w = adv::width - 1;
        int c_h = adv::height - 1;
        int s_w = c_w;
        int s_h = c_h;
        int e_w = s_w * 0.5f;

        if (e_w > s_h)
            s_w = s_h * 2;
        if (e_w < s_h)
            s_h = e_w;

        int s_xo = (c_w - s_w) * 0.5f;
        int s_yo = (c_h - s_h) * 0.5f;

        constexpr int LINELEN = 256;
        char buf[LINELEN];
        char *pbuf = &buf[0];

        adv::border(s_xo - 1, s_yo - 1, s_xo + s_w + 1, s_yo + s_h + 1, FWHITE|BBLACK);

        adv::line(s_xo, s_h * .5 + s_yo, s_xo + s_w, s_h * .5 + s_yo, L'-', FRED|BBLACK);
        adv::line(s_w * .5 + s_xo, s_yo, s_w * .5 + s_xo, s_yo + s_h, L'|', FRED|BBLACK);

        for (const auto &tup : data) {
            data_t n_x = (std::get<0>(tup) - offsetx) / rangex;
            data_t n_y = (std::get<1>(tup) - offsety) / rangey;

            int s_x = n_x * s_w + s_xo;
            int s_y = n_y * s_h + s_yo;

            adv::write(s_x, s_y, L'x', FWHITE|BBLACK);
        }

        snprintf(pbuf, LINELEN, "%i+%i, %i+%i, %i, %i", s_w, s_xo, s_h, s_yo, adv::width, adv::height);
        adv::write(0,0,pbuf);
        snprintf(pbuf, LINELEN, "x: %.3g -> %.3g (%.3g), y: %.3g -> %.3g (%.3g)", 
            get_min<0>(), get_max<0>(), get_range<0>(),
            get_min<1>(), get_max<1>(), get_range<1>());
        adv::write(0,1,pbuf);
    }
} plot;

void error_exit(std::string text, int err = -1) {
    plot.destroy();
    std::cout << text << std::endl;
    exit(err);
}

void errno_exit(std::string text) {
    int errsv = errno;
    plot.destroy();
    std::cout << text << " (" << strerror(errsv) << ") " << std::endl;
    exit(errsv);
}


int main(int argc, char** argv) {
    Plot plot;
    bool useArgs = argc > 1;
    bool isPipe = !isatty(fileno(stdin));

    if (!useArgs && !isPipe)
        error_exit("Pipe or filename required");

    plot.init();

    if (isPipe) {
        plot.load(stdin);

        if (!freopen("/dev/tty", "r", stdin))
            errno_exit("Reopen /dev/tty failed");
    } else {
        error_exit("No data to plot");
    }

    int key = 0;

    while (true) {
        if (HASKEY(key, VK_ESCAPE) || HASKEY(key, 'q'))
            break;

        adv::isNewSize();

        adv::clear();

        plot.render();        

        adv::draw();

        key = console::readKey();
    }

    plot.destroy();

    return 0;
}