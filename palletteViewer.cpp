#include <typeinfo>
#include <string>
#include <concepts>
#include <functional>
#include <cstddef>

#include "../console-2/autoconsole.h"
#include "../console-2/buffers.h"

using color_t = cons::con_color;

#include "../imgcat/colorMappingFast.h"
//#include "../imgcat/colorMappingDither.h"

using namespace cons;
using namespace std;

pixel_image<cpix_wide> image_target;
buffer<cpix_wide> target;

bool active_colors[4] = {true, true, true, false};
bool graphics_change = true;

template<typename TB, typename TC>
concept is_sink = std::is_base_of<i_buffer_sink<TC>, TB>::value;

template<typename TB, typename TC> requires ( is_sink<TB, TC> )
void writeOrDiscard(TB* _b, TC _d, con_norm x, con_norm y) {
    _b->writeSample(x, y, _d);
}

template<typename TB, typename TC> requires ( !is_sink<TB, TC> )
void writeOrDiscard(TB* _b, TC _d, con_norm x, con_norm y) {
}

template<typename TB>
void displayGradient(TB *target) {
    con_norm sheight = target->getSampleHeight();
    con_norm swidth = target->getSampleWidth();
    for (con_norm y = 0; y < sheight; y+=target->getSampleHeightStep()) {
        for (con_norm x = 0; x < swidth; x+=target->getSampleWidthStep()) {
            pixel color;
            con_norm ix = (swidth - x), iy = (sheight - y);

            posf current = posf(x, y);

            posf top_left = posf(0, 0);
            posf bottom_left = posf(0, 1);
            posf bottom_right = posf(1, 1);
            posf top_right = posf(1, 0);
            posf center = posf(0.5, 0.5);

            con_norm div = 3;

            con_norm bf = -1.1;
            con_norm af = 0.0;
            con_norm ml = 1.5;
            
            pixel::value_type r = std::max((bf + (ml * current.distance(top_left)) + af) * 255, 0.f);
            pixel::value_type g = std::max((bf + (ml * current.distance(bottom_left)) + af) * 255, 0.f);
            pixel::value_type b = std::max((bf + (ml * current.distance(bottom_right)) + af) * 255, 0.f);
            con_norm a = std::min((0 + (-2.0f * current.distance(center))), 1.f);

            color = pixel(r, g, b, 255);

            for (int i = 0; i < 3; i++)
                ((pixel::value_type*)&color)[i] *= active_colors[i];
            if (!active_colors[3]) {
                color.a = 255;
                a = 1.0f;
            }

            color.r *= a;
            color.g *= a;
            color.b *= a;

            cpix_wide cpix;
            cpix.a = color.a;
            getDitherColored(color.r, color.g, color.b, &cpix.ch, &cpix.co);
            
            //target.writeSample(x, y, cpix);
            //target.writeSample(x, y, color);
            writeOrDiscard(target, cpix, x, y);
            writeOrDiscard(target, color, x, y);
        }
    }
}

int main() {
    auto key = con.readKeyAsync();
    image_target.make(500, 500);
    target.make(con.getDimensions());
    //characters = (wchar_t*)L" H#";
    //colormapper_init_table();

    while (key != 'q' && key != '\b') {
        key = con.readKeyAsync() & 0xFF;

        if (key-'1' > -1 && key-'1' < 4) {
            active_colors[key-'1'] = !active_colors[key-'1'];
            graphics_change = true;
        }

        if (key == 's') {
            std::string filename = to_string(time(0)) + "_gradient.png";
            displayGradient(&image_target);
            image_target.save(filename.c_str());
            fprintf(stderr, "Saved to %s\n", filename.c_str());
        }

        if (graphics_change) {
            graphics_change = false;

            target.clear();
            con.setCursor(0, 0);
            srand(0);

            displayGradient(&target);

            copyTo(target.buffer<cpix_wide>::sink(), con.sink_wcpix::sink());
        }

        con.sleep(10);
    }
}