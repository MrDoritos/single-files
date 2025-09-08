#include <iterator>
#include <algorithm>
#include <initializer_list>
#include <vector>
#include <functional>

#include "../console/advancedConsole.h"
#include <math.h>
#include <cmath>

#include <tiny_obj_loader.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace Util {
    template<typename T, typename U, typename V, typename RT = T>
    constexpr inline RT lerp(const T &a, const U &b, const V &factor) {
        return (a * (V(1.0) - factor)) + (b * (factor));
    }

    template<typename T, typename U, typename V, typename RT = T>
    constexpr inline RT wrap(const T &v, const U &min, const V &max) {
        RT ret = v;
        const auto r = max - min;
        while (ret < min) ret += r;
        while (ret > max) ret -= r;
        return ret;
    }

    template<typename T, typename U, typename V, typename RT = T>
    constexpr inline RT clip(const T &v, const U &min, const V &max) {
        RT ret = v;
        if (ret < min) ret = min;
        if (ret > max) ret = max;
        return ret;
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

    friend vec2 operator*(const vec2 &a, const float &b) {
        return vec2 {a.x * b, a.y * b};
    }

    friend vec2 operator+(const vec2 &a, const vec2 &b) {
        return vec2 {a.x + b.x, a.y + b.y};
    }
};

struct vertex_t {
    glm::vec3 vertex, normal;
    glm::vec2 tex;
    glm::vec3 color;
};

struct mesh_t {
    std::vector<vertex_t> verts;
    int vert_count;
    glm::mat4 model;

    bool load(const char *filepath) {
        return mesh_t::load(filepath, *this);
    }

    static bool load(const char *filepath, mesh_t &out_mesh) {
        tinyobj::attrib_t inattrib;
        std::vector<tinyobj::shape_t> inshapes;
        std::vector<tinyobj::material_t> inmaterials;
        
        std::string warn, err;
        bool ret = tinyobj::LoadObj(&inattrib, &inshapes, &inmaterials, &warn, &err, filepath, ".");

        if (!ret) return false;

        inmaterials.push_back(tinyobj::material_t());

        auto &attrib = inattrib;
        auto &materials = inmaterials;
        auto &verticies = out_mesh.verts;

        for (size_t s = 0; s < inshapes.size(); s++) {
            auto &shape = inshapes[s];
            auto &mesh = shape.mesh;
            auto &verts = attrib.vertices;
            auto &norms = attrib.normals;
            auto &texs = attrib.texcoords;
            auto &index = mesh.indices;
            auto vert_count = index.size();
            auto tri_count = vert_count / 3;

            verticies.reserve(verticies.size() + vert_count);

            for (size_t f = 0; f < tri_count; f++) {
                int material_id = mesh.material_ids[f];
                auto *ii = &index[3 * f];
                auto i0 = ii[0];
                glm::vec<3, decltype(i0)> is = {ii[0], ii[1], ii[2]};

                if (material_id < 0 || material_id >= materials.size())
                    material_id = materials.size() - 1;

                glm::vec3 color;
                for (int i = 0; i < 3; i++) color[i] = materials[material_id].diffuse[i];

                vertex_t vnt[3];

                for (int k = 0; k < 3; k++) {
                    glm::ivec3 vi, ni, ti;

                    vnt[k].color = color;
                    for (int i = 0; i < 3; i++) {
                        vi[i] = is[i].vertex_index;
                        ni[i] = is[i].normal_index;
                        ti[i] = is[i].texcoord_index;

                        auto &vert = vnt[i];
                        vert.vertex[k] = verts[3 * vi[i] + k];
                        vert.normal[k] = norms[3 * ni[i] + k];
                        if (k < 2) vert.tex[k] = texs[2 * ti[i] + k];
                    }
                }

                for (int i = 0; i < 3; i++) verticies.push_back(vnt[i]);
            }    
        }
        
        out_mesh.vert_count = verticies.size();

        return true;    
    }
};

struct fragment_t {
    float depth;
};

struct renderctx_t {
    glm::mat4 model, view, projection;
    glm::vec3 position, up, front, right;
    float yaw, pitch, fov, width, height, near, far;

    renderctx_t(float yaw, float pitch, float fov, float width, float height, float near, float far)
    :yaw(yaw),pitch(pitch),fov(fov),width(width),height(height),near(near),far(far),
    up(0,1.0,0),position(0) {
    }

