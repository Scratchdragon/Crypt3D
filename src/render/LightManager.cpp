// Simple lighting system
#include <raylib.h>

struct Light {
    char index;

    float active = 0;
    Vector3 position;
    float brightness;
};

class LightManager {
    public:
    int light_count = 0;
    Light lights[255];
    Renderer * renderer;

    LightManager(Renderer * renderer) {
        this->renderer = renderer;
    }
    LightManager(){}

    void UpdateLight(int index) {
        // Get the light from the buffer
        Light light = lights[index];

        // Pass all the light data to the shader
        renderer->SetAllShaderVal(
            TextFormat("lights[%i].active", light.index),
            &light.active,
            SHADER_UNIFORM_FLOAT
        );
        renderer->SetAllShaderVal(
            TextFormat("lights[%i].brightness", light.index),
            &light.brightness,
            SHADER_UNIFORM_FLOAT
        );
        renderer->SetAllShaderVal(
            TextFormat("lights[%i].position", light.index),
            &light.position,
            SHADER_UNIFORM_VEC3
        );
    }

    void CreateLight(float brightness, Vector3 position) {
        // Define the light
        Light light;
        light.active = 1;
        light.brightness = brightness;
        light.position = position;
        light.index = light_count;

        // Assign the light and inform the shaders
        lights[light_count] = light;
        ++light_count;
        renderer->SetAllShaderVal("lightc", &light_count, SHADER_UNIFORM_INT);

        // Initial update
        UpdateLight(light_count - 1);
    }

    void Update() {
        for(int i = 0;i<light_count;++i) {
            UpdateLight(i);
        }
    }

    void Reset() {
        for(int i = 0;i<light_count;++i) {
            lights[i].active = 0;
            UpdateLight(i);
        }
        light_count = 0;
    }
};