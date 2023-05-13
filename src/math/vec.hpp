#include <raylib.h>

#include <iostream>

using namespace std;

Vector3 Vec3FromString(string str) {
    int count = 0;
    const char ** tokens = TextSplit(str.c_str(), ',', &count);

    if(count != 3)
        return {0};
    
    return {
        stof(tokens[0]),
        stof(tokens[1]),
        stof(tokens[2])
    };
}