    void camera_matrix() {
        yaw = Util::wrap(yaw, -180, 180);
        pitch = Util::clip(pitch, -89.9f, 89.9f);

        front = glm::normalize(
            glm::vec3(
                std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch)),
                std::sin(glm::radians(pitch)),
                std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch))
            )
        );

        right = glm::normalize(glm::cross(front, up));
    }

    void projection_matrix() {
        projection = glm::perspective(glm::radians(fov), width / height, near, far);
    }

    void view_matrix() {
        view = glm::lookAt(position, position + front, up);
    }

    void camera_move(int key) {
        float factor = 1.0;
        float dt = 1.0;

        glm::vec3 front = this->front; front.y = 0;
        glm::vec3 right = this->right; right.y = 0;
        front = glm::normalize(front);
        right = glm::normalize(right);

        switch (key) {
            case 'W':
            case 'S':
            case 'A':
            case 'D':
                factor = 2.0;
            break;
        }

        switch (key) {
            case 'W':
            case 'w':
                position += front * dt * factor;
                break;
            case 'S':
            case 's':
                position -= front * dt * factor;
                break;
            case 'A':
            case 'a':
                position -= right * dt * factor;
                break;
            case 'D':
            case 'd':
                position += right * dt * factor;
                break;
        }
    }

    void camera_pan(int key) {
        float factor = 1.0;

        float dx = 0, dy = 0;

        switch (key) {
            case 'i': dy = 1; break;
            case 'k': dy = -1; break;
            case 'j': dx = -1; break;
            case 'l': dx = 1; break;
        }

        yaw += factor * dx;
        pitch += factor * dy;
    }

    void keyboard(int key) {
        camera_move(key);
        camera_pan(key);
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

template<typename callback_t>
void draw_triangle_cb(const vec2 &a, const vec2 &b, const vec2 &c, callback_t callback) {
    //vec2 scale { adv::width, adv::height };

    //draw_line(a, b, scale);
    //draw_line(b, c, scale);
    //draw_line(c, a, scale);

    //draw_rectangle(vec2::min({a,b,c}), vec2::max({a,b,c}), scale);

    //vec2 A = a * scale, B = b * scale, C = c * scale;

    vec2 A = a, B = b, C = c;

    if (B.x < A.x) std::swap(A, B);

    if (C.x < A.x) std::swap(A, C);

    if (C.x < B.x) std::swap(B, C);

    //adv::write(A.x, A.y, 'A');
    //adv::write(B.x, B.y, 'B');
    //adv::write(C.x, C.y, 'C');

    //color_t color = rand() % 255;

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

        for (float y = y0; y < y1; y += 1)
            //adv::write(x + A.x, y, 'x', color);
            callback(x + A.x, y);
    }

    for (float x = 0.0; x < rBC; x += 1) {
        const float fBC = x * iBC;
        const float fAC = (x + rAB) * iAC;

        float y0 = Util::lerp(B.y, C.y, fBC);
        float y1 = Util::lerp(A.y, C.y, fAC);

        if (y0 > y1) std::swap(y0, y1);

        for (float y = y0; y < y1; y += 1)
            //adv::write(x + B.x, y, 'x', color);
            callback(x + B.x, y);
    }
}

void px_callback(const color_t &color, const float &x, const float &y) {
    if (adv::bound(x, y))
        adv::write(x, y, 'x', color);
}

void draw_triangle(const vec2 &a, const vec2 &b, const vec2 &c) {
    const color_t color = rand() % 255;
    const vec2 scale = {adv::width, adv::height};

    draw_triangle_cb(a * scale, b * scale, c * scale, [&](const float &x, const float &y) {
        if (adv::bound(x, y))
            adv::write(x, y, 'x', color);
    });
}

void draw_triangle(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c) {
    const color_t color = rand() % 255;
    const vec2 scale = {adv::width, adv::height};
    const vec2 shift = scale * 0.5;

    draw_triangle_cb(vec2{a.x, a.y} * scale + shift, vec2{b.x, b.y} * scale + shift, vec2{c.x, c.y} * scale + shift, std::bind(px_callback, color, std::placeholders::_1, std::placeholders::_2));
}

void render(renderctx_t &ctx, mesh_t &mesh) {
    ctx.camera_matrix();
    ctx.projection_matrix();
    ctx.view_matrix();

    const int tri_count = mesh.vert_count / 3;

    const auto mvp = ctx.model * ctx.view * ctx.projection;

    for (int t = 0; t < tri_count; t++) {
        const auto *tris = &mesh.verts[t * 3];

        glm::vec4 ps[3];
        for (int v = 0; v < 3; v++) {
            const auto vert = tris[v].vertex;
            ps[v] = glm::vec4(vert, 1) * mvp;
            debug_text("%.1f %.1f %.1f", ps[v].x, ps[v].y, ps[v].z);
            debug_text("%.1f %.1f %.1f", vert.x, vert.y, vert.z);
        }

        /*
        debug_text("%.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f",
            ps[0].x, ps[0].y, ps[0].z,
            ps[1].x, ps[1].y, ps[1].z,
            ps[2].x, ps[2].y, ps[2].z);
        */

        draw_triangle(ps[0], ps[1], ps[2]);

        for (int v = 0; v < 3; v++) {}
    }
}

int main() {
    adv::setThreadState(false);
    adv::setThreadSafety(false);

    mesh_t mesh;
    mesh.load("cave.obj");

    renderctx_t ctx(0, 0, 90, adv::width, adv::height, 0.1, 1000.0);
    ctx.model = glm::mat4(1.);

    int key = 0;

    while (true) {

        switch (key) {
            case 'q':
            case 'Q':
            case '\x1b':
                goto end;
            default:
                ctx.keyboard(key);
                break;
        }

        debug_y = 0;

        adv::clear();

        //draw_triangle(vec2::get_random(), vec2::get_random(), vec2::get_random());
        render(ctx, mesh);

        adv::draw();

        key = NOMOD(console::readKey());
        //console::sleep(1000);
    }

    end:;

    adv::_advancedConsoleDestruct();

    return 0;
}