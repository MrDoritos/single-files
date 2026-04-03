#include <vector>
#include <string>
#include <iostream>

struct CharColor {
    char character;
    char color;
};

template<typename T>
struct PosT {
    T x, y;
};

template<typename T, typename P = PosT<T>>
struct SizeT {
    using pos = P;

    T width, height;

    T area() const {
        return width * height;
    }

    T index(const pos &p) const {
        return width * p.y + p.x;
    }
};

template<typename SIZE, typename POS>
struct RectT : public SIZE, public POS {
    using pos = POS;
    using size = SIZE;
};

using sizei = SizeT<int>;
using posi = PosT<int>;
using recti = RectT<sizei, posi>;

using tile_id = short;

struct TileInstance {
    tile_id id;
    char fog:4;
    char light:4;
    char hp:4;
    char state:4;
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
    using V::operator[];

    id_type nextId = 0;

    T *add(T *tile) {
        tile->id = nextId++;
        this->resize(nextId);
        this->at(tile->id) = tile;
        
        return tile;
    }

    T *add(T &tile) {
        return this->add(&tile);
    }

    T *get(const id_type &id) {
        return this->at(id);
    }
};

struct Registry {
    RegistryT<TileBase> tiles;
} registry;

struct Map : public sizei {
    TileInstance *tiles;

    Map(const sizei &size):
        sizei(size) 
    {
        tiles = new TileInstance[this->area()];
    }

    ~Map() {
        delete [] tiles;
    }

    TileInstance &get(const posi &pos) {
        return tiles[this->index(pos)];
    }

    TileInstance &get(const int &index) {
        return tiles[index];
    }

    TileBase *getTile(const posi &pos) {
        const auto &tile = this->get(pos);
        return registry.tiles[tile.id];   
    }

    void setMap(const TileInstance &inst) {
        for (int i = 0; i < this->area(); i++)
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