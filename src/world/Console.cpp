#include <raylib.h>

#include <iostream>
#include <vector>
#include <functional>

#include "world/World.cpp"

using namespace std;

struct Command {
    string command;
    function<int (vector<string>)> exec;
};

// The console is used to control most larger game events like level switching, like the quake console.
namespace Console {
    World * world;

    vector<string> history;
    string input = "";
    bool open = false;
    
    void Out(string msg) {
        cout << msg << "\n";
        history.push_back(msg);
    }
    
    const Command commands[] = {
        {"clear", [](vector<string> args){
            history.clear();
            Out("[Console Cleared]");
            return 0;
        }},
        {"maps", [](vector<string> args){
            FilePathList directory = LoadDirectoryFiles("resources/world");
            Out("Maps in 'resources/world/'");
            for(int i = 0; i < directory.count; ++i)
                Out("  " + string(directory.paths[i]));
            return 0;
        }},
        {"map", [](vector<string> args){
            if(args.size() == 0) {
                Out("Expected at least 1 argument");
                return 1;
            }

            world->Reset();
            bool loaded = world->Load(args[0].c_str());
            if(loaded) {
                Out("Loaded map '" + args[0] + "'");
                return 0;
            }
            else {
                Out("Map '" + args[0] + "' doe's not exist.");
                return 1;
            }
        }},
        {"help", [](vector<string> args){
            Out("Console is used to control all major game events");
            return 0;
        }}
    };

    bool Exec(string line) {
        Out("> " + line);
        int count = 0;
        const char ** tokens = TextSplit(line.c_str(), ' ', &count);
        string comm = tokens[0];
        vector<string> args;
        if(count != 1) {
            for(int i = 1; i < count; ++i)
                args.push_back(tokens[i]);
        }

        for(int i = 0; true; ++i) {
            if(commands[i].command == comm) {
                commands[i].exec(args);
                return true;
            }
            
            if(commands[i].command == "help")
                break;
        }
        Out("Command '" + comm + "' not found");
        return false;
    }

    void Render(Vector2 window) {
        DrawRectangle(10, 10, window.x - 20, window.y / 1.93f, (Color){30, 30, 30, 200});
        for(int i = 0; i < history.size(); ++i) {
            int line = history.size() - i - 1;
            DrawText(history[line].c_str(), 20, window.y / 2.0f - (i + 1) * (window.y / 40), window.y / 50, GRAY);
        }

        DrawRectangle(15, window.y / 2.0f, window.x - 30, window.y / 50, (Color){0, 0, 0, 100});
        DrawText(input.c_str(), 20, window.y / 2.0f, window.y / 50, WHITE);
    }

    void AddInput(int keycode) {
        if(keycode == KEY_ENTER) {
            Exec(input);
            input = "";
            return;
        }
        if(keycode == KEY_BACKSPACE) {
            input = input.substr(0, input.size()-1);
            return;
        }
        input = TextToLower((input + (char)keycode).c_str());
    }
};