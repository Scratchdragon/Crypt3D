#ifndef Renderer

#include <raylib.h>

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
    
    // Main constructor
    Renderer(Vector2 resolution, const char * title, Color bg, unsigned int config) {
        SetConfigFlags(config);
        InitWindow(resolution.x, resolution.y, title);

        this->width = resolution.x;
        this->height = resolution.y;
        
        render = LoadRenderTexture(width, height);

        this->veiwport = {0, 0, resolution.x, resolution.y};

        this->background = bg;
    }
    // Null constructor
    Renderer() {}
    
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
        UnloadRenderTexture(render);
        CloseWindow();
    }
};

#endif