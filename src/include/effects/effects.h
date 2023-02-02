#pragma once

#ifndef Effects_h
#define Effects_h

#include <stdint.h>
#include <FastLED.h>
#include <SDL.h>
#define CIRCLE_R 4

#define SCREEN_WIDTH (2 * CIRCLE_R + 2) * 64 + CIRCLE_R
#define SCREEN_HEIGHT (2 * CIRCLE_R + 2) * 64 + CIRCLE_R
#define MATRIX_WIDTH 64
#define MATRIX_HEIGHT 64




class Effects{
  public:
    CRGBPalette16 palettes[5] = {HeatColors_p, LavaColors_p, RainbowColors_p, RainbowStripeColors_p, CloudColors_p};
    uint16_t NUM_LEDS = (MATRIX_WIDTH * MATRIX_HEIGHT) + 1; // one led spare to capture out of bounds
    CRGB *leds;//[NUM_LEDS];
    uint32_t noise_x, noise_y, noise_z;
    uint32_t noise_scale_x, noise_scale_y;
    int16_t dx, dy, dz, dsx, dsy;

    uint8_t noisesmoothing;

    const int MATRIX_CENTER_X = MATRIX_WIDTH / 2;
    const int MATRIX_CENTER_Y = MATRIX_HEIGHT / 2;
    const byte MATRIX_CENTRE_X = MATRIX_CENTER_X - 1;
    const byte MATRIX_CENTRE_Y = MATRIX_CENTER_Y - 1;


    CRGBPalette16 currentPalette = palettes[2];
    uint8_t noise[MATRIX_WIDTH][MATRIX_HEIGHT];
    CRGB ColorFromCurrentPalette(uint8_t index = 0, uint8_t brightness = 255, TBlendType blendType = LINEARBLEND)
    {
      return ColorFromPalette(currentPalette, index, brightness, blendType);
    }
    void DrawCircle(SDL_Renderer *renderer, int32_t centreX, int32_t centreY, int32_t radius)
    {
      const int32_t diameter = (radius * 2);

      int32_t x = (radius - 1);
      int32_t y = 0;
      int32_t tx = 1;
      int32_t ty = 1;
      int32_t error = (tx - diameter);
      // SDL_Renderco

      while (x >= y)
      {
        //  Each of the following renders an octant of the circle
        SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
        SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
        SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
        SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
        SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);
        SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
        SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
        SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);

        if (error <= 0)
        {
          ++y;
          error += ty;
          ty += 2;
        }

