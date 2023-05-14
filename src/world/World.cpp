#define RUN_DIR "run/"

#include "render/LightManager.cpp"
#include "object/GameObject.cpp"
#include "object/ObjectManager.cpp"
#include "math/vec.hpp"
#include "render/Renderer.cpp"

#include <raylib.h>
#include <raymath.h>

#include <vector>

using namespace std;


class World {
    public:
    ObjectManager object_manager;
    LightManager light_manager;
    vector<Model> models;
    int world_shader;
    Texture2D texmap;

    World(Renderer * renderer);

    void Load(const char * filename);
    void Render() {
        for(Model model : models)
            DrawModel(model, {0}, 1, WHITE);
    }

    private:
    void Reset();
    Renderer * renderer;
};

World::World(Renderer * renderer) {
    this->renderer = renderer;
    light_manager = LightManager(renderer);
}

void World::Load(const char * filename) {
    int line_count = 0;
    const char ** lines_c = TextSplit(LoadFileText(filename), '\n', &line_count);
    string * lines = new string[line_count];

    // Move to string arrow to de-allocate char ptr
    for(int i = 0; i < line_count; ++i)
        lines[i] = lines_c[i];
    
    for(int i = 0; i < line_count; ++i) {
        int count = 0;
        const char ** tokens_c = TextSplit(lines[i].c_str(), ' ', &count);
        if(count != 3) continue;

        string * tokens = new string[count];
        for(int token_i = 0; token_i < count; ++token_i)
            tokens[token_i] = tokens_c[token_i];

        Vector3 position = Vec3FromString(tokens[2]);

        switch(lines[i][0]) {
            case 'M':
                models.push_back(LoadModel(tokens[1].c_str()));
                models[models.size() - 1].transform = MatrixTranslate(position.x, position.y, position.z);
                models[models.size() - 1].materials[0].shader = renderer->shaders[world_shader].shader; // Set shader effect to 3d model
                models[models.size() - 1].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texmap;
                break;
            case 'L':
                light_manager.CreateLight(
                    stof(tokens[1].c_str()),
                    position
                );
                break;
            case 'O':
                break;
            default:
                continue;
        }
    }
}

void World::Reset() {
    // Light manager has a built in reset method
    light_manager.Reset();

    // Object manager can be reset by just deleting all the objects
    object_manager.object_count = 0;
    object_manager.objects = {};
}