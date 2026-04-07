#include <vector>
#include <string>
#include <iostream>
#include <inttypes.h>
#include <cmath>

using color_t = uint8_t;

namespace Util {
    template<typename T>
    T clamp(const T &val, const T &min, const T &max) {
        if (val < min)
            return min;
        if (val > max)
            return max;
        return val;
    }

    template<typename T>
    T wrap(T val, const T &max) {
        while (val > max)
            val -= max;
        return val;
    }
}

/*
    Change to FTXUI type and modify references
    Or find compatible type to cast to for rendering
*/
struct CharColor {
    char character;
    color_t color;
};

template<typename T>
struct Vec2T {
    T x, y;

    T magnitude() const {
        return std::sqrt(magnitudeSq());
    }

    T magnitudeSq() const {
        return (x * x) + (y * y);
    }

    T distanceToSq(const Vec2T &other) const {
        Vec2T diff = this->operator-(other);
        return diff.magnitudeSq();
    }

    T distanceTo(const Vec2T &other) const {
        return std::sqrt(distanceToSq(other));
    }

    Vec2T operator+(const Vec2T &other) const {
        return { x + other.x, y + other.y };
    }

    Vec2T operator-(const Vec2T &other) const {
        return { x - other.x, y - other.y };
    }

    Vec2T operator*(const Vec2T &other) const {
        return { x * other.x, y * other.y };
    }

    Vec2T operator/(const Vec2T &other) const {
        return { x / other.x, y / other.y };
    }

    Vec2T operator+(const T &addend) const {
        return { x + addend, y + addend };
    }

    Vec2T operator-(const T &subtractend) const {
        return { x - subtractend, y - subtractend };
    }

    Vec2T operator*(const T &scalar) const {
        return { x * scalar, y * scalar};
    }

    Vec2T operator/(const T &divisor) const {
        return { x / divisor, y / divisor };
    }
};

template<typename T> using PosT = Vec2T<T>;

template<typename T, typename POS = PosT<T>>
struct SizeT {
    using position_type = POS;

    union {
        Vec2T<T> vec;
        struct { 
            T width, height;
        };
    };

    T area() const {
        return width * height;
    }

    T index(const position_type &p) const {
        return width * p.y + p.x;
    }

    explicit operator position_type() const {
        return position_type { width, height };
    }

    SizeT(const T &width, const T &height):
        width(width),
        height(height)
    {}

    SizeT(const position_type &pos):
        SizeT(pos.x, pos.y)
    {}
};

template<typename T, typename SIZE = SizeT<T>, typename POS = PosT<T>>
struct RectT {
    using position_type = POS;
    using size_type = SIZE;

    union {
        position_type pos;
        struct {
            T x, y;
        };
    };

    union {
        position_type size;
        struct {
            T width, height;
        };      
    };

    T left() const { return x; }

    T top() const { return y + height; }

    T right() const { return x + width; }

    T bottom() const { return y; }

    bool intersect(const RectT &other) {
        return left() <= other.right() &&
               right() >= other.left() &&
               bottom() <= other.top() &&
               top() >= other.bottom();
    }

    position_type center() {
        return pos + (size * 0.5);
    }

    explicit operator size_type() const {
        return size;
    }

    T area() const {
        return size_type(size).area();
    }
};

using sizei = SizeT<int>;
using posi = PosT<int>;
using recti = RectT<sizei, posi>;

using tile_id = short;

struct TileInstance {
    tile_id id;
    uint8_t fog:4;
    uint8_t light:4;
    uint8_t hp:4;
    uint8_t state:4;
};

struct TileBase {
    tile_id id;
    const char *name;
    std::vector<CharColor> textures;
    float animation_speed;
    /*
        We can have preset values for hp, state, lighting
    */

    /*
        We can immediately add a tile to the registry
        after construction if desired
    */
    TileBase(const char *name, 
             const std::vector<CharColor> &textures, 
             const float &animation_speed=0):
        name(name),
        textures(textures),
        animation_speed(animation_speed)
    {}

    /*
        Where the data for the map is created
    */
    virtual TileInstance getDefaultInstance() {
        return TileInstance {
            .id = this->id,
            .fog = 0,
            .light = 0,
            .hp = 0,
            .state = 0,
        };
    }

    /*
        To-Do color and fog processing
        FTXUI supports RGB?
    */
    CharColor getSpriteData(const TileInstance &inst, const posi &pos) const {
        const auto state = inst.state;
        if (state == 0)
            return textures[0];
        return textures[Util::wrap<uint8_t>(state, textures.size())];
    }
};

struct TileDirt : public TileBase {
    CharColor DIRT_TEXTURE{'$', 3};

    TileDirt():
        TileBase("Dirt", {DIRT_TEXTURE})
    {}
};

struct TileStone : public TileBase {
    TileStone():
        TileBase("Stone", {{'@', 1}})
    {}
};

struct TileWater : public TileBase {
    std::vector<CharColor> WATER_ANIMATION =
    {
        {'W', 4},
        {'W', 5},
        {'M', 9},
        {'M', 4},
        {'M', 5},
        {'N', 10},
        {'M', 5}
    };

    TileWater():
        TileBase("Water", WATER_ANIMATION)
    {}
};

template<typename T, typename V = std::vector<T*>>
struct RegistryT : public V {
    using id_type = decltype(T::id);
    V elements;

    id_type nextId = 0;

    T *add(T *tile) {
        tile->id = nextId++;
        elements.push_back(tile);
        
        return tile;
    }

    T *insertAt(T *tile, const id_type &id) {
        tile->id = id;

        if (id >= elements.size()) {
            elements.resize(id + 1, nullptr);
        }

        elements[id] = tile;

        if (id >= nextId) {
            nextId = id + 1;
        }

        return tile;
    }

    T *add(T &tile) {
        return this->add(&tile);
    }

    T *get(const id_type &id) {
        return elements.at(id);
    }

    T *operator[](const id_type &id) {
        return get(id);
    }
};

struct Registry {
    RegistryT<TileBase> tiles;
} registry;

struct Map {
    const sizei size;
    TileInstance *tiles;

    Map(const sizei &size):
        size(size) 
    {
        tiles = new TileInstance[size.area()];
    }

    ~Map() {
        delete [] tiles;
    }

    TileInstance &get(const posi &pos) {
        return tiles[size.index(pos)];
    }

    TileInstance &get(const int &index) {
        return tiles[index];
    }

    TileBase *getTile(const posi &pos) {
        const auto &tile = this->get(pos);
        return registry.tiles[tile.id];   
    }

    void setMap(const TileInstance &inst) {
        for (int i = 0; i < size.area(); i++)
            get(i) = inst;
    }
};

void printTile(TileBase *tile) {
    std::cout << "Tile id: " << tile->id << " " << tile->name << std::endl;
}

int main() {
    TileDirt dirt;
    TileStone stone;
    TileBase brick("Brick", {{'#', 2}});

    registry.tiles.add(dirt);
    registry.tiles.add(stone);
    registry.tiles.add(brick);

    printTile(registry.tiles[0]);
    printTile(registry.tiles[1]);

    Map map({50, 50});

    map.setMap(registry.tiles[2]->getDefaultInstance());

    printTile(map.getTile({5, 6}));

    return 0;
}