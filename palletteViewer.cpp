#include <typeinfo>
#include <string>
#include <concepts>
#include <functional>
#include <cstddef>

#include "../console-2/autoconsole.h"
#include "../console-2/buffers.h"

using color_t = cons::con_color;

#ifndef DITHERING
#include "../imgcat/colorMappingFast.h"
#else
#include "../imgcat/colorMappingDither.h"
#endif

using namespace cons;
using namespace std;

struct program;
struct screen_object;
struct screen_handler;
template<int sliderCount>
struct sliders_object;
struct pviewer_object;
struct gradient_object;

typedef pixel(program::*remap_func)(pixel&);
const int slider_cnt = 8;

struct program {
    bool active_colors[4];
    bool graphics_change;
    pixel_image<cpix_wide> *image_target;
    buffer<cpix_wide> *target;
    remap_func remap_rgb = 0;
    pixel remap_ramp(pixel&);
    pixel remap_comp(pixel&);
    screen_handler *handler;
    sliders_object<slider_cnt> *sliders;
    pviewer_object *pviewer;
    gradient_object *gradient;
    void init();
} *state;

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

            color.r = (x * pixel::bit_max);

            for (int i = 0; i < 3; i++)
                ((pixel::value_type*)&color)[i] *= state->active_colors[i];
            if (!state->active_colors[3]) {
                color.a = 255;
                a = 1.0f;
            }

            color.r *= a;
            color.g *= a;
            color.b *= a;

            cpix_wide cpix;
            cpix.a = color.a;

            //color = state->*remap_rgb(color);
            color = std::invoke(state->remap_rgb, state, color);

            getDitherColored(color.r, color.g, color.b, &cpix.ch, &cpix.co);
            
            //target.writeSample(x, y, cpix);
            //target.writeSample(x, y, color);
            writeOrDiscard(target, cpix, x, y);
            writeOrDiscard(target, color, x, y);
        }
    }
}

struct our_key {
    
};

struct screen_object {
    screen_object(sizef _size, bool _active = false, bool _render = true) {
        size = _size;
        active = _active;
        render = _render;
        locked = false;
        ignorekeys = false;
    }
    screen_object(bool _active = false, bool _render = false) : screen_object({0.,0.,1.,1.}, _active, _render) {}
    bool active, render, locked, ignorekeys;
    sizef size;

    virtual const char* name() { return typeid(*this).name(); }
    virtual void display(i_buffer_sink_dim<cpix_wide> *sink) {}
    virtual bool keyboard(con_basic_key key) { return false; }
};

struct screen_handler {
    screen_handler() {}
    template<typename... Args> 
    screen_handler(Args... screens) : objects({screens...}) {}
    std::vector<screen_object*> objects;
    void display(i_buffer_sink_dim<cpix_wide> *sink) {
        for (auto *obj : objects)
            if (obj->render)
                obj->display(sink);
    }

    void incr() {
        int i = 0, c = 0, faa = -1, laa = -1, la = -1;
        
        for (auto *obj : objects) {
            c = i;
            i++;

            if (obj->locked)
                continue;

            if (obj->active)
                la = c;
            else
                laa = c;

            if (faa < 0)
                faa = c;
        }

        if (laa == la || faa < 0 || laa < 0)
            return;

        if (i >= objects.size())
            objects[faa]->active = true;
        else
            objects[laa]->active = true;
    }

    screen_object *getActive() {
        for (auto *obj : objects)
            if (obj->active)
                return obj;
        return nullptr;
    }

    bool keyboard(con_basic_key _key) {
        char key = _key & 0xFF;
        if (key == 0x9) {
            incr();
            return true;
        }

        for (auto *obj : objects) {
            if (obj->active && !obj->ignorekeys)
                if (obj->keyboard(_key))
                    return true;
        }

        for (auto *obj : objects) {
            if (!obj->ignorekeys)
                if (obj->keyboard(_key))
                    return true;
        }

        return false;
    }
};

template<int sliderCount>
struct sliders_object : public screen_object {
    sliders_object(sizef _size, bool _active = false) : screen_object(_size,_active) {
        setSliders(VALUE, 0);
        setSliders(MIN, -1);
        setSliders(MAX, 1);
        setSliders(STEP, 0.1);
        selected = 0;
    }

    enum prop {
        VALUE = 0,
        MIN,
        MAX,
        STEP,
        PROP_COUNT
    };

    float sliders[PROP_COUNT][sliderCount];
    std::string names[sliderCount];
    int selected;

    const int slider_count = sliderCount;

    void setSlider(int slider, prop p, float value) {
        sliders[p][slider] = value;
    }

    void setSliders(prop p, float value) {
        for (int i = 0; i < sliderCount; i++)
            sliders[p][i] = value;
    }

    float getSliderNorm(int slider) {
        float value = sliders[VALUE][slider];
        float min = sliders[MIN][slider];
        float max = sliders[MAX][slider];

        return (value - min) / (max - min);
    }

