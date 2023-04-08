#include <raylib.h>
#include <iostream>
#include <map>

using namespace std;

class RShader {
    public:
    Shader shader;
    map<string, int> locs;

    int GetLoc(const char * key) {
        // Create key if it doesnt exist yet
        if(locs.find(key) == locs.end()) {
            locs.insert({key, GetShaderLocation(shader, key)});
            cout << "ShaderLoc [Cache] '" << key << "'=" << locs.at(key) << endl;
        }

        return locs.at(key);
    }

    // Operator [] can also be used to get shader locs
    int operator [](const char * key) {
        return GetLoc(key);
    }

    // When the class is called like RShader(...) it will be treated as a shader value set
    void operator()(const char * key, const void * value, int uniform) {
        SetShaderValue(shader, GetLoc(key), value, uniform);
    }

    // Used to ensure the raylib shader values are still set
    void SetInbuiltLoc(int loc, const char * key) {
        shader.locs[loc] = GetShaderLocation(shader, key);
    }

    // Main constructor
    RShader(const char * vertex, const char * frag) {
        shader = LoadShader(
            TextFormat(vertex, GLSL_VERSION),
            TextFormat(frag, GLSL_VERSION)
        );
    }
    // Null constructor
    RShader() {}
};

class Renderer {
    public:
    // The dimensions of the renderer
    Vector2 resolution;

    // The size of the window
    int width, height;

    // The renderer dimensions as a rectange
    Rectangle veiwport;

    // The main render texture
    RenderTexture2D render;

    // The background color
    Color background;

    // All of the shaders
    RShader * shaders;
    char shader_count;
    
    // Main constructor
    Renderer(Vector2 resolution, const char * title, Color bg, char shader_count, unsigned int config) {
        SetConfigFlags(config);
        InitWindow(resolution.x, resolution.y, title);

        DisableCursor();

        this->width = resolution.x;
        this->height = resolution.y;
        
        render = LoadRenderTexture(width, height);

        this->veiwport = {0, 0, resolution.x, resolution.y};

        this->background = bg;

        this->shaders = new RShader[shader_count];
        this->shader_count = shader_count;
    }
    // Null constructor
    Renderer() {}

    void InitShader(char index, const char * vertex, const char * frag) {
        shaders[index] = RShader(vertex, frag);
    }

    void SetAllShaderVal(const char * key, void * value, int uniform) {
        for(int i = 0;i<shader_count;++i) {
            shaders[i](key, value, uniform);
        }
    }
    
    void ChangeResolution(Vector2 resolution) {
        this->veiwport = {0, 0, resolution.x, resolution.y};
        this->resolution = resolution;
    }

    void BeginRender() {
        // Update the window dimensions
        if(IsWindowResized()) {
            this->width = GetRenderWidth();
            this->height = GetRenderHeight();
        }
        
        BeginTextureMode(render);
        ClearBackground(background);
    }

    void StopRender() {
        EndTextureMode();

        BeginDrawing();

        DrawTexturePro(
            render.texture,
            {
                veiwport.x,
                veiwport.y,
                veiwport.width,
                -veiwport.height
            },
            {
                0,
                0,
                (float)width,
                (float)height
            },
            {0, 0},
            0,
            WHITE
        );

        EndDrawing();
    }

    void Close() {
        EnableCursor();
        UnloadRenderTexture(render);
        CloseWindow();
    }
};