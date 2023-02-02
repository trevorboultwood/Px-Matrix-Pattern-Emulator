#pragma once
#ifndef DisplayRenderer_h 
#define DisplayRenderer_h

#include<SDL.h>
#include<stdio.h>
#include"effects.h"

class DisplayRenderer
{
    public:
        DisplayRenderer(/* args */);
        ~DisplayRenderer();
        void init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen);
        void render();
        void handleEvent();
        void clean();
        bool running();
        
    private:
        Effects *effects;
        SDL_Window *window;
        SDL_Renderer *renderer;
        bool isRunning;

};



#endif // Display_h