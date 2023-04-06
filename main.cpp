#define GLSL_VERSION 100

#include <raylib.h>
#include <raymath.h>

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
float fogAmount = 0;

float r = 0;

int main(void) {
    // Initialise the renderer
    renderer = Renderer(
        {1920, 1080},
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
    int fogamount_loc = GetShaderLocation(shader, "fogAmount");

    // Initial shader value set
    SetShaderValue(shader, texsize_loc, &texsize, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, gridsize_loc, &gridsize, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, texscale_loc, &texscale, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, fogcolor_loc, &fogColor, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, fogamount_loc, &fogAmount, SHADER_UNIFORM_FLOAT);

    // Initialise the lights manager
    light_manager = LightManager(&shader);
    light_manager.CreateLight(
        1.5,
        {0, 0, 0}
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
    model.materials[0].shader = shader;                     // Set shader effect to 3d model
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texmap;

    Model gun = LoadModel("resources/models/rifle.obj");
    gun.materials[0].shader = shader;
    gun.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texmap;

    while(!WindowShouldClose()) {
        player.Update();

        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], &player.position, SHADER_UNIFORM_VEC3);

        renderer.BeginRender();
        {
            BeginMode3D(player.camera); 
            {
                gun.transform = MatrixRotateXYZ((Vector3){0, player.gun_rotation.x, player.gun_rotation.y});
                DrawModel(gun, player.gun_position, 0.2, WHITE);

                DrawModel(model, {0,-5,0}, 1, WHITE);

                player.grounded = false;
                for(GenericBounds b : collision_map.bounds) {
                    // Don't check if far away
                    if(Distance(ExcludeY(player.position), ExcludeY(GetCenter(b))) > Distance(ExcludeY(b.min), ExcludeY(b.max))) continue;

                    // Draw wireframe if in debug mode
                    if(DEBUG) DrawGenericBounds(b, RED);

                    if(CheckCollisionBounds(player.feet, b)) {
                        player.grounded = true;
                        if(b.max.y > b.min.y)
                            player.position.y = b.max.y + 0.101f;
                        else
                            player.position.y = b.min.y + 0.101f;
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
        
        if(IsKeyDown(KEY_F1))
            collision_map = CollisionMap("resources/world/collision/start.cmap");
    }

    renderer.Close();
}