    void getSliders(prop p, float *_out) {
        for (int i = 0; i < sliderCount; i++)
            _out[i] = getSlider(i, p);
    }

    float getSlider(int slider, prop p) {
        return sliders[p][slider];
    }

    bool keyboard(con_basic_key key) override {
        float prevValue = getSlider(selected, VALUE);
        float step = getSlider(selected, STEP);
        if (key == ',' || key == '.') {
            step *= 10;
        } 
        if (key == 'l' || key == ';') {
            step *= 0.1;
        }
        if (key == 't' || key == KEY_UP || key == ',' || key == 'l') {
            setSlider(selected, VALUE, prevValue + step);
        } else
        if (key == 'g' || key == KEY_DOWN || key == '.' || key == ';') {
            setSlider(selected, VALUE, prevValue - step);
        } else
        if (key == 'f' || key == KEY_LEFT) {
            selected = (selected - 1 + sliderCount) % sliderCount;
        } else
        if (key == 'h' || key == KEY_RIGHT) {
            selected = (selected + 1) % sliderCount;
        } else
            return false;

        state->graphics_change = true;
        return true;
    }

    void display(i_buffer_sink_dim<cpix_wide> *sink) override {
        for (int i = 0; i < sliderCount; i++) {
            con_color _c = i == selected && active ? 15 : 7;

            con_norm xsize = size.width / con_norm(sliderCount);
            con_norm ysize = size.height;
            con_norm xpos = xsize * con_norm(i);
            con_norm ypos = 0;
            _2d<con_norm> _norm_rel_pos = 
                                   {xpos, ypos};
            auto _real_sink_size = sink->getDimensions();
            _2d<con_norm> _norm_abs_pos = 
                                   {size.x + _norm_rel_pos.x,
                                    size.y + _norm_rel_pos.y};
                                    
            float xhalf = xsize / 2;

            for (con_norm y = 0; y < ysize; y += sink->getSampleHeightStep()) {
                sink->writeSample(_norm_abs_pos.x + xhalf, y + _norm_abs_pos.y, cpix_wide(L'|',_c));
            }

            float _valuey = (1-getSliderNorm(i)) * ysize;

            for (con_norm x = xsize*.25; x < xsize*.75; x += sink->getSampleWidthStep()) {
                sink->writeSample(_norm_abs_pos.x + x, _norm_abs_pos.y, cpix_wide(L'-', _c));
                sink->writeSample(_norm_abs_pos.x + x, _norm_abs_pos.y + ysize, cpix_wide(L'-', _c));
                sink->writeSample(_norm_abs_pos.x + x, _norm_abs_pos.y + _valuey, cpix_wide(L'-', _c));
            }

            float _chw = sink->getSampleWidthStep();
            int _ccnt = xsize / _chw;
            {
                auto valstr = __gnu_cxx::__to_xstring<string>(&std::vsnprintf, _ccnt, "%.2f", getSlider(i, VALUE));
                std::string info = names[i] + ": " + valstr;
                const char *str = info.c_str();
                int l = strlen(str);
                int ox = l / 2;
                int scrx = sink->getWidth(_norm_abs_pos.x + xhalf);
                int scry = sink->getHeight(_norm_abs_pos.y + ysize);
                for (int j = 0; j < l; j++) {
                    //sink->writeSample(_norm_abs_pos.x + xhalf - ox + (float(j) * _chw), _norm_abs_pos.y + ysize, cpix_wide(_b[j], 13));
                    sink->write(scrx - ox + j, scry, cpix_wide(str[j], 13));
                }
            }
        }
    }
};

using prop = sliders_object<slider_cnt>::prop;

struct pviewer_object : public screen_object {
    pviewer_object() : screen_object({0.,0.,1.,1.}) { 
        locked = true;
        message = "Initialized";
    }

    void display(i_buffer_sink_dim<cpix_wide> *sink) {
        auto *active = state->handler->getActive();
        std::string _activestr = "Active: " + string(active ? active->name() : "None");
        std::string _messagestr = "Message: " + message;
        std::string _keystr = "Keys: " + (to_string(_lkey) + " " + to_string(lkey));

        std::string _fin = _activestr + " | " + _messagestr + " | " + _keystr;

        const char *str = _fin.c_str();
        int len = strlen(str);
        int x = 0;
        for (int i = 0; i < len; i++) {
            sink->write(x++, 0, cpix_wide(str[i], 0b11110000));
        }
    }

    con_basic_key _lkey, lkey;
    std::string message;

    bool keyboard(con_basic_key _key) override {
        con_basic_key key = _key & 0xFF;
        _lkey = _lkey;
        lkey = key;

        if (key-'1' > -1 && key-'1' < 4) {
            state->active_colors[key-'1'] = !state->active_colors[key-'1'];
            state->graphics_change = true;
        } else
        if (key == 's') {
            std::string filename = to_string(time(0)) + "_gradient.png";
            displayGradient(state->image_target);
            state->image_target->save(filename.c_str());
            message = "Saved to " + filename;
            state->graphics_change = true;
        } else
        if (key == 0x9) {
            for (int i = 0; i < 4; i++)
                state->active_colors[i] = true;
            state->graphics_change = true;
        } else
            return false;
        return true;
    }
};

