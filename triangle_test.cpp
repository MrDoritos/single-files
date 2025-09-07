#include <iterator>
#include <algorithm>
#include <initializer_list>

#include "../console/advancedConsole.h"
#include <math.h>

namespace Util {
    template<typename T, typename U, typename V, typename RT = T>
    constexpr inline RT lerp(const T &a, const U &b, const V &factor) {
        return (a * (V(1.0) - factor)) + (b * (factor));
    }
}

struct vec2 {
    float x,y;

    static vec2 get_random() {
        vec2 ret {
            rand() / float(RAND_MAX),
            rand() / float(RAND_MAX)
        };

        return ret;
    }

    vec2 min(const vec2 &other) {
        return vec2 { x < other.x ? x : other.x,
                      y < other.y ? y : other.y };
    }

    vec2 max(const vec2 &other) {
        return vec2 {
            x > other.x ? x : other.x,
            y > other.y ? y : other.y
        };
    }

    vec2 lerp(const vec2 &other, const float &factor) {
        return vec2::lerp(*this, other, factor);
    }

    static vec2 lerp(const vec2 &a, const vec2 &b, const float &factor) {
        return vec2 {
            Util::lerp(a.x, b.x, factor),
            Util::lerp(a.y, b.y, factor)
        };
    }

    template<typename T>
    static vec2 min(std::initializer_list<T> list) {
        vec2 ret = *list.begin();

        for (const auto &v : list) ret = ret.min(v);

        return ret;
    }

    template<typename T>
    static vec2 max(std::initializer_list<T> list) {
        vec2 ret = *list.begin();

        for (const auto &v : list) ret = ret.max(v);

        return ret;
    }

    friend vec2 operator*(const vec2 &a, const vec2 &b) {
        return vec2 {a.x * b.x, a.y * b.y};
    }
};

void draw_line(const vec2 &a, const vec2 &b, const vec2 &scale, const wchar_t &character = L'#', const color_t &color = FWHITE|BBLACK) {
    adv::line(a.x * scale.x, a.y * scale.y, b.x * scale.x, b.y * scale.y, character, color);
}

void draw_rectangle(const vec2 &a, const vec2 &b, const vec2 &scale, const wchar_t &character = L'*', const color_t &color=FWHITE|BBLACK) {
    adv::rectangle(a.x * scale.x, a.y * scale.y, b.x * scale.x, b.y * scale.y, character, color);
}

int debug_y=0;

template<int buflen=100, typename Format, typename ...Args>
void debug_text(Format format, const Args&... args) {
    char buf[buflen];
    snprintf(buf, buflen, format, args...);
    adv::write(0, debug_y++, buf, BWHITE|FBLACK);
}

void draw_triangle(const vec2 &a, const vec2 &b, const vec2 &c) {
    vec2 scale { adv::width, adv::height };

    //draw_line(a, b, scale);
    //draw_line(b, c, scale);
    //draw_line(c, a, scale);

    //draw_rectangle(vec2::min({a,b,c}), vec2::max({a,b,c}), scale);

    vec2 A = a * scale, B = b * scale, C = c * scale;

    if (B.x < A.x) std::swap(A, B);

    if (C.x < A.x) std::swap(A, C);

    if (C.x < B.x) std::swap(B, C);

    //adv::write(A.x, A.y, 'A');
    //adv::write(B.x, B.y, 'B');
    //adv::write(C.x, C.y, 'C');

    color_t color = rand() % 255;

    const float rAB = B.x - A.x;
    const float rBC = C.x - B.x;
    const float rAC = C.x - A.x;

    const float iAB = 1.0f / rAB;
    const float iBC = 1.0f / rBC;
    const float iAC = 1.0f / rAC;

    for (float x = 0.0; x < rAB; x += 1) {
        const float fAB = x * iAB;
        const float fAC = x * iAC;

        float y0 = Util::lerp(A.y, B.y, fAB);
        float y1 = Util::lerp(A.y, C.y, fAC);

        if (y0 > y1) std::swap(y0, y1);

        for (int y = y0; y < y1; y++)
            adv::write(x + A.x, y, 'x', color);
    }

    for (float x = 0.0; x < rBC; x += 1) {
        const float fBC = x * iBC;
        const float fAC = (x + rAB) * iAC;

        float y0 = Util::lerp(B.y, C.y, fBC);
        float y1 = Util::lerp(A.y, C.y, fAC);

        if (y0 > y1) std::swap(y0, y1);

        for (int y = y0; y < y1; y++)
            adv::write(x + B.x, y, 'x', color);
    }
}

int main() {
    adv::setThreadState(false);
    adv::setThreadSafety(false);

    int key = 0;

    while (true) {

        switch (key) {
            case 'q':
            case 'Q':
            case '\x1b':
                goto end;
            default:
                break;
        }

        //debug_y = 0;

        adv::clear();

        draw_triangle(vec2::get_random(), vec2::get_random(), vec2::get_random());

        adv::draw();

        key = NOMOD(console::readKey());
        //console::sleep(1000);
    }

    end:;

    adv::_advancedConsoleDestruct();

    return 0;
}