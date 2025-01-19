#include <string>
#include "../console-2/autoconsole.h"
#include "../console-2/buffers.h"

using namespace cons;

template<typename Tcpix, int _depth, int _components>
struct palette {
    const int depth = depth;
    const int components = components;
    Tcpix data[_components][_depth];
};

typedef palette<cons::cpix_wide, 256, 3> default_palette;

void displaySelected(int y, default_palette* p, int index, int component) {
    con.write(0, y, std::to_string(index));
    int stop = std::min(index + con.getWidth(), p->depth);
    auto _r = std::to_string(stop);
    con.write(stop-_r.size()+1,y,_r);
    y++;
    
    for (int x = 0; x < stop; x++) {
        auto cpix = p->data[component][x];
        con.setCursor(x,y);
        con.setColor(cpix.co);
        con.write(x, y, cpix.ch, cpix.co);
    }
}

int main() {
    default_palette p;
    displaySelected(3, &p, 0, 1);
    con.sleep(-1);
}