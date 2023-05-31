#pragma once

#include <raylib.h>
#include <raymath.h>
#include <math.h>

#include <iostream>

#define DEBUG false

using namespace std;

class Player {
    private:
    float deltat;

    public:
    float gravity = 15.5f;
    float friction = 10;
    float jump = 5;
    float speed = 1;
    float sensitivity = 50;

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

        input_axis.y = Lerp(input_axis.y, IsKeyDown(KEY_W) - IsKeyDown(KEY_S), deltat * 8);
        input_axis.x = Lerp(input_axis.x, IsKeyDown(KEY_A) - IsKeyDown(KEY_D), deltat * 8);

        // Controls
        velocity.x += speed * sinf(rotation.x) * input_axis.y;
        velocity.z += speed * cosf(rotation.x) * input_axis.y;

        velocity.x += speed * sinf(rotation.x + PI/2.0) * input_axis.x;
        velocity.z += speed * cosf(rotation.x + PI/2.0) * input_axis.x;
        if(IsKeyDown(KEY_LEFT_SHIFT)) {
            velocity.y = -jump;
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

        gun_height = Lerp(
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
            Lerp(gun_rotation.x, rotation.x - (PI/2), 0.5), 
            Lerp(gun_rotation.y, rotation.y -input_axis.y / 20, 0.5)
        };

        rotation.x -= GetMouseDelta().x * sensitivity / 10000;
        rotation.y -= GetMouseDelta().y * sensitivity / 10000;
        
        if(rotation.y > PI/2.1)
            rotation.y = PI/2.1;
        if(rotation.y < -PI/2.1)
            rotation.y = -PI/2.1;

        last_pos = position;

        velocity.x /= 1 + friction;
        velocity.z /= 1 + friction;

        position.x += velocity.x * deltat;
        position.y += velocity.y * deltat;
        position.z += velocity.z * deltat;

        velocity.y -= gravity * deltat;
        
        friction = grounded ? 0.15 : 0.01;
        speed = grounded ? 0.7 : 0.04;

        if(position.y < -10)
            position = {0, 20, 0};

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

    void OnCollide(Vector3 point) {
        if(DEBUG) return;

        // The center of the point (x and z only)
        Vector2 center = {
            point.x,
            point.z
        };
   
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

    void CheckCollision(Model model) {
        grounded = false;

        Ray collisionRay;
        RayCollision rayCollision;

        // Add extra room for error when moving fast
        float boundry = velocity.y < -10 ? -velocity.y * deltat / 2 : 0;
        
        for(float angle = 0; angle < PI * 2; angle += PI / 8) {
            // Check for walls
            for(float height = 0.15f; height < 1.3f; height += 0.1f) {
                // Get the direction of movement
                collisionRay = {
                    {
                        position.x,
                        position.y + height,
                        position.z
                    },
                    {
                        cosf(angle),
                        0,
                        sinf(angle)
                    }
                };

                rayCollision = GetRayCollisionMesh(collisionRay, model.meshes[0], model.transform);
                if(rayCollision.distance <= 0.3f && rayCollision.hit) {
                    OnCollide(rayCollision.point);
                    break;
                }
            }

            // Check for ground
            collisionRay = {
                {
                    position.x + sinf(angle) * 0.15f,
                    position.y,
                    position.z + cosf(angle) * 0.15f
                },
                {
                    0,
                    -1,
                    0
                }
            };

            rayCollision = GetRayCollisionMesh(collisionRay, model.meshes[0], model.transform);
            if(rayCollision.distance <= 0.3 + boundry && rayCollision.hit) {
                grounded = true;

                // Move up
                if(rayCollision.distance < 0.29 + boundry)
                    position.y = Lerp(position.y, rayCollision.point.y + 0.3f, deltat * 5);
            }
        }

        // Check center of player if not grounded
        if(!grounded) {
            collisionRay = {
                {position.x, position.y + 0.5f, position.z},
                {0, -1, 0}
            };
            rayCollision = GetRayCollisionMesh(collisionRay, model.meshes[0], model.transform);

            if(rayCollision.distance <= 0.3 + boundry && rayCollision.hit) {
                grounded = true;

                // Move up
                if(rayCollision.distance < 0.29 + boundry)
                    position.y = Lerp(position.y, rayCollision.point.y + 0.3f, deltat * 5);
            }
        }

        // Check above player if jumping
        if(velocity.y > 0) {
            collisionRay = {
                camera.position,
                {0, 1, 0}
            };
            rayCollision = GetRayCollisionMesh(collisionRay, model.meshes[0], model.transform);
            if(rayCollision.distance <= 0.1 && rayCollision.hit)
                velocity.y = 0;
        }
    }
};