        if (error > 0)
        {
          --x;
          tx += 2;
          error += (tx - diameter);
        }
      }
    }
    void DrawFilledCircle(SDL_Renderer *renderer, int x0, int y0, int radius)
    {
      int x = radius;
      int y = 0;
      int xChange = 1 - (radius << 1);
      int yChange = 0;
      int radiusError = 0;

      while (x >= y)
      {
        for (int i = x0 - x; i <= x0 + x; i++)
        {
          SDL_RenderDrawPoint(renderer, i, y0 + y);
          SDL_RenderDrawPoint(renderer, i, y0 - y);
        }
        for (int i = x0 - y; i <= x0 + y; i++)
        {
          SDL_RenderDrawPoint(renderer, i, y0 + x);
          SDL_RenderDrawPoint(renderer, i, y0 - x);
        }

        y++;
        radiusError += yChange;
        yChange += 2;
        if (((radiusError << 1) + xChange) > 0)
        {
          x--;
          radiusError += xChange;
          xChange += 2;
        }
      }
    }
    void DrawCircleArray(SDL_Renderer *renderer)
    {
      for (int x = 0; x < 64; x++)
        for (int y = 0; y < 64; y++)
        {
          DrawFilledCircle(renderer, CIRCLE_R + x * 10 + (x * 2), CIRCLE_R + y * 10 + (y * 2), CIRCLE_R);
        }
    }
    uint16_t XY(uint8_t x, uint8_t y)
    {
      if (x >= MATRIX_WIDTH || x < 0)
        return 0;
      if (y >= MATRIX_HEIGHT || y < 0)
        return 0;

      return (y * MATRIX_WIDTH) + x + 1; // everything offset by one to capute out of bounds stuff - never displayed by ShowFrame()
    }
    void ShowFrame(SDL_Renderer *renderer)
    {
      for (int x = 0; x < 64; x++)
        for (int y = 0; y < 64; y++)
        {
          CRGB tmp_led = leds[XY(x, y)];
          SDL_SetRenderDrawColor(renderer, tmp_led.r, tmp_led.g, tmp_led.b, 255); // setbrush based on led value
          DrawFilledCircle(renderer, CIRCLE_R + x * 10 + (x * 2), CIRCLE_R + y * 10 + (y * 2), CIRCLE_R);
        }
    }
    uint16_t time_counter = 0;

    void savePixel(int16_t x, int16_t y, CRGB color)
    {

      if (x < 0 || x > 63 || y < 0 || y > 63)
      {
        ; // safety
      }
      else
      {
        // if (!currentBuffer.isPixel(x, y)) // box for time
        //{
        leds[XY(x, y)] = color; //; //do nothing
        //}
      }
    }

    void FillNoise()
    {
      for (uint8_t i = 0; i < MATRIX_WIDTH; i++)
      {
        uint32_t ioffset = noise_scale_x * (i - MATRIX_CENTRE_Y);

        for (uint8_t j = 0; j < MATRIX_HEIGHT; j++)
        {
          uint32_t joffset = noise_scale_y * (j - MATRIX_CENTRE_Y);

          byte data = inoise16(noise_x + ioffset, noise_y + joffset, noise_z) >> 8;

          uint8_t olddata = noise[i][j];
          uint8_t newdata = scale8(olddata, noisesmoothing) + scale8(data, 256 - noisesmoothing);
          data = newdata;

          noise[i][j] = data;
        }
      }
    }

    void ShowNoiseLayer2(byte layer, byte colorrepeat, byte colorshift)
    {
      for (uint8_t i = 0; i < MATRIX_WIDTH; i++)
      {
        for (uint8_t j = 0; j < MATRIX_HEIGHT; j++)
        {

          uint8_t color = noise[i][j];

          // assign a color depending on the actual palette
          CRGB pixel = ColorFromCurrentPalette(colorrepeat * (color + colorshift));

          leds[XY(i, j)] = pixel;
        }
      }
    }

    void Caleidoscope1()
    {
      for (int x = 0; x < MATRIX_CENTER_X; x++)
      {
        for (int y = 0; y < MATRIX_CENTER_Y; y++)
        {
          leds[XY(MATRIX_WIDTH - 1 - x, y)] = leds[XY(x, y)];
          leds[XY(MATRIX_WIDTH - 1 - x, MATRIX_HEIGHT - 1 - y)] = leds[XY(x, y)];
          leds[XY(x, MATRIX_HEIGHT - 1 - y)] = leds[XY(x, y)];
        }
      }
    }
    void Caleidoscope3()
    {
      for (int x = 0; x <= MATRIX_CENTRE_X; x++)
      {
        for (int y = 0; y <= x; y++)
        {
          leds[XY(x, y)] = leds[XY(y, x)];
        }
      }
    }

    void Caleidoscope6()
    {
      for (int x = 1; x < MATRIX_CENTER_X; x++)
      {
        leds[XY(7 - x, 7)] = leds[XY(x, 0)];
      } // a
      for (int x = 2; x < MATRIX_CENTER_X; x++)
      {
        leds[XY(7 - x, 6)] = leds[XY(x, 1)];
      } // b
      for (int x = 3; x < MATRIX_CENTER_X; x++)
      {
        leds[XY(7 - x, 5)] = leds[XY(x, 2)];
      } // c
      for (int x = 4; x < MATRIX_CENTER_X; x++)
      {
        leds[XY(7 - x, 4)] = leds[XY(x, 3)];
      } // d
      for (int x = 5; x < MATRIX_CENTER_X; x++)
      {
        leds[XY(7 - x, 3)] = leds[XY(x, 4)];
      } // e
      for (int x = 6; x < MATRIX_CENTER_X; x++)
      {
        leds[XY(7 - x, 2)] = leds[XY(x, 5)];
      } // f
      for (int x = 7; x < MATRIX_CENTER_X; x++)
      {
        leds[XY(7 - x, 1)] = leds[XY(x, 6)];
      } // g
    }

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

    void drawPattern(SDL_Renderer *renderer)
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

    void drawPattern2(SDL_Renderer *renderer)
  {
    // 4 corners of colour. Looks sick.
    noise_y += dy;
    noise_x += dx;
    noise_z += dz;

    FillNoise();
    ShowNoiseLayer2(0, 2, 1);

    // Caleidoscope3();
    // Caleidoscope1();
    //Caleidoscope6();
    //Caleidoscope6();
    ShowFrame(renderer);
  }


};
#endif
