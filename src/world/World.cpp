#include "render/LightManager.cpp"
#include "world/CollisionMap.cpp"
#include "object/GameObject.cpp"
#include "object/ObjectManager.cpp"

#define RUN_DIR "run/"

class World {
    public:
    ObjectManager object_manager;
    CollisionMap collision_map;
    LightManager light_manager;

    bool Load(const char * filename);

    private:
    void Reset();
    void Decompress(const char * filename, const char * destination);
};

bool World::Load(const char * filename) {
    Decompress(filename, RUN_DIR);
}

void World::Reset() {
    // Light manager has a built in reset method
    light_manager.Reset();

    // Object manager can be reset by just deleting all the objects
    object_manager.object_count = 0;
    object_manager.objects = {};
}