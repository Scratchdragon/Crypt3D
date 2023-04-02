#include <raylib.h>
#include <math.h>

#define GLSL_VERSION            100

#include "render/renderer.cpp"
#include "render/world.cpp"

Camera3D camera;
Renderer renderer;

float r = 0;

int main(void) {
    // Init the renderer
    renderer = Renderer(
        {800, 500},
        "Crypt 3D",
        0
    );

    Shader shader = LoadShader(
        TextFormat("resources/shaders/fragment/base.vs", 
        GLSL_VERSION), 
        TextFormat("resources/shaders/fragment/world.fs", 
        GLSL_VERSION)
    );
    Texture2D texmap = LoadTexture("resources/textures/texmap.png");

    float texsize = 31 / (float)texmap.width;
    float gridsize = 32 / (float)texmap.width;
    float texscale = 3;
    int texsize_loc = GetShaderLocation(shader, "texsize");
    int gridsize_loc = GetShaderLocation(shader, "gridsize");
    int texscale_loc = GetShaderLocation(shader, "texscale");
    SetShaderValue(shader, texsize_loc, &texsize, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, gridsize_loc, &gridsize, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, texscale_loc, &texscale, SHADER_UNIFORM_FLOAT);

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

        renderer.BeginRender();
        {
            DrawFPS(0, 0);

            BeginMode3D(camera); 
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
    }

    renderer.Close();
}