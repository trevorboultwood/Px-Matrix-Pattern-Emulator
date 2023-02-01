#include <SDL2/SDL.h>
#include <stdio.h>
#include <effects.h>
#include <FastLED.h>
#define CIRCLE_R 4
#define __linux__

void initPatterns(void)
{
      noisesmoothing = 200;

  // just any free input pin
  // random16_add_entropy(analogRead(18));

  // fill coordinates with random values
  // set zoom levels
  noise_x = random16();
  noise_y = random16();
  noise_z = random16();
  noise_scale_x = 6000;
  noise_scale_y = 6000;

  // for the random movement
  dx = random8();
  dy = random8();
  dz = random8();
  dsx = random8();
  dsy = random8();
}

void drawPattern(SDL_Renderer* renderer)
{
    for (int x = 0; x < 64; x++)
    {
        for (int y = 0; y < 64; y++)
        {
            int16_t v = 0;
            uint8_t wibble = sin8(time_counter);
            v += sin16(x * wibble * 3 + time_counter);
            v += cos16(y * (128 - wibble) + time_counter);
            v += sin16(y * x * cos8(-time_counter) / 8);
            CRGB color = ColorFromCurrentPalette((v >> 8) + 127);
            savePixel(x, y, color);
            // effects.drawBackgroundFastLEDPixelCRGB(x, y, (v >> 8) + 127); working but boring
            // efect.pixel not working?!
        }
    }
    ShowFrame(renderer);
    time_counter++;
}

void drawPattern2(SDL_Renderer* renderer)
{
            // 4 corners of colour. Looks sick.
        noise_y += dy;
        noise_x += dx;
        noise_z += dz;

        FillNoise();
        ShowNoiseLayer2(0, 2, 1);

        //Caleidoscope3();
        //Caleidoscope1();
        Caleidoscope6();
        Caleidoscope6();
        ShowFrame(renderer);

}

int main(int argc, char **argv)
{
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        printf("Error: SDL failed to initialize\nSDL Error: '%s'\n", SDL_GetError());
        return 1;
    }

    SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // setbrush

    SDL_RenderDrawPoint(renderer, 640 / 2, 480 / 2); // draw a point
    // DrawFilledCircle(renderer, 320+i,240,50);

    //DrawCircleArray(renderer);
    initPatterns(); //init some variables used in in some of the patterns.
    bool running = true;




        while(running){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    running = false;
                    break;

                default:
                    break;
            }



                    //default loop

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        //drawPattern(renderer);
        drawPattern2(renderer);
        SDL_RenderPresent(renderer);
        }

    }
    SDL_DestroyWindow(window);
    SDL_Quit();



    return 0;
}