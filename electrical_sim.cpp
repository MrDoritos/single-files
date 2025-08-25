#include <stdio.h>
#include <stdlib.h>

#include <cmath>

#include "../console/advancedConsole.h"
#include "../imgcat/colorMappingPalette.h"

typedef float nt;

namespace Util {
    template<typename IType, typename RType = IType, typename FType = float>
    constexpr inline RType lerp(const IType &a, const IType &b, const FType &factor) {
        return a * (1 - factor) + b * factor;
    }

    template<typename IType, typename RType = IType, typename CalcType = IType>
    constexpr inline RType length2(const IType &x0, const IType &y0, const IType &x1, const IType &y1) {
        return (CalcType)(x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
    }

    template<typename IType, typename RType = IType, typename CalcType = IType>
    constexpr inline RType length(const IType &x0, const IType &y0, const IType &x1, const IType &y1) {
        return std::sqrt(length2<IType, RType, CalcType>(x0, y0, x1, y1));
    }
}

struct Board {
    const int width, height;
    nt *board;

    Board(const int &width, const int &height):
        width(width),height(height),board(nullptr) {
        board = new nt[width * height];
    }

    ~Board() {
        if (board)
            delete [] board;
    }

    inline constexpr int get_index(const int &x, const int &y) const { return y * width + x; }

    inline constexpr nt get(const int &x, const int &y) const { return board[get_index(x, y)]; }

    inline nt &get(const int &x, const int &y) { return board[get_index(x, y)]; }

    inline void set(const int &x, const int &y, const nt &value) { board[get_index(x, y)] = value; }

    inline constexpr int get_width() const { return width; }

    inline constexpr int get_height() const { return height; }

    inline constexpr int get_size() const { return this->get_width() * this->get_height(); }

    inline void clear(const nt &value = 0) { for (int i = 0; i < this->get_size(); board[i] = value, i++); }

    inline void draw(const int &x1, const int &y1, const int &x2, const int &y2) {
        for (int x = x1; x < x2; x++) {
            for (int y = y1; y < y2; y++) {
                const nt v = get(x, y);
                wchar_t ch; color_t co;
                getDitherColored(v < 0 ? -v * 255 : 0, v < 0 ? 0 : v * 255, 0, &ch, &co);
                adv::write(x, y, ch, co);
            }
        }
    }
};

int main(int argc, char **argv) {
    colormapper_init_table();

    Board board(25, 25);

    board.clear();

    float cx = 12.5;
    float cy = 12.5;
    float r = 4.5;

    for (int x = 0; x < board.get_width(); x++) {
        for (int y = 0; y < board.get_height(); y++) {
            const auto d = Util::length<float>(cx, cy, x, y);
            const auto rt = (d-r) / -r;
            if ((rt < 0.0 ? -rt : rt) > 1.0)
                continue;
            board.set(x, y, rt);
        }
    }

    board.draw(0, 0, board.width, board.height);

    console::readKey();

    return 0;
}