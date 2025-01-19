#include <typeinfo>
#include <string>
#include <concepts>
#include <functional>
#include <cstddef>
#include <functional>

#include "../console-2/autoconsole.h"
#include "../console-2/buffers.h"
#include "../console-2/colors/colorMappingFast.h"
#include "../console-2/colors/colorMappingFaster.h"
#include "../console-2/colors/colorMappingDither.h"

using color_t = cons::con_color;

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
    image<pixel> *image_target;
    buffer<cpix_wide> *target;
    remap_func remap_rgb = 0;
    pixel remap_ramp(pixel&);
    pixel remap_comp(pixel&);
    pixel remap_fun(pixel&);
    pixel remap_none(pixel&);
    screen_handler *handler;
    sliders_object<slider_cnt> *sliders;
    pviewer_object *pviewer;
    gradient_object *gradient;
    color_map<wchar_t> *colormap;
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

            //getDitherColored(color.r, color.g, color.b, &cpix.ch, &cpix.co);
            cpix = state->colormap->getCpix(color);
            
            //target.writeSample(x, y, cpix);
            //target.writeSample(x, y, color);
            writeOrDiscard(target, cpix, x, y);
            writeOrDiscard(target, color, x, y);
        }
    }
}

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

template<typename iter>
static void incr(iter &objects) {
    int i = 0, c = 0, faa = -1, laa = -1, la = -1;
    
    for (auto *obj : objects) {
        c = i;
        i++;

        if (obj->locked)
            continue;

        if (faa < 0)
            faa = c;

        if (obj->active)
            la = c;
        else {
            laa = c;
            if (la >= 0)
                break;
        }
    }

    if (laa == la || faa < 0 || laa < 0)
        return;

    if (la > -1)
        objects[la]->active = false;

    if (la >= objects.size()-1 || la < 0)
        objects[faa]->active = true;
    else
        objects[laa]->active = true;
}

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

    void add(screen_object *obj) {
        objects.push_back(obj);
    }

    void add_if_not_exists(screen_object *obj) {
        for (auto *o : objects)
            if (o == obj)
                return;
        add(obj);
    }

    void remove(screen_object *obj) {
        for (int i = 0; i < objects.size(); i++)
            if (objects[i] == obj)
                objects.erase(objects.begin() + i);
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
            incr(objects);
            state->graphics_change = true;
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

struct list_item_object : public screen_object {
    list_item_object(sizef _size, std::string _text = "", bool _active = false, bool _locked = false)
        : screen_object(_size, _active) {
            setText(_text);
            locked = _locked;
        }

    virtual void setText(std::string _text, con_color color = 15) {
        text = _text;
        _tc.clear();
        for (auto &c : _text)
            _tc.push_back({c,color});
    }

    virtual con_pos getTextHeight(con_pos width) {
        return (text.size() / width) + 1;
    }

    std::string text;
    std::basic_string<cpix_wide> _tc;

    void display(i_buffer_sink_dim<cpix_wide> *sink) override {
        setText(text, active ? 13 : 15);
        sink->write(_tc.c_str(), sink->getSample(size), _tc.size());
    }
};

struct toggle_item_object : public list_item_object {
    typedef std::function<void(bool)> callback_type;

    toggle_item_object(std::string text, callback_type _callback) : list_item_object({0.,0.,1.,1.}) {
        setText(text);
        this->callback = _callback;
        this->toggle = false;
    }

    callback_type callback;
    bool toggle;

    bool keyboard(con_basic_key key) override {
        if (key == ' ') {
            callback(toggle = !toggle);
            state->graphics_change = true;
        } else
            return false;
        return true;
    }
};

struct text_item_object : public list_item_object {
    typedef std::function<void(std::basic_string<wchar_t>)> callback_type;

    text_item_object(std::string _prompt, std::basic_string<wchar_t> _entered, callback_type _callback) : list_item_object({0.,0.,1.,1.}) {
        setText(_prompt);
        this->prompt = _prompt;
        this->entered = _entered;
        this->callback = _callback;
    }

    void setText(std::string _text, con_color color = 15) override {
        text = _text;
        _tc.clear();
        for (auto &c : text)
            _tc.push_back({c,color});
        for (auto &c : entered)
            _tc.push_back({c,15});
    }

    std::string prompt;
    std::basic_string<wchar_t> entered;
    callback_type callback;

    bool keyboard(con_basic_key key) override {
        con_basic ascii = key & 0xFF;
        
        if (ascii == 0x7) {
            if (entered.size() > 0)
                entered.pop_back();
        } else {
            entered.push_back(ascii);
        }

        if (entered.size() > 0)
            callback(entered);

        state->graphics_change = true;
        setText(prompt);
        
        return true;
    }
};

struct list_object : public screen_object {
    list_object(sizef _size) : screen_object(_size) {}

    std::vector<list_item_object*> items;

    void display(i_buffer_sink_dim<cpix_wide> *sink) override {
        sizei scr = sizei{posi{},sink->getDimensions()};
        sizef p = size.denorm(scr);
        posf textOffset;
        int i = 0;
        for (auto *obj : items) {
            textOffset = textOffset + posf(0, obj->getTextHeight(p.width));
            obj->size = (p + textOffset).norm(scr);
            obj->display(sink);
        }
    }

    bool keyboard(con_basic_key key) override {
        if (key == KEY_DOWN) {
            incr(items);
            state->graphics_change = true;
        } else {
            for (auto *obj : items) {
                if (obj->active && obj->keyboard(key))
                    return true;
            }
        }
        return true;
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
            con_color _c = i == selected && active ? 15 : 9;

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
        mapping = "None";
    }

    void display(i_buffer_sink_dim<cpix_wide> *sink) {
        auto *active = state->handler->getActive();
        std::string _activestr = "Active: " + string(active ? active->name() : "None");
        std::string _messagestr = "Message: " + message;
        std::string _mappingstr = "Mapping: " + mapping;
        std::string _colormappingstr = "Color Mapping: " + colormap;
        std::string _keystr = "Keys: " + (to_string(_lkey) + " " + to_string(lkey));

        std::string _fin = _activestr + " | " + _messagestr + " | " + _mappingstr + " | " + _colormappingstr + " | " + _keystr;

        const char *str = _fin.c_str();
        int len = strlen(str);
        int x = 0;
        for (int i = 0; i < len; i++) {
            sink->write(x++, 0, cpix_wide(str[i], 0b11110000));
        }
    }

    con_basic_key _lkey, lkey;
    std::string message;
    std::string mapping;
    std::string colormap;

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

struct graph_object : public screen_object {
    graph_object(sizef _size) : screen_object(_size) { locked = true; }
    void display(i_buffer_sink_dim<cpix_wide> *sink) override {
        for (con_norm x = 0; x < size.width; x += sink->getSampleWidthStep()) {
            pixel cand = pixel(x * 255, 0, 0, 255);
            pixel test = std::invoke(state->remap_rgb, state, cand);
            con_norm y = 1-(test.r / con_norm(pixel::bit_max));
            cpix_wide ch;
            ch = state->colormap->getCpix(test);
            sink->writeSample(size.x + x, size.height * y - size.y, ch);
        }
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
            outputs[j] = t(outputs[j] + mul); 
        }
    }

    for (int i = 0; i < 3; i++)
        ;//outputs[i] = outn[i] * pixel::bit_max;
    
    return pixel(outputs[0], outputs[1], outputs[2], 255);
    //return pixel(inputs[0], inputs[1], inputs[2], 255);
}

template<template<typename> typename INB, typename INT,
         template<typename> typename OUTB, typename OUTT>
struct source_convert : OUTB<OUTT> {
    INB<INT> *source;

    source_convert(INB<INT> * b):source(b)  {}

    OUTT readSample(con_norm x, con_norm y) override {
        return OUTT(source->readSample(x, y));
    }

    OUTT read(con_pos x, con_pos y) override {
        return OUTT(source->read(x, y));
    }

    virtual OUTT convert(INT in) {
        return OUTT(in);
    }

    ssize_t read(OUTT* buf, size_t start, size_t count) override {
        INT inbuf[count];
        ssize_t cnt = source->read(&inbuf[0], start, count);
        for (int i = 0; i < count; i++)
            buf[i] = OUTT(inbuf[i]);
        return count;
    }

    ssize_t read(OUTT* buf, size_t count) override {
        INT inbuf[count];
        ssize_t cnt = source->read(&inbuf[0], count);
        for (int i = 0; i < count; i++)
            buf[i] = OUTT(inbuf[i]);
        return count;
    }
};

pixel program::remap_fun(pixel& p) {
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


    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < count; j++) {
            t upper = divr * (j+1);
            t lower = divr * j;
            t value = inputs[i] - t(lower);
            if (value < 0 || value >= divr)
                continue;
            outputs[i] += t(float(value) + ((slider[j] - 1) * divr));
            //outputs[i] = t(outputs[i] + value);
        }
    }


    return pixel(outputs[0], outputs[1], outputs[2], 255);
}

