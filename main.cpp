// Non-standard raylib libraries
#include <raylib.h>
#include <raymath.h>

// Standard libraries
#include <math.h>
#include <vector>

// Limit GLSL version to 100
#define GLSL_VERSION 100

// Shader locations constants
#define SHADER_COUNT 2
#define WORLD_SHADER 0
#define MODEL_SHADER 1

// Local imports
#include "render/Renderer.cpp"
#include "player/Player.cpp"
#include "render/LightManager.cpp"
#include "object/GameObject.cpp"
#include "world/World.cpp"

// Global shader values
Vector3 fog_color = {0.7f, 0.5f, 0.5f};
float fog_amount = 0;

int main(void) {
    // Initialise the renderer
    Renderer renderer = Renderer(
        {1366, 768},
        "Crypt 3D",
        BLACK,
        SHADER_COUNT,
        FLAG_FULLSCREEN_MODE
    );

    // Initialise the player
    Player player = Player({0, 10, 0.1});
    
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

    // Init the first map
    World world = World(&renderer);
    world.texmap = texmap;
    world.world_shader = WORLD_SHADER;
    world.Load("resources/world/hub.map");

    Model gun = LoadModel("resources/models/rifle.obj");
    gun.materials[0].shader = renderer.shaders[MODEL_SHADER].shader;

    while(!WindowShouldClose()) {
        float deltat = GetFrameTime();

        renderer.shaders[WORLD_SHADER]("view", &player.position, SHADER_UNIFORM_VEC3);
        renderer.shaders[MODEL_SHADER]("view", &player.position, SHADER_UNIFORM_VEC3);

        renderer.BeginRender();
        {
            BeginMode3D(player.camera); 
            {
                gun.transform = MatrixRotateXYZ((Vector3){0, player.gun_rotation.x - player.input_axis.x / 20, player.gun_rotation.y - 0.1});
                DrawModel(gun, player.gun_position, 0.1, WHITE);
                
                world.Render();
            }

            EndMode3D();

            DrawFPS(3, 3);

            DrawText(
                (to_string(int(player.position.x*100)) + ", " + to_string(int(player.position.y*100)) + ", " + to_string(int(player.position.z*100))).c_str(),
                3,
                28,
                20,
                WHITE
            );

            DrawText(
                player.grounded ? "grounded" : "!grounded",
                3,
                53,
                20,
                WHITE
            );
        }
        renderer.StopRender();

        for(Model model : world.models)
            player.CheckCollision(model);

        player.Update();
    }

    renderer.Close();
}