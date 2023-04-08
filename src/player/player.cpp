#include <raylib.h>
#include <math.h>

#include <iostream>

#define DEBUG false

using namespace std;

float lerp(float a, float b, float f) {
    return a * (1.0 - f) + (b * f);
}

class Player {
    private:
    float deltat;

    public:
    float gravity = 15.5f;
    float friction = 10;
    float jump = 5;
    float speed = 1;

    bool grounded;

    Vector3 position, last_pos;
    Vector3 velocity = {0, 0, 0};
    Vector2 input_axis = {0, 0};
    float net_velocity; // Total direction velocity
    Camera3D camera;
    Vector2 rotation = {0, 0};

    Vector3 look;

    Vector2 gun_rotation;
    Vector3 gun_position;
    float gun_height = 0.2;

    BoundingBox feet;
    BoundingBox bounds;

    Player(Vector3 position) {
        this->position = position;
        this->camera = {0};
        this->camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
        this->camera.fovy = 60.0f;
        this->camera.projection = CAMERA_PERSPECTIVE;

        if(DEBUG) gravity = 0;
    }
    Player(){}

    void Update() {
        deltat = GetFrameTime();

        if(IsKeyDown(KEY_SPACE) && grounded)
            velocity.y = jump;
        else if(grounded)
            velocity.y = 0;

        input_axis.y = lerp(input_axis.y, IsKeyDown(KEY_W) - IsKeyDown(KEY_S), deltat * 8);
        input_axis.x = lerp(input_axis.x, IsKeyDown(KEY_A) - IsKeyDown(KEY_D), deltat * 8);

        // Controls
        velocity.x += speed * sinf(rotation.x) * input_axis.y;
        velocity.z += speed * cosf(rotation.x) * input_axis.y;

        velocity.x += speed * sinf(rotation.x + PI/2.0) * input_axis.x;
        velocity.z += speed * cosf(rotation.x + PI/2.0) * input_axis.x;
        if(IsKeyDown(KEY_LEFT_SHIFT)) {
            velocity.y -= speed;
        }

        // Calculate net horizontal velocity
        net_velocity = sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z));

        this->look = {
            sinf(rotation.x) * cos(rotation.y), 
            sin(rotation.y), 
            cosf(rotation.x) * cos(rotation.y)
        };

        this->camera.position = {position.x, position.y + 1, position.z};
        this->camera.target = {
            position.x + look.x, 
            position.y + 1 + look.y, 
            position.z + look.z
        };

        gun_height = lerp(
            gun_height,
            (sin(rotation.x) * sin(position.x * 2) + cos(rotation.x) * sin(position.z * 2))
            * Clamp(net_velocity, 0, 1) / 100 + 0.2f + Clamp(velocity.y / 200, -0.03, 0.1),
            0.5
        );

        gun_position = {
            position.x + look.x / 2.2f + sin(rotation.y) * gun_height * sin(rotation.x),
            position.y + look.y / 2.2f + 1 - cos(rotation.y) * gun_height,
            position.z + look.z / 2.2f + sin(rotation.y) * gun_height * cos(rotation.x)
        };

        gun_rotation = {
            lerp(gun_rotation.x, rotation.x - (PI/2), 0.5), 
            lerp(gun_rotation.y, rotation.y -input_axis.y / 20, 0.5)
        };

        rotation.x -= GetMouseDelta().x / 100;
        rotation.y -= GetMouseDelta().y / 100;
        
        if(rotation.y > PI/2.1)
            rotation.y = PI/2.1;
        if(rotation.y < -PI/2.1)
            rotation.y = -PI/2.1;

        if(DEBUG) {
            velocity.y /= 1 + friction;
            if(IsKeyDown(KEY_SPACE))
                velocity.y += speed;
        };

        last_pos = position;

        velocity.x /= 1 + friction;
        velocity.z /= 1 + friction;

        position.x += velocity.x * deltat;
        position.y += velocity.y * deltat;
        position.z += velocity.z * deltat;

        velocity.y -= gravity * deltat;
        
        friction = grounded ? 0.15 : 0.01;
        speed = grounded ? 0.7 : 0.04;

        if(position.y < -7)
            position = {0, 5, 0};

        feet = {
            {position.x - 0.15f, position.y - 0.1f, position.z - 0.15f},
            {position.x + 0.15f, position.y, position.z + 0.15f}
        };

        bounds = {
            {position.x - 0.2f, position.y, position.z - 0.2f},
            {position.x + 0.2f, position.y + 1.2f, position.z + 0.2f}
        };
    }

    void OnCollide(BoundingBox box) {
        if(DEBUG) return;

        // The center of the box (x and z only)
        Vector2 center = {(box.min.x + box.max.x) / 2.0f, (box.min.z + box.max.z) / 2.0f};
   
        // Calculate the direction away from the wall and the net velocity
        float angle = atan2f(-center.y + position.z, -center.x + position.x); // tan-1(rise/run) == angle
        net_velocity = sqrtf((velocity.x * velocity.x) + (velocity.z * velocity.z));

        // Move in the calculated direction
        position.x += cosf(angle) * net_velocity * deltat;
        position.z += sinf(angle) * net_velocity * deltat;

        velocity = {
            (cosf(angle) * net_velocity + velocity.x) / 2.0f,
            velocity.y,
            (sinf(angle) * net_velocity + velocity.z) / 2.0f
        };
    }
};