pixel program::remap_none(pixel& p) {
    return p;
}

void program::init() {
    graphics_change = true;
    for (int i = 0; i < 4; i++)
        active_colors[i] = true;
    active_colors[3] = false;
    image_target = new image<pixel>(500, 500);
    target = new buffer<cpix_wide>(con.getDimensions());
    
    sliders = new sliders_object<slider_cnt>({0.05, 0.66, 0.9, 0.22}, true);
    pviewer = new pviewer_object();
    gradient = new gradient_object({0.0, 0.0, 1.0, 0.5});

    graph_object *graph = new graph_object({0.2, 0.0, 0.7, 0.5});
    list_object *list = new list_object({0.0, 0.5, 1.0, 0.5});
    auto toggle_object = [](bool s, screen_object* obj) {
        if (s) state->handler->add_if_not_exists(obj); else state->handler->remove(obj);
    };

    static color_map<wchar_t>* color_maps[] = {
        new color_map_fast(),
        new color_map_faster(),
        new color_map_dither()
    };

    static int color_map_index = 0;

    static remap_func remaps[] = {
        &program::remap_none,
        &program::remap_ramp,
        &program::remap_comp,
        &program::remap_fun
    };

    static int remap_index = 0;

    list->items.push_back(new list_item_object({}, "[Settings]", false, true));
    list->items.push_back(new toggle_item_object("Exit", [](bool){ exit(0); }));
    list->items.push_back(new toggle_item_object("Remap Function", [](bool) {
        static std::string func_strings[] = {"None", "Ramp", "Comp", "Fun"};
        static int func_count = sizeof(remaps) / sizeof(remaps[0]);
        remap_index = ++remap_index % func_count;
        state->remap_rgb = remaps[remap_index];
        state->pviewer->mapping = func_strings[remap_index];
    }));
    list->items.push_back(new toggle_item_object("Color Map Function", [](bool) {
        static std::string func_strings[] = {"Fast", "Faster", "Dither"};
        static int func_count = sizeof(color_maps) / sizeof(color_maps[0]);
        color_map_index = ++color_map_index % func_count;
        state->colormap = color_maps[color_map_index];
        state->pviewer->colormap = func_strings[color_map_index];
    }));
    list->items.push_back(new text_item_object("Characters: ", L" LH", [](std::basic_string<wchar_t> s) {
        state->colormap->setCharacters(s.c_str());
    }));
    list->items.push_back(new toggle_item_object("Graph", [toggle_object,graph](bool s){ toggle_object(s, graph); }));
    list->items.push_back(new toggle_item_object("Sliders", [toggle_object](bool s){ toggle_object(s, state->sliders); }));
    list->items.push_back(new toggle_item_object("PViewer", [toggle_object](bool s){ toggle_object(s, state->pviewer); }));
    list->items.push_back(new toggle_item_object("Red", [toggle_object](bool s){ state->active_colors[0] = s; }));
    list->items.push_back(new toggle_item_object("Green", [toggle_object](bool s){ state->active_colors[1] = s; }));
    list->items.push_back(new toggle_item_object("Blue", [toggle_object](bool s){ state->active_colors[2] = s; }));
    list->items.push_back(new toggle_item_object("Alpha", [toggle_object](bool s){ state->active_colors[3] = s; }));

    handler = new screen_handler(gradient, pviewer, sliders, list);

    sliders->setSliders(prop::VALUE, 1.);
    sliders->setSliders(prop::STEP, 0.1);
    sliders->setSliders(prop::MIN, -1);
    sliders->setSliders(prop::MAX, 4);

    sliders->names[0] = "R/0";
    sliders->names[1] = "G/1";
    sliders->names[2] = "B/2";
    sliders->names[3] = "Rr/3";
    sliders->names[4] = "Gr/4";
    sliders->names[5] = "Br/5";
    sliders->names[6] = "X/6";
    sliders->names[7] = "Y/7";

    remap_rgb = remaps[0];
    colormap = color_maps[0];
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

        if (key || _key) {
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

        if (key == 0)
            con.sleep(10);
    }
}
