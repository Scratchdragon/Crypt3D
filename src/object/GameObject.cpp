#pragma once

#include <raylib.h>
#include <iostream>

#include "render/Renderer.cpp"

// Collision flags
// ---------------
//  Do not respond to collisions
#define NO_COLLISION        0
// Only respond to player collisions
#define PLAYER_COLLISION    1
// Respond to collision with an equal collision level
#define PARTNER_COLLISION   2
// Respond to all collisions (increases lag)
#define GLOBAL_COLLISION    3

using namespace std;

class GameObject {
    public:
    // Flags
    const int collision_level = NO_COLLISION;
    
    // The name is per object while the type is per subclass
    string name;
    string type = "GameObject";
    
    // ID is unique per object instance
    unsigned int id;

    // When setting the objects bounds, use "local_bounds" since "bounds" is calculated by the object manager
    BoundingBox bounds, local_bounds;

    // The current position of the object
    Vector3 position;

    // Rotation is not used by the object manager and is purely for the object to utilize
    Vector3 rotation;

    // OnStart runs once on the object initilisation
    void OnStart();

    // OnDelete runs once on the object deletion
    void OnDelete();

    // Update runs once per frame for object logic
    void Update(float deltat);

    // Render runs once per frame to draw the object
    void Render(float deltat, Renderer * renderer);

    // On collide is triggered when two GameObjects overlap their collision boxes
    void OnCollide(GameObject * object);

    // On collide (player) is triggered if the GameObject collides with the player
    void OnCollide(Player * object);

    GameObject(){}
};