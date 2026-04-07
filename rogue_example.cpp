#include <vector>
#include <string>
#include <iostream>
#include <inttypes.h>

struct CharColor {
    char character;
    char color;
};

template<typename T>
struct Vec2T {
    T x, y;
};

template<typename T> using PosT = Vec2T<T>;

template<typename T, typename P = PosT<T>>
struct SizeT {
    using pos = P;

    union {
        Vec2T<T> vec;
        struct { 
            T width, height; 
        };
    };

    T area() const {
        return width * height;
    }

    T index(const pos &p) const {
        return width * p.y + p.x;
    }
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
        size_type size;
        struct {
            T width, height;
        };      
    };
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
    /*
        We can have preset values for hp, state, lighting
    */

    /*
        We can immediately add a tile to the registry
        after construction if desired
    */
    TileBase(const char *name):
        name(name) 
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
};

struct TileDirt : public TileBase {
    TileDirt():
        TileBase("Dirt")
    {}
};

struct TileStone : public TileBase {
    TileStone():
        TileBase("Stone")
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
    TileBase brick("Brick");

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