struct gradient_object : public screen_object {
    gradient_object(sizef _size) : screen_object(_size) { locked = true; }
    void display(i_buffer_sink_dim<cpix_wide> *sink) override {
        displayGradient(sink);
    }
};

pixel program::remap_ramp(pixel& p) {
    float inn[] = 
               {float(p.r) / pixel::bit_max,
                float(p.g) / pixel::bit_max,
                float(p.b) / pixel::bit_max};

    pixel::value_type outputs[3];
    
    float outn[3];

    float slider[] = 
                   {sliders->getSlider(0, prop::VALUE),
                    sliders->getSlider(1, prop::VALUE),
                    sliders->getSlider(2, prop::VALUE)};

    float ramp[] = 
                   {sliders->getSlider(3, prop::VALUE),
                    sliders->getSlider(4, prop::VALUE),
                    sliders->getSlider(5, prop::VALUE)};

    for (int i = 0; i < 3; i++) {
        outn[i] = inn[i] * slider[i];
        outn[i] = pow(outn[i], ramp[i]);

        outputs[i] = outn[i] * pixel::bit_max;
    }

    return pixel(outputs[0], outputs[1], outputs[2], 255);
}

pixel program::remap_comp(pixel& p) {
    float inn[] = 
               {float(p.r) / pixel::bit_max,
                float(p.g) / pixel::bit_max,
                float(p.b) / pixel::bit_max};

    typedef pixel::value_type t;

    t outputs[3] = {0,0,0};
    t inputs[3] = {p.r,p.g,p.b}; //{p.r, p.g, p.b};
    
    float outn[3] = {1,1,1};

    const int count = sliders->slider_count;

    float slider[count];

    sliders->getSliders(prop::VALUE, &slider[0]);

    float div = 1.0f / count;
    t divr = 256 / count;

    for (int i = 0; i < count; i++) {
        for (int j = 0; j < 3; j++) {
            int n = count - i;
            t upper = divr * (i+1);
            t lower = divr * i;
            int value = inputs[j] - int(lower);
            if (value < 0)
                continue;
                //value = 0;
            t range = std::max(std::min((t)value, t(divr-1)), t(0));
            float norm = range / float(divr);
            //t mul = t(slider[i] * divr * 0.5) + range;
            //t mul = pow(range, slider[i]);

            t mul = (slider[7]-pow(slider[6]-norm, slider[i])) * divr;

            outn[j] *= (1-pow(1-(value/float(divr)), slider[i]));
            outputs[j] += mul; 
        }
    }

    for (int i = 0; i < 3; i++)
        ;//outputs[i] = outn[i] * pixel::bit_max;
    
    return pixel(outputs[0], outputs[1], outputs[2], 255);
    //return pixel(inputs[0], inputs[1], inputs[2], 255);
}

void program::init() {
    graphics_change = true;
    for (int i = 0; i < 4; i++)
        active_colors[i] = true;
    active_colors[3] = false;
    image_target = new pixel_image<cpix_wide>(500, 500);
    target = new buffer<cpix_wide>(con.getDimensions());
    sliders = new sliders_object<slider_cnt>({0.1, 0.66, 0.8, 0.22}, true);
    pviewer = new pviewer_object();
    gradient = new gradient_object({0.0, 0.0, 1.0, 0.5});
    handler = new screen_handler(gradient, pviewer, sliders);

    sliders->setSliders(prop::VALUE, 1.);
    sliders->setSliders(prop::STEP, 0.1);
    sliders->setSliders(prop::MIN, -1);
    sliders->setSliders(prop::MAX, 4);

    sliders->names[0] = "Red/0";
    sliders->names[1] = "Green/1";
    sliders->names[2] = "Blue/2";
    sliders->names[3] = "RRamp/3";
    sliders->names[4] = "GRamp/4";
    sliders->names[5] = "BRamp/5";
    sliders->names[6] = "X-shift";
    sliders->names[7] = "Y-shift";

    remap_rgb = &program::remap_ramp;
    remap_rgb = &program::remap_comp;
}

int main() {
    auto key = con.readKeyAsync();

    state = new program();
    state->init();

    #ifdef DITHERING
    characters = (wchar_t*)L" 39";
    //characters = (wchar_t*)L" ▒█"; //full block character
    #endif

    while (key != 'q' && key != '\b') {
        con_basic_key _key = con.readKeyAsync();
        key = _key & 0xFF;

        if (key) {
            state->handler->keyboard(_key);
            state->pviewer->lkey = key;
            state->pviewer->_lkey = _key;
        }

        if (state->graphics_change) {
            state->target->clear();
            con.setCursor(0, 0);
            srand(0);

            state->handler->display(state->target->buffer<cpix_wide>::sink());

            copyTo(state->target->buffer<cpix_wide>::sink(), con.sink_wcpix::sink());

            state->graphics_change = false;
        }

        con.sleep(10);
    }
}