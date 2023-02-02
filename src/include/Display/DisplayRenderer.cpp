#include "DisplayRenderer.h"

DisplayRenderer::DisplayRenderer()
{
    effects = new Effects;
    effects->leds = (CRGB*) malloc(sizeof(CRGB) * effects->NUM_LEDS);
}
DisplayRenderer::~DisplayRenderer()
{
}
void DisplayRenderer::clean()
{
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();

}
void DisplayRenderer::init(const char *title, int xpos, int ypos, int width, int height, bool fullscreen)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        printf("Error: SDL failed to initialize\nSDL Error: '%s'\n", SDL_GetError());
        isRunning = false;
    }
    else
    {
        SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer);
        isRunning = true;

        SDL_SetRenderDrawColor(renderer,255,255,255,255);
        SDL_RenderClear(renderer);//????
        effects->initPatterns();
    }
}
void DisplayRenderer::render()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    effects->drawPattern2(renderer);
    SDL_RenderPresent(renderer);
}
void DisplayRenderer::handleEvent()
{
    SDL_Event event;
    SDL_PollEvent(&event);
    switch(event.type)
    {
        case SDL_QUIT:
        {
            isRunning = false;
            break;
        }
        default:
        {
            break;
        }
    }
}
bool DisplayRenderer::running()
{
    return isRunning;
}
