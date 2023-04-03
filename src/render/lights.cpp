// Simple lighting system
#include <raylib.h>

struct Light {
    char index;
    int shader_locs[3];

    float active = 0;
    Vector3 position;
    float brightness;
};

class LightManager {
    private:
    int light_count_loc;

    public:
    int light_count = 0;
    Light lights[255];
    Shader * shader;

    LightManager(Shader * shader) {
        this->shader = shader;
        this->light_count_loc = GetShaderLocation(*shader, "light_count");
    }
    LightManager(){}

    void UpdateLight(int index) {
        // Get the light from the buffer
        Light light = lights[index];

        // Pass all the light data to the shader
        SetShaderValue(
            *shader, 
            light.shader_locs[0], 
            &light.active, 
            SHADER_UNIFORM_FLOAT
        );
        SetShaderValue(
            *shader, 
            light.shader_locs[1], 
            &light.brightness, 
            SHADER_UNIFORM_FLOAT
        );
        SetShaderValue(
            *shader, 
            light.shader_locs[2], 
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

        // Get the shader locs
        light.shader_locs[0] = GetShaderLocation(*shader, TextFormat("lights[%i].active", light.index));
        light.shader_locs[1] = GetShaderLocation(*shader, TextFormat("lights[%i].brightness", light.index));
        light.shader_locs[2] = GetShaderLocation(*shader, TextFormat("lights[%i].position", light.index));

        // Assign the light and inform the shader
        lights[light_count] = light;
        ++light_count;
        SetShaderValue(*shader, light_count_loc, &light_count, SHADER_UNIFORM_INT);

        // Initial update
        UpdateLight(light_count - 1);
    }

    void Update() {
        for(int i = 0;i<light_count;++i) {
            UpdateLight(i);
        }
    }
};