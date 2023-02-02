#include<DisplayRenderer.h>
#define CIRCLE_R 4
#define __linux__

DisplayRenderer *myDisplay;


int main(int argc, char **argv)
{
    myDisplay = new DisplayRenderer();


    myDisplay->init("PxMatrix Effects Emulator",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,SCREEN_WIDTH,SCREEN_HEIGHT,false);

    while (myDisplay->running())
    {
       myDisplay->render();
       myDisplay->handleEvent();
    }

    myDisplay->clean();
    return 0;
}