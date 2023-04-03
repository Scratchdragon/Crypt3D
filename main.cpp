#define GLSL_VERSION 100

#include <raylib.h>
#include <math.h>

#include "render/renderer.cpp"
#include "render/world.cpp"
#include "player/player.cpp"
#include "render/lights.cpp"

Camera3D camera;
Renderer renderer;
Player player;
LightManager light_manager;

Vector3 fogColor = {0.7f, 0.5f, 0.5f};

float r = 0;

int main(void) {
    // Initialise the renderer
    renderer = Renderer(
        {800, 500},
        "Crypt 3D",
        BLACK,
        0
    );

    // Initialise the player
    player = Player({0, 10, 0.1});
    
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

    // Init the camera
    float zoom = 10;
    camera = {0};
    camera.position = (Vector3){ 0.0f, 10.0f, 10.0f };  // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

    Model model = LoadModel("resources/models/start_room.obj");
    model.materials[0].shader = shader;                     // Set shader effect to 3d model
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texmap;

    while(!WindowShouldClose()) {
        camera.position.x = sin(r) * zoom;
        camera.position.z = cos(r) * zoom;
        camera.position.y = zoom;

        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], &camera.position, SHADER_UNIFORM_VEC3);

        renderer.BeginRender();
        {
            DrawFPS(0, 0);

            BeginMode3D(player.camera); 
            {
                DrawModel(model, {0,-5,0}, 1, WHITE);
            }

            EndMode3D();
        }
        renderer.StopRender();

        if(IsKeyDown(KEY_LEFT))
            r -= 2.0f * GetFrameTime();
        if(IsKeyDown(KEY_RIGHT))
            r += 2.0f * GetFrameTime();

        if(IsKeyDown(KEY_UP))
            zoom -= 2.0f * GetFrameTime();
        if(IsKeyDown(KEY_DOWN))
            zoom += 2.0f * GetFrameTime();

        player.Update();
    }

    renderer.Close();
}