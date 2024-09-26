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

struct screen_object {
    screen_object(sizef _size, bool _active = false) : size(_size), active(_active) {}
    screen_object(bool _active = false) : screen_object({0.,0.,1.,1.}, _active) {}
    bool active;
    sizef size;
    virtual void display(i_buffer_sink_dim<cpix_wide> *sink) = 0;
};

template<int sliderCount>
struct sliders_object : screen_object {
    sliders_object(sizef _size, bool _active = false) : screen_object(_size,_active) {
        setSliders(VALUE, 0);
        setSliders(MIN, -1);
        setSliders(MAX, 1);
        setSliders(STEP, 0.1);
    }

    enum prop {
        VALUE = 0,
        MIN,
        MAX,
        STEP,
        PROP_COUNT
    };

    float sliders[PROP_COUNT][sliderCount];

    void setSlider(int slider, prop p, float value) {
        sliders[p][slider] = value;
    }

    void setSliders(prop p, float value) {
        for (int i = 0; i < sliderCount; i++)
            sliders[p][i] = value;
    }

    float getSlider(int slider, prop p) {
        return sliders[p][slider];
    }

    void display(i_buffer_sink_dim<cpix_wide> *sink) override {
        for (int i = 0; i < sliderCount; i++) {
            con_norm xsize = size.width / sliderCount;
            con_norm ysize = size.height;
            con_norm xpos = xsize * i;
            con_norm ypos = 0;
            _2dsize<con_norm> _norm_rel_pos = 
                                   {xpos, ypos};
            auto _real_sink_size = sink->getDimensions();
            _2d<con_norm> _norm_abs_pos = 
                                   {size.x + _norm_rel_pos.x,
                                    size.y + _norm_rel_pos.y};
                                    

            for (con_norm y = 0; y < ysize; y += sink->getSampleHeightStep()) {
                //fprintf(stderr, "Slider %i: %f\n", i, getSlider(i, VALUE));
                sink->writeSample(y + _norm_abs_pos.y, _norm_abs_pos.x, cpix_wide(L'|', 4));
            }
        }
    }
};

int main() {
    auto key = con.readKeyAsync();
    image_target.make(500, 500);
    target.make(con.getDimensions());
    sliders_object<6> sliders({0.25, 0.66, 0.5, 0.22});
    //characters = (wchar_t*)L" H#";
    //colormapper_init_table();

    while (key != 'q' && key != '\b') {
        con_basic_key _key = con.readKeyAsync();
        key = _key & 0xFF;
        if (key) {
            fprintf(stderr, "Raw: %i (%c) Key: %i (%c) \n", _key, _key, key, key);
        }

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

        if (key == 0x9) {
            for (int i = 0; i < 4; i++)
                active_colors[i] = true;
            graphics_change = true;
        }

        if (graphics_change) {
            graphics_change = false;

            target.clear();
            con.setCursor(0, 0);
            srand(0);

            displayGradient(&target);

            copyTo(target.buffer<cpix_wide>::sink(), con.sink_wcpix::sink());
        }

        sliders.display(con.sink_wcpix::sink());

        con.sleep(10);
    }
}