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
#include "world/Editor.cpp"
#include "world/Console.cpp"

// Global shader values
Vector3 fog_color = {0.7f, 0.5f, 0.5f};
float fog_amount = 0;

int main(void) {
    // Initialise the renderer
    Renderer renderer = Renderer(
        {0, 0},
        "Crypt 3D",
        BLACK,
        SHADER_COUNT,
        FLAG_VSYNC_HINT,
        true
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
    float pixscale = (float)texmap.width / texscale;
    Vector3 tint = {
        100,
        82,
        68
    };
    tint.x /= 255;
    tint.y /= 255;
    tint.z /= 255;

    // Make tint add up to 3.0
    float total = tint.x + tint.y + tint.z;
    tint.x *= 3.0/total;
    tint.y *= 3.0/total;
    tint.z *= 3.0/total;

    // Initial shader value set
    renderer.shaders[WORLD_SHADER]("texsize", &texsize, SHADER_UNIFORM_FLOAT);
    renderer.shaders[WORLD_SHADER]("gridsize", &gridsize, SHADER_UNIFORM_FLOAT);
    renderer.shaders[WORLD_SHADER]("texscale", &texscale, SHADER_UNIFORM_FLOAT);
    renderer.shaders[WORLD_SHADER]("fogColor", &fog_color, SHADER_UNIFORM_VEC3);
    renderer.shaders[MODEL_SHADER]("fogColor", &fog_color, SHADER_UNIFORM_VEC3);
    renderer.shaders[WORLD_SHADER]("fogAmount", &fog_amount, SHADER_UNIFORM_FLOAT);
    renderer.shaders[MODEL_SHADER]("fogAmount", &fog_amount, SHADER_UNIFORM_FLOAT);
    renderer.shaders[WORLD_SHADER]("pixscale", &pixscale, SHADER_UNIFORM_FLOAT);
    renderer.SetAllShaderVal("tint", &tint, SHADER_UNIFORM_VEC3);

    // Init the first map
    World world = World(&renderer, &player);
    world.texmap = texmap;
    world.world_shader = WORLD_SHADER;
    world.Load("resources/world/hub.map");

    Model gun = LoadModel("resources/models/rifle.obj");
    gun.materials[0].shader = renderer.shaders[MODEL_SHADER].shader;

    // Set the console's world pointer
    Console::world = &world;

    while(!WindowShouldClose()) {
        float deltat = GetFrameTime();

        renderer.shaders[WORLD_SHADER]("view", &player.position, SHADER_UNIFORM_VEC3);
        renderer.shaders[MODEL_SHADER]("view", &player.position, SHADER_UNIFORM_VEC3);

        renderer.BeginRender();
        {
            BeginMode3D(player.camera); 
            {
                gun.transform = MatrixRotateXYZ((Vector3){0, player.gun_rotation.x - player.input_axis.x / 20, player.gun_rotation.y - 0.1f});

                // Check if in edit mode
                if(world.edit) {
                    DrawGrid(5, 2);
                    Ray cameraLook = {
                        player.camera.position,
                        player.look
                    };

                    RayCollision finalCollide = GetRayCollisionBox(
                        cameraLook, 
                        {
                            {player.position.x - 100, 0, player.position.y - 100},
                            {player.position.x + 100, 0, player.position.y + 100}
                        }
                    );

                    for(Model model : world.models) {
                        RayCollision cameraCollide = GetRayCollisionMesh(cameraLook, model.meshes[0], model.transform);
                        if(cameraCollide.hit && cameraCollide.distance < finalCollide.distance)
                            finalCollide = cameraCollide;
                    }

                    if(IsKeyDown(KEY_LEFT_CONTROL)) {
                        Vector3 pos = {
                            floor(finalCollide.point.x / 2.0f) * 2 + 1.0f,
                            finalCollide.point.y,
                            floor(finalCollide.point.z / 2.0f) * 2 + 1.0f,
                        };
                        DrawSphere(pos, 0.1, (Color){255, 0, 0, 100});
                    }
                    else
                        DrawSphere(finalCollide.point, 0.1, (Color){255, 0, 0, 100});
                }
                // Only draw player weapon if not in edit mode
                else
                    DrawModel(gun, player.gun_position, 0.1, WHITE);

                world.Render();
            }
            EndMode3D();

            if(Console::open)
                Console::Render({(float)renderer.width, (float)renderer.height});

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

        if(!world.edit) {
            for(Model model : world.models)
                player.CheckCollision(model);
        }
        else {
            player.grounded = true; 
        }

        if(IsKeyPressed(KEY_GRAVE)) {
            Console::open = !Console::open;
            continue;
        }

        if(Console::open) {
            int key = GetKeyPressed();
            if(key == KEY_ESCAPE)
                Console::open = false;
            else if(key)
                Console::AddInput(key);
        }
        else
            player.Update();
    }

    renderer.Close();
}