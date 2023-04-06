#include <raylib.h>

#include <vector>
#include <iostream>

// How accurate wall collision should be
float COLLISION_DEPTH = 200;

Vector3 Vec3FromString(std::string str) {
    // Split each number by ','
    int count = 0;
    const char ** values = TextSplit(str.c_str(), ',', &count);

    // Don't continue if the string is invalid
    if(count != 3)
        return {0, 0, 0};
    
    // Return the vector
    return {
        // Convert values to floats
        std::stof(values[0]),
        std::stof(values[1]),
        std::stof(values[2])
    };
}

// Get position excluding vertical component
inline Vector2 ExcludeY(Vector3 position) {
    return {position.x, position.z};
}

float Distance(Vector3 a, Vector3 b) {
    // √ (x2−x1)^2 + (y2−y1)^2 + (z2−z1)^2
    return sqrtf((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y) + (a.z-b.z)*(a.z-b.z));
}

float Distance(Vector2 a, Vector2 b) {
    // √ (x2−x1)^2 + (y2−y1)^2
    return sqrtf((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y));
}

struct BoundingWall {
    Vector3 min, max;
};

// A generic struct to encompass both bounding boxes and bounding walls
struct GenericBounds {
    Vector3 min, max;
    char type;
};

inline Vector3 GetCenter(GenericBounds bounds) {
    return {(bounds.min.x + bounds.max.x) / 2.0f, (bounds.min.y + bounds.max.y) / 2.0f, (bounds.min.z + bounds.max.z) / 2.0f};
}
inline Vector3 GetCenter(BoundingBox bounds) {
    return {(bounds.min.x + bounds.max.x) / 2.0f, (bounds.min.y + bounds.max.y) / 2.0f, (bounds.min.z + bounds.max.z) / 2.0f};
}
inline Vector3 GetCenter(BoundingWall bounds) {
    return {(bounds.min.x + bounds.max.x) / 2.0f, (bounds.min.y + bounds.max.y) / 2.0f, (bounds.min.z + bounds.max.z) / 2.0f};
}

void DrawBoundingWall(BoundingWall wall, Color color) {
    DrawLine3D(
        wall.min,
        {wall.max.x, wall.min.y, wall.max.z},
        color
    );
    DrawLine3D(
        {wall.max.x, wall.min.y, wall.max.z},
        wall.max,
        color
    );
    DrawLine3D(
        wall.max,
        {wall.min.x, wall.max.y, wall.min.z},
        color
    );
    DrawLine3D(
        {wall.min.x, wall.max.y, wall.min.z},
        wall.min,
        color
    );
}

void DrawGenericBounds(GenericBounds bounds, Color color) {
    if(bounds.type == 0) {
        DrawBoundingBox({bounds.min, bounds.max}, color);
        return;
    }
    DrawBoundingWall({bounds.min, bounds.max}, color);
}

bool CheckWallCollision(BoundingBox a, BoundingWall b) {
	if(b.min.z == b.max.z || b.min.x == b.max.x)
		return CheckCollisionBoxes(a, {b.min, b.max});

	float dz = ((b.max.z - b.min.z)/(b.max.x - b.min.x));
	for(float d = 0; d < 1; d+=1/COLLISION_DEPTH) {
		float x = d*(b.max.x-b.min.x);

		BoundingBox slice = {
			{b.min.x + x, b.min.y, b.min.z + x*dz}, 
			{b.min.x + x, b.max.y, b.min.z + x*dz}
		};
		
		if(CheckCollisionBoxes(a, slice))
			return true;
	}
	return false;
}

BoundingBox GetWallCollide(BoundingBox a, BoundingWall b) {
	if(b.min.z == b.max.z || b.min.x == b.max.x) {
		if(CheckCollisionBoxes(a, {b.min, b.max}))
            return a;
    }

	float dz = ((b.max.z - b.min.z)/(b.max.x - b.min.x));
	for(float d = 0; d < 1; d+=1/COLLISION_DEPTH) {
		float x = d*(b.max.x-b.min.x);

		BoundingBox slice = {
			{b.min.x + x, b.min.y, b.min.z + x*dz}, 
			{b.min.x + x, b.max.y, b.min.z + x*dz}
		};
		
		if(CheckCollisionBoxes(a, slice))
			return slice;
	}
	return {0};
}

bool CheckCollisionBounds(BoundingBox a, GenericBounds b) {
    if(b.type == 0)
        return CheckCollisionBoxes(a, {b.min, b.max});
    return CheckWallCollision(
        a,
        {b.min, b.max}
    );
}

// Ensures that the values of "max" in the bounding box is bigger than "min"
GenericBounds OrderBounds(GenericBounds b) {
    int min; // The smaller of the two measurements
    if(b.min.y > b.max.y) {
        // Swap the y values
        min = b.max.y;
        b.max.y = b.min.y;
        b.min.y = min;
    }

    if(b.type == 1) return b;

    if(b.min.x > b.max.x) {
        // Swap the x values
        min = b.max.x;
        b.max.x = b.min.x;
        b.min.x = min;
    }
    
    if(b.min.z > b.max.z) {
        // Swap the z values
        min = b.max.z;
        b.max.z = b.min.z;
        b.min.z = min;
    }
    return b;
}

BoundingBox OrderBounds(BoundingBox box) {
    GenericBounds generic = OrderBounds((GenericBounds){box.min, box.max, 0});
    return {
        generic.min,
        generic.max
    };
}

class CollisionMap {
    public:
    std::vector<GenericBounds> bounds;

    void AddBounds(GenericBounds b) {
        // Ensure the box is ordered before appending
        bounds.push_back(OrderBounds(b));
    }

    // Null constructor
    CollisionMap(){}

    // Construct from file
    CollisionMap(const char * filename) {
        /* The layout for a collision map file follows the following rules:
            -  First char indicates bounding type
            -  1 bounding box per line
            -  Line follows the format:  x,y,z:x,y,z
            -  Whitespace is NOT allowed
        */

        // Raw text file data
        char * content = LoadFileText(filename);
        
        // Split the text file by the \n charcode (meaning 'newline')
        int line_count = 0;
        const char ** lines = TextSplit(content, '\n', &line_count);
        
        // The TextSplit() method has a buffer that is overwrittin each time its used,
        //   for this reason the values got from TextSplit() need to be copied to avoid memory overwrites
        std::string lines_str[line_count];

        // Copy all the lines into a string array
        for(int i = 0 ;i < line_count; ++i) {
            // Assigning to a string is more memory safe than using strcpy() and allows more string modification
            lines_str[i] = lines[i]; 
        }

        for(int i = 0; i < line_count; ++i) {
            char type = 0;
            if(lines_str[i].at(0) == '/')
                type = 1; // BoundingWall
            
            // Trim the first char
            lines_str[i] = lines_str[i].substr(1, lines_str[i].length() - 1);
            
            // Split the line by ':'
            int count = 0;
            const char ** pair = TextSplit(lines_str[i].c_str(), ':', &count);

            // Skip this iteration if the line is incorrect
            if(count != 2) 
                continue;
            
            // Again, copy the values to strings to avoid memory overwrites
            std::string min = pair[0];
            std::string max = pair[1];

            // Construct the box
            GenericBounds bounds = {
                Vec3FromString(min),
                Vec3FromString(max),
                type
            };

            // Finally, add the box
            AddBounds(bounds);
        }
    }
};