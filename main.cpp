// Non-standard raylib libraries
#include <raylib.h>
#include <raymath.h>

// Standard libraries
#include <math.h>
#include <vector>

// Local imports
#include "render/Renderer.cpp"
#include "player/Player.cpp"
#include "render/LightManager.cpp"
#include "world/CollisionMap.cpp"
#include "object/GameObject.cpp"

// Limit GLSL version to 100
#define GLSL_VERSION 100

// Shader locations constants
#define SHADER_COUNT 2
#define WORLD_SHADER 0
#define MODEL_SHADER 1

// Global shader values
Vector3 fog_color = {0.7f, 0.5f, 0.5f};
float fog_amount = 0;

int main(void) {
    // Initialise the renderer
    Renderer renderer = Renderer(
        {1920, 1080},
        "Crypt 3D",
        BLACK,
        SHADER_COUNT,
        0
    );

    SetTargetFPS(120);

    // Initialise the player
    Player player = Player({0, 10, 0.1});

    // Create the collision map
    CollisionMap collision_map = CollisionMap("resources/world/collision/start.cmap");
    
    // Load the world texture map
    Texture2D texmap = LoadTexture("resources/textures/texmap.png");

    // Setup the shaders
    renderer.InitShader(WORLD_SHADER, "resources/shaders/base.vs", "resources/shaders/world.fs");
    renderer.InitShader(MODEL_SHADER, "resources/shaders/base.vs", "resources/shaders/model.fs");

    // Set the inbuilt shader locations
    renderer.shaders[WORLD_SHADER].SetInbuiltLoc(SHADER_LOC_MATRIX_MODEL, "matModel");
    renderer.shaders[MODEL_SHADER].SetInbuiltLoc(SHADER_LOC_MATRIX_MODEL, "matModel");
    
    // All shader values
    float texsize = 31 / (float)texmap.width;
    float gridsize = 32 / (float)texmap.width;
    float texscale = 3;

    // Initial shader value set
    renderer.shaders[WORLD_SHADER]("texsize", &texsize, SHADER_UNIFORM_FLOAT);
    renderer.shaders[WORLD_SHADER]("gridsize", &gridsize, SHADER_UNIFORM_FLOAT);
    renderer.shaders[WORLD_SHADER]("texscale", &texscale, SHADER_UNIFORM_FLOAT);
    renderer.shaders[WORLD_SHADER]("fogColor", &fog_color, SHADER_UNIFORM_VEC3);
    renderer.shaders[MODEL_SHADER]("fogColor", &fog_color, SHADER_UNIFORM_VEC3);
    renderer.shaders[WORLD_SHADER]("fogAmount", &fog_amount, SHADER_UNIFORM_FLOAT);
    renderer.shaders[MODEL_SHADER]("fogAmount", &fog_amount, SHADER_UNIFORM_FLOAT);

    // Initialise the lights manager
    LightManager light_manager = LightManager(&renderer);
    light_manager.CreateLight(
        1,
        {0, -1.4f, 0}
    );

    light_manager.CreateLight(
        4,
        {0, 20, 0}
    );

    light_manager.CreateLight(
        -1.5,
        {0, -8, 0}
    );

    Model model = LoadModel("resources/models/start_room.obj");
    model.materials[0].shader = renderer.shaders[WORLD_SHADER].shader; // Set shader effect to 3d model
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texmap;

    Model gun = LoadModel("resources/models/rifle.obj");
    gun.materials[0].shader = renderer.shaders[MODEL_SHADER].shader;

    while(!WindowShouldClose()) {
        renderer.shaders[WORLD_SHADER]("view", &player.position, SHADER_UNIFORM_VEC3);
        renderer.shaders[MODEL_SHADER]("view", &player.position, SHADER_UNIFORM_VEC3);

        renderer.BeginRender();
        {
            BeginMode3D(player.camera); 
            {
                gun.transform = MatrixRotateXYZ((Vector3){0, player.gun_rotation.x-player.input_axis.x / 20, player.gun_rotation.y});
                DrawModel(gun, player.gun_position, 0.1, WHITE);

                DrawModel(model, {0,-5,0}, 1, WHITE);
            }

            EndMode3D();

            DrawText(
                (to_string(int(player.position.x*100)) + ", " + to_string(int(player.position.y*100)) + ", " + to_string(int(player.position.z*100))).c_str(),
                0,
                30,
                20,
                WHITE
            );

            DrawFPS(0, 0);

        }
        renderer.StopRender();

        player.Update();

        player.grounded = false;
        for(GenericBounds b : collision_map.bounds) {
            // Don't check if far away
            if(Distance(ExcludeY(player.position), ExcludeY(GetCenter(b))) > Distance(ExcludeY(b.min), ExcludeY(b.max))) continue;

            if(CheckCollisionBounds(player.feet, b)) {
                player.grounded = true;
                if(b.max.y > b.min.y)
                    player.position.y = b.max.y + 0.1f;
                else
                    player.position.y = b.min.y + 0.1f;
            }
            else if(CheckCollisionBounds(player.bounds, b)) {
                if(b.type == 1)
                    player.OnCollide(GetWallCollide(
                        player.bounds, {b.min, b.max}
                    ));
                else
                    player.OnCollide({b.min, b.max});
            }
        }
        
        if(IsKeyDown(KEY_F1))
            collision_map = CollisionMap("resources/world/collision/start.cmap");
    }

    renderer.Close();
}