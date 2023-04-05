#define GLSL_VERSION 100

#include <raylib.h>
#include <math.h>
#include <vector>

#include "render/renderer.cpp"
#include "player/player.cpp"
#include "render/lights.cpp"
#include "world/collision.cpp"

Camera3D camera;
Renderer renderer;
Player player;
LightManager light_manager;

CollisionMap collision_map;

Vector3 fogColor = {0.7f, 0.5f, 0.5f};

float r = 0;

int main(void) {
    // Initialise the renderer
    renderer = Renderer(
        {900, 500},
        "Crypt 3D",
        BLACK,
        0
    );

    SetTargetFPS(120);

    // Initialise the player
    player = Player({0, 10, 0.1});

    // Create the collision map
    collision_map = CollisionMap("resources/world/collision/start.cmap");
    
    // Load the world texture map
    Texture2D texmap = LoadTexture("resources/textures/texmap.png");

    // Setup the shaders
    Shader shader = LoadShader(
        TextFormat("resources/shaders/fragment/base.vs", 
        GLSL_VERSION), 
        TextFormat("resources/shaders/fragment/world.fs", 
        GLSL_VERSION)
    );
    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "view");
    
    // All shader values
    float texsize = 31 / (float)texmap.width;
    float gridsize = 32 / (float)texmap.width;
    float texscale = 3;

    // All shader value locations 
    int texsize_loc = GetShaderLocation(shader, "texsize");
    int gridsize_loc = GetShaderLocation(shader, "gridsize");
    int texscale_loc = GetShaderLocation(shader, "texscale");
    int fogcolor_loc = GetShaderLocation(shader, "fogColor");

    // Initial shader value set
    SetShaderValue(shader, texsize_loc, &texsize, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, gridsize_loc, &gridsize, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, texscale_loc, &texscale, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, fogcolor_loc, &fogColor, SHADER_UNIFORM_VEC3);

    // Initialise the lights manager
    light_manager = LightManager(&shader);
    light_manager.CreateLight(
        1,
        {0, 0, 0}
    );

    light_manager.CreateLight(
        4,
        {0, 20, 0}
    );

    Model model = LoadModel("resources/models/start_room.obj");
    model.materials[0].shader = shader;                     // Set shader effect to 3d model
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texmap;

    while(!WindowShouldClose()) {
        player.Update();

        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], &player.position, SHADER_UNIFORM_VEC3);

        renderer.BeginRender();
        {
            DrawFPS(0, 0);

            BeginMode3D(player.camera); 
            {
                DrawBoundingBox(player.feet, GREEN);
                DrawBoundingBox(player.bounds, ORANGE);

                player.grounded = false;
                for(BoundingBox box : collision_map.bounds) {
                    DrawBoundingBox(box, RED);
                    if(CheckCollisionBoxes(box, player.feet)) {
                        player.grounded = true;
                        if(box.max.y > box.min.y)
                            player.position.y = box.max.y + 0.101f;
                        else
                            player.position.y = box.min.y + 0.101f;
                    }
                    else if(CheckCollisionBoxes(box, player.bounds))
                        player.OnCollide(box);
                }

                if(CheckWallCollision(player.bounds, {{1, -1, -1}, {5, 2, 3}}, 200)) {
                    player.OnCollide(GetWallCollide(
                        player.bounds, {{1, -1, -1}, {5, 2, 3}}, 200
                    ));
                }
                
                DrawModel(model, {0,-5,0}, 1, WHITE);

                DrawBoundingWall(
                    {{1, -1, -1}, {5, 2, 3}},
                    RED
                );
            }

            EndMode3D();

            collision_map = CollisionMap("resources/world/collision/start.cmap");
        }
        renderer.StopRender();
    }

    renderer.Close();
}