#include<display.h>
#include<stdint.h>
#include<stdbool.h>
#include<math.h>
typedef uint8_t  byte;

#define MATRIX_WIDTH 64
#define MATRIX_HEIGHT 64
#define MATRIX_CENTER_X MATRIX_WIDTH/2
#define MATRIX_CENTER_Y MATRIX_HEIGHT/2
#define NUM_LEDS MATRIX_WIDTH * MATRIX_HEIGHT


CRGB currentColor;
CRGBPalette16 palettes[] = {HeatColors_p, LavaColors_p, RainbowColors_p, RainbowStripeColors_p, CloudColors_p};
CRGBPalette16 currentPalette = palettes[0];

CRGB ColorFromCurrentPalette(uint8_t index = 0, uint8_t brightness = 255, TBlendType blendType = LINEARBLEND)
{
  return ColorFromPalette(currentPalette, index, brightness, blendType);
}

struct timerSpiral
{
  unsigned long takt;
  unsigned long lastMillis;
  unsigned long count;
  int delta;
  byte up;
  byte down;
};
timerSpiral multiTimer[5];
byte theta1 = 0;
byte theta2 = 0;
byte hueoffset = 0;

uint8_t radiusx = MATRIX_WIDTH / 4;
uint8_t radiusy = MATRIX_HEIGHT / 4;
uint8_t minx = MATRIX_CENTER_X - radiusx;
uint8_t maxx = MATRIX_CENTER_X + radiusx + 1;
uint8_t miny = MATRIX_CENTER_Y - radiusy;
uint8_t maxy = MATRIX_CENTER_Y + radiusy + 1;

uint8_t spirocount = 1;
uint8_t spirooffset = 256 / spirocount;
bool spiroincrement = false;

bool handledChange = false;
int timers = sizeof(multiTimer) / sizeof(multiTimer[0]);
// Some standard colors
/*uint16_t myRED = display.color565(255, 0, 0);
uint16_t myGREEN = display.color565(0, 255, 0);
uint16_t myBLUE = display.color565(0, 0, 255);
uint16_t myWHITE = display.color565(255, 255, 255);
uint16_t myYELLOW = display.color565(255, 255, 0);
uint16_t myCYAN = display.color565(0, 255, 255);
uint16_t myMAGENTA = display.color565(255, 0, 255);
uint16_t myBLACK = display.color565(0, 0, 0);
*/
int16_t dx;
int16_t dy;
int16_t dz;
int16_t dsx;
int16_t dsy;

struct vec3d
{
  float x, y, z;
};

struct triangle
{
  vec3d p[3];
};

struct mat4x4
{
  float m[4][4];
};

const static triangle cube[12]  = {
    // SOUTH
    {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f},

    // EAST
    {1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f},
    {1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f},

    // NORTH
    {1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f},
    {1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f},

    // WEST
    {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},

    // TOP
    {0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
    {0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f},

    // BOTTOM
    {1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f},
    {1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
};

triangle tri;
mat4x4 matProj, matRotZ, matRotX;
float fTheta;

void MultiplyMatrixVector(vec3d &i, vec3d &o, mat4x4 &m)
{
  o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
  o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
  o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
  float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

  if (w != 0.0f)
  {
    o.x /= w;
    o.y /= w;
    o.z /= w;
  }
}

/* Convert x,y co-ordinate to flat array index.
 * x and y positions start from 0, so must not be >= 'real' panel width or height
 * (i.e. 64 pixels or 32 pixels.).  Max value: MATRIX_WIDTH-1 etc.
 */
uint16_t XY(uint8_t x, uint8_t y)
{
  if (x >= MATRIX_WIDTH || x < 0)
    return 0;
  if (y >= MATRIX_HEIGHT || y < 0)
    return 0;

  return (y * MATRIX_WIDTH) + x + 1; // everything offset by one to capute out of bounds stuff - never displayed by ShowFrame()
}

uint8_t beatcos8(accum88 beats_per_minute, uint8_t lowest = 0, uint8_t highest = 255, uint32_t timebase = 0, uint8_t phase_offset = 0)
{
  uint8_t beat = beat8(beats_per_minute, timebase);
  uint8_t beatcos = cos(beat + phase_offset);
  uint8_t rangewidth = highest - lowest;
  uint8_t scaledbeat = scale8(beatcos, rangewidth);
  uint8_t result = lowest + scaledbeat;
  return result;
}

uint8_t mapsin8(uint8_t theta, uint8_t lowest = 0, uint8_t highest = 255)
{
  uint8_t beatsin = sin(theta);
  uint8_t rangewidth = highest - lowest;
  uint8_t scaledbeat = scale8(beatsin, rangewidth);
  uint8_t result = lowest + scaledbeat;
  return result;
}

uint8_t mapcos8(uint8_t theta, uint8_t lowest = 0, uint8_t highest = 255)
{
  uint8_t beatcos = cos(theta);
  uint8_t rangewidth = highest - lowest;
  uint8_t scaledbeat = scale8(beatcos, rangewidth);
  uint8_t result = lowest + scaledbeat;
  return result;
}

// Constants for out of bound region (for clck/time to not get erazed)
int boundx1 = 1;
int boundx2 = 62;
int boundy1 = 20;
int boundy2 = 41;

// Array of temperature readings at each simulation cell
byte heat[NUM_LEDS];

uint32_t noise_x;
uint32_t noise_y;
uint32_t noise_z;
uint32_t noise_scale_x;
uint32_t noise_scale_y;

uint8_t noise[MATRIX_WIDTH][MATRIX_HEIGHT];
uint8_t noisesmoothing;

bool direction = false;
byte theta = 0;
byte hueoffset2 = 0;
int patternIndex = 7;
// snake stuff.

static const byte SNAKE_LENGTH = 16;

CRGB colors[SNAKE_LENGTH];
uint8_t initialHue;

enum Direction
{
  UP,
  DOWN,
  LEFT,
  RIGHT
};

struct Pixel
{
  uint8_t x;
  uint8_t y;
};

// cube stuff
float zOff = 150.0f;
float xOff = 0.0f;
float yOff = 0.0f;
float cSize = 40.0f;
int view_plane = 64;
float angle = M_PI / 100;
int rsteps = 100;
float pyrimid3d[8][3] = {
    {-cSize, cSize, zOff - cSize}, // 4
    {cSize, cSize, zOff - cSize},  // 1
    {-cSize, yOff - cSize, zOff - cSize},
    {0, yOff - cSize / 2, zOff},   // top point
    {-cSize, cSize, zOff + cSize}, // 2
    {cSize, cSize, zOff + cSize},  // 3
    {-cSize, -cSize, zOff + cSize},
    {cSize, -cSize, zOff + cSize}};

float cube3d[8][3] = {
    {xOff - cSize, yOff + cSize, zOff - cSize},
    {xOff + cSize, yOff + cSize, zOff - cSize},
    {xOff - cSize, yOff - cSize, zOff - cSize},
    {xOff + cSize, yOff - cSize, zOff - cSize},
    {xOff - cSize, yOff + cSize, zOff + cSize},
    {xOff + cSize, yOff + cSize, zOff + cSize},
    {xOff - cSize, yOff - cSize, zOff + cSize},
    {xOff + cSize, yOff - cSize, zOff + cSize}};

unsigned char cube2d[8][2];

static const int boidCount = 50;



//byte hue = 0;
bool predatorPresent = true;

class Effects
{
public:
  // CRGB *leds;
  CRGB leds[NUM_LEDS];

  // CRGB leds2[NUM_LEDS]; // Faptastic: getting rid of this and any dependant effects or algos. to save memory 24*64*32 bytes of ram (50k).

  /* The only 'framebuffer' we have is what is contained in the leds and leds2 variables.
   * We don't store what the color a particular pixel might be, other than when it's turned
   * into raw electrical signal output gobbly-gook (i.e. the DMA matrix buffer), but this * is not reversible.
   *
   * As such, any time these effects want to write a pixel color, we first have to update
   * the leds or leds2 array, and THEN write it to the RGB panel. This enables us to 'look up' the array to see what a pixel color was previously, each drawFrame().
   */
  void savePixel(int16_t x, int16_t y, CRGB color)
  {

    if (x < 0 || x > 63 || y < 0 || y > 63)
    {
      ; // safety
    }
    else
    {
      //if (!currentBuffer.isPixel(x, y)) // box for time
      //{
        leds[XY(x, y)] = color; //; //do nothing
      //}
    }
  }

  // write one pixel with the specified color from the current palette to coordinates NOW
  void Pixel(int x, int y, uint8_t colorIndex)
  {
    if (!currentBuffer.isPixel(x, y)) // box for time
    {
      CRGB temp = ColorFromCurrentPalette(colorIndex);
      display.drawPixelRGB888(x, y, temp.r, temp.g, temp.b); // now draw it?
    }
  }
  void ShowFrameComplete() // Clears all with no regard to center time.
  {
    for (int y = 0; y < MATRIX_HEIGHT; y++)
    {
      for (int x = 0; x < MATRIX_WIDTH; x++)
      {

        CRGB tmp_led = leds[XY(x, y)];
        display.drawPixelRGB888(x, y, tmp_led.r, tmp_led.g, tmp_led.b);
      } // end loop to copy fast led to the dma matrix
    }
  }

  void ShowFrame()
  {
    // #if (FASTLED_VERSION >= 3001000)
    //       nblendPaletteTowardPalette(currentPalette, targetPalette, 24);
    // #else
    //  currentPalette = targetPalette;
    // #endif

    //  backgroundLayer.swapBuffers();
    // leds = (CRGB*) backgroundLayer.backBuffer();
    // LEDS.countFPS();

    for (int y = 0; y < MATRIX_HEIGHT; y++)
    {
      for (int x = 0; x < MATRIX_WIDTH; x++)
      {
        if (!currentBuffer.isPixel(x, y))
        {
          CRGB tmp_led = leds[XY(x, y)];
          display.drawPixelRGB888(x, y, tmp_led.r, tmp_led.g, tmp_led.b);
        }
      } // end loop to copy fast led to the dma matrix
    }
  }
  // scale the brightness of the screenbuffer down
  void DimAll(byte value) // Have to fix this
  {
    for (int i = 0; i < NUM_LEDS; i++)
    {
      leds[i].nscale8(value);
    }
  }

  void ClearFrame()
  {
    memset(leds, 0x00, sizeof(leds)); // flush
  }

  /*
    void CircleStream(uint8_t value) {
      DimAll(value); ShowFrame();

      for (uint8_t offset = 0; offset < MATRIX_CENTER_X; offset++) {
        boolean hasprev = false;
        uint16_t prevxy = 0;

        for (uint8_t theta = 0; theta < 255; theta++) {
          uint8_t x = mapcos8(theta, offset, (MATRIX_WIDTH - 1) - offset);
          uint8_t y = mapsin8(theta, offset, (MATRIX_HEIGHT - 1) - offset);

          uint16_t xy = XY(x, y);

          if (hasprev) {
            leds[prevxy] += leds[xy];
          }

          prevxy = xy;
          hasprev = true;
        }
      }

      for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
        for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
          uint16_t xy = XY(x, y);
          leds[xy] = leds2[xy];
          leds[xy].nscale8(value);
          leds2[xy].nscale8(value);
        }
      }
    }
  */

  // palettes
  static const int paletteCount = 10;
  int paletteIndex = -1;
  TBlendType currentBlendType = LINEARBLEND;
  CRGBPalette16 currentPalette;
  CRGBPalette16 targetPalette;
  char *currentPaletteName;

  static const int HeatColorsPaletteIndex = 6;
  static const int RandomPaletteIndex = 9;

  void Setup()
  {
    currentPalette = RainbowColors_p;
    loadPalette(0);
    NoiseVariablesSetup();
  }

  void CyclePalette(int offset = 1)
  {
    loadPalette(paletteIndex + offset);
  }

  void RandomPalette()
  {
    loadPalette(RandomPaletteIndex);
  }

  void loadPalette(int index)
  {
    paletteIndex = index;

    if (paletteIndex >= paletteCount)
      paletteIndex = 0;
    else if (paletteIndex < 0)
      paletteIndex = paletteCount - 1;

    switch (paletteIndex)
    {
    case 0:
      targetPalette = RainbowColors_p;
      currentPaletteName = (char *)"Rainbow";
      break;
      // case 1:
      //   targetPalette = RainbowStripeColors_p;
      //   currentPaletteName = (char *)"RainbowStripe";
      //   break;
    case 1:
      targetPalette = OceanColors_p;
      currentPaletteName = (char *)"Ocean";
      break;
    case 2:
      targetPalette = CloudColors_p;
      currentPaletteName = (char *)"Cloud";
      break;
    case 3:
      targetPalette = ForestColors_p;
      currentPaletteName = (char *)"Forest";
      break;
    case 4:
      targetPalette = PartyColors_p;
      currentPaletteName = (char *)"Party";
      break;
    case 5:
      setupGrayscalePalette();
      currentPaletteName = (char *)"Grey";
      break;
    case HeatColorsPaletteIndex:
      targetPalette = HeatColors_p;
      currentPaletteName = (char *)"Heat";
      break;
    case 7:
      targetPalette = LavaColors_p;
      currentPaletteName = (char *)"Lava";
      break;
    case 8:
      setupIcePalette();
      currentPaletteName = (char *)"Ice";
      break;
    case RandomPaletteIndex:
      loadPalette(random(0, paletteCount - 1));
      paletteIndex = RandomPaletteIndex;
      currentPaletteName = (char *)"Random";
      break;
    }
  }

  void setPalette(String paletteName)
  {
    if (paletteName == "Rainbow")
      loadPalette(0);
    // else if (paletteName == "RainbowStripe")
    //   loadPalette(1);
    else if (paletteName == "Ocean")
      loadPalette(1);
    else if (paletteName == "Cloud")
      loadPalette(2);
    else if (paletteName == "Forest")
      loadPalette(3);
    else if (paletteName == "Party")
      loadPalette(4);
    else if (paletteName == "Grayscale")
      loadPalette(5);
    else if (paletteName == "Heat")
      loadPalette(6);
    else if (paletteName == "Lava")
      loadPalette(7);
    else if (paletteName == "Ice")
      loadPalette(8);
    else if (paletteName == "Random")
      RandomPalette();
  }

  void listPalettes()
  {
    Serial.println(F("{"));
    Serial.print(F("  \"count\": "));
    Serial.print(paletteCount);
    Serial.println(",");
    Serial.println(F("  \"results\": ["));

    String paletteNames[] = {
        "Rainbow",
        // "RainbowStripe",
        "Ocean",
        "Cloud",
        "Forest",
        "Party",
        "Grayscale",
        "Heat",
        "Lava",
        "Ice",
        "Random"};

    for (int i = 0; i < paletteCount; i++)
    {
      Serial.print(F("    \""));
      Serial.print(paletteNames[i]);
      if (i == paletteCount - 1)
        Serial.println(F("\""));
      else
        Serial.println(F("\","));
    }

    Serial.println("  ]");
    Serial.println("}");
  }

  void setupGrayscalePalette()
  {
    targetPalette = CRGBPalette16(CRGB::Black, CRGB::White);
  }

  void setupIcePalette()
  {
    targetPalette = CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);
  }

  // Oscillators and Emitters

  // the oscillators: linear ramps 0-255
  byte osci[6];

  // sin8(osci) swinging between 0 to MATRIX_WIDTH - 1
  byte p[6];

  // set the speeds (and by that ratios) of the oscillators here
  void MoveOscillators()
  {
    osci[0] = osci[0] + 5;
    osci[1] = osci[1] + 2;
    osci[2] = osci[2] + 3;
    osci[3] = osci[3] + 4;
    osci[4] = osci[4] + 1;
    if (osci[4] % 2 == 0)
      osci[5] = osci[5] + 1; // .5
    for (int i = 0; i < 4; i++)
    {
      p[i] = map8(sin8(osci[i]), 0, MATRIX_WIDTH - 1); // why? to keep the result in the range of 0-MATRIX_WIDTH (matrix size)
    }
  }

  // All the caleidoscope functions work directly within the screenbuffer (leds array).
  // Draw whatever you like in the area x(0-15) and y (0-15) and then copy it arround.

  // rotates the first 16x16 quadrant 3 times onto a 32x32 (+90 degrees rotation for each one)
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

  // mirror the first 16x16 quadrant 3 times onto a 32x32
  void Caleidoscope2()
  {
    for (int x = 0; x < MATRIX_CENTER_X; x++)
    {
      for (int y = 0; y < MATRIX_CENTER_Y; y++)
      {
        leds[XY(MATRIX_WIDTH - 1 - x, y)] = leds[XY(y, x)];
        leds[XY(x, MATRIX_HEIGHT - 1 - y)] = leds[XY(y, x)];
        leds[XY(MATRIX_WIDTH - 1 - x, MATRIX_HEIGHT - 1 - y)] = leds[XY(x, y)];
      }
    }
  }

  // copy one diagonal triangle into the other one within a 16x16
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

  // copy one diagonal triangle into the other one within a 16x16 (90 degrees rotated compared to Caleidoscope3)
  void Caleidoscope4()
  {
    for (int x = 0; x <= MATRIX_CENTRE_X; x++)
    {
      for (int y = 0; y <= MATRIX_CENTRE_Y - x; y++)
      {
        leds[XY(MATRIX_CENTRE_Y - y, MATRIX_CENTRE_X - x)] = leds[XY(x, y)];
      }
    }
  }

  // copy one diagonal triangle into the other one within a 8x8
  void Caleidoscope5()
  {
    for (int x = 0; x < MATRIX_WIDTH / 4; x++)
    {
      for (int y = 0; y <= x; y++)
      {
        leds[XY(x, y)] = leds[XY(y, x)];
      }
    }

    for (int x = MATRIX_WIDTH / 4; x < MATRIX_WIDTH / 2; x++)
    {
      for (int y = MATRIX_HEIGHT / 4; y >= 0; y--)
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

  // create a square twister to the left or counter-clockwise
  // x and y for center, r for radius
  void SpiralStream(int x, int y, int r, byte dimm)
  {
    for (int d = r; d >= 0; d--)
    { // from the outside to the inside
      for (int i = x - d; i <= x + d; i++)
      {
        leds[XY(i, y - d)] += leds[XY(i + 1, y - d)]; // lowest row to the right
        leds[XY(i, y - d)].nscale8(dimm);
      }
      for (int i = y - d; i <= y + d; i++)
      {
        leds[XY(x + d, i)] += leds[XY(x + d, i + 1)]; // right colum up
        leds[XY(x + d, i)].nscale8(dimm);
      }
      for (int i = x + d; i >= x - d; i--)
      {
        leds[XY(i, y + d)] += leds[XY(i - 1, y + d)]; // upper row to the left
        leds[XY(i, y + d)].nscale8(dimm);
      }
      for (int i = y + d; i >= y - d; i--)
      {
        leds[XY(x - d, i)] += leds[XY(x - d, i - 1)]; // left colum down
        leds[XY(x - d, i)].nscale8(dimm);
      }
    }
  }

  // expand everything within a circle
  void Expand(int centerX, int centerY, int radius, byte dimm)
  {
    if (radius == 0)
      return;

    int currentRadius = radius;

    while (currentRadius > 0)
    {
      int a = radius, b = 0;
      int radiusError = 1 - a;

      int nextRadius = currentRadius - 1;
      int nextA = nextRadius - 1, nextB = 0;
      int nextRadiusError = 1 - nextA;

      while (a >= b)
      {
        // move them out one pixel on the radius
        leds[XY(a + centerX, b + centerY)] = leds[XY(nextA + centerX, nextB + centerY)];
        leds[XY(b + centerX, a + centerY)] = leds[XY(nextB + centerX, nextA + centerY)];
        leds[XY(-a + centerX, b + centerY)] = leds[XY(-nextA + centerX, nextB + centerY)];
        leds[XY(-b + centerX, a + centerY)] = leds[XY(-nextB + centerX, nextA + centerY)];
        leds[XY(-a + centerX, -b + centerY)] = leds[XY(-nextA + centerX, -nextB + centerY)];
        leds[XY(-b + centerX, -a + centerY)] = leds[XY(-nextB + centerX, -nextA + centerY)];
        leds[XY(a + centerX, -b + centerY)] = leds[XY(nextA + centerX, -nextB + centerY)];
        leds[XY(b + centerX, -a + centerY)] = leds[XY(nextB + centerX, -nextA + centerY)];

        // dim them
        leds[XY(a + centerX, b + centerY)].nscale8(dimm);
        leds[XY(b + centerX, a + centerY)].nscale8(dimm);
        leds[XY(-a + centerX, b + centerY)].nscale8(dimm);
        leds[XY(-b + centerX, a + centerY)].nscale8(dimm);
        leds[XY(-a + centerX, -b + centerY)].nscale8(dimm);
        leds[XY(-b + centerX, -a + centerY)].nscale8(dimm);
        leds[XY(a + centerX, -b + centerY)].nscale8(dimm);
        leds[XY(b + centerX, -a + centerY)].nscale8(dimm);

        b++;
        if (radiusError < 0)
          radiusError += 2 * b + 1;
        else
        {
          a--;
          radiusError += 2 * (b - a + 1);
        }

        nextB++;
        if (nextRadiusError < 0)
          nextRadiusError += 2 * nextB + 1;
        else
        {
          nextA--;
          nextRadiusError += 2 * (nextB - nextA + 1);
        }
      }

      currentRadius--;
    }
  }

  // give it a linear tail to the right
  void StreamRight(byte scale, int fromX = 0, int toX = MATRIX_WIDTH, int fromY = 0, int toY = MATRIX_HEIGHT)
  {
    for (int x = fromX + 1; x < toX; x++)
    {
      for (int y = fromY; y < toY; y++)
      {
        leds[XY(x, y)] += leds[XY(x - 1, y)];
        leds[XY(x, y)].nscale8(scale);
      }
    }
    for (int y = fromY; y < toY; y++)
      leds[XY(0, y)].nscale8(scale);
  }

  // give it a linear tail to the left
  void StreamLeft(byte scale, int fromX = MATRIX_WIDTH, int toX = 0, int fromY = 0, int toY = MATRIX_HEIGHT)
  {
    for (int x = toX; x < fromX; x++)
    {
      for (int y = fromY; y < toY; y++)
      {
        leds[XY(x, y)] += leds[XY(x + 1, y)];
        leds[XY(x, y)].nscale8(scale);
      }
    }
    for (int y = fromY; y < toY; y++)
      leds[XY(0, y)].nscale8(scale);
  }

  // give it a linear tail downwards
  void StreamDown(byte scale)
  {
    for (int x = 0; x < MATRIX_WIDTH; x++)
    {
      for (int y = 1; y < MATRIX_HEIGHT; y++)
      {
        leds[XY(x, y)] += leds[XY(x, y - 1)];
        leds[XY(x, y)].nscale8(scale);
      }
    }
    for (int x = 0; x < MATRIX_WIDTH; x++)
      leds[XY(x, 0)].nscale8(scale);
  }

  // give it a linear tail upwards
  void StreamUp(byte scale)
  {
    for (int x = 0; x < MATRIX_WIDTH; x++)
    {
      for (int y = MATRIX_HEIGHT - 2; y >= 0; y--)
      {
        leds[XY(x, y)] += leds[XY(x, y + 1)];
        leds[XY(x, y)].nscale8(scale);
      }
    }
    for (int x = 0; x < MATRIX_WIDTH; x++)
      leds[XY(x, MATRIX_HEIGHT - 1)].nscale8(scale);
  }

  // give it a linear tail up and to the left
  void StreamUpAndLeft(byte scale)
  {
    for (int x = 0; x < MATRIX_WIDTH - 1; x++)
    {
      for (int y = MATRIX_HEIGHT - 2; y >= 0; y--)
      {
        leds[XY(x, y)] += leds[XY(x + 1, y + 1)];
        leds[XY(x, y)].nscale8(scale);
      }
    }
    for (int x = 0; x < MATRIX_WIDTH; x++)
      leds[XY(x, MATRIX_HEIGHT - 1)].nscale8(scale);
    for (int y = 0; y < MATRIX_HEIGHT; y++)
      leds[XY(MATRIX_WIDTH - 1, y)].nscale8(scale);
  }

  // give it a linear tail up and to the right
  void StreamUpAndRight(byte scale)
  {
    for (int x = 0; x < MATRIX_WIDTH - 1; x++)
    {
      for (int y = MATRIX_HEIGHT - 2; y >= 0; y--)
      {
        leds[XY(x + 1, y)] += leds[XY(x, y + 1)];
        leds[XY(x, y)].nscale8(scale);
      }
    }
    // fade the bottom row
    for (int x = 0; x < MATRIX_WIDTH; x++)
      leds[XY(x, MATRIX_HEIGHT - 1)].nscale8(scale);

    // fade the right column
    for (int y = 0; y < MATRIX_HEIGHT; y++)
      leds[XY(MATRIX_WIDTH - 1, y)].nscale8(scale);
  }

  // just move everything one line down
  void MoveDown()
  {
    for (int y = MATRIX_HEIGHT - 1; y > 0; y--)
    {
      for (int x = 0; x < MATRIX_WIDTH; x++)
      {
        leds[XY(x, y)] = leds[XY(x, y - 1)];
      }
    }
  }

  // just move everything one line down
  void VerticalMoveFrom(int start, int end)
  {
    for (int y = end; y > start; y--)
    {
      for (int x = 0; x < MATRIX_WIDTH; x++)
      {
        leds[XY(x, y)] = leds[XY(x, y - 1)];
      }
    }
  }

  // copy the rectangle defined with 2 points x0, y0, x1, y1
  // to the rectangle beginning at x2, x3
  void Copy(byte x0, byte y0, byte x1, byte y1, byte x2, byte y2)
  {
    for (int y = y0; y < y1 + 1; y++)
    {
      for (int x = x0; x < x1 + 1; x++)
      {
        leds[XY(x + x2 - x0, y + y2 - y0)] = leds[XY(x, y)];
      }
    }
  }

  // rotate + copy triangle (MATRIX_CENTER_X*MATRIX_CENTER_X)
  void RotateTriangle()
  {
    for (int x = 1; x < MATRIX_CENTER_X; x++)
    {
      for (int y = 0; y < x; y++)
      {
        leds[XY(x, 7 - y)] = leds[XY(7 - x, y)];
      }
    }
  }

  // mirror + copy triangle (MATRIX_CENTER_X*MATRIX_CENTER_X)
  void MirrorTriangle()
  {
    for (int x = 1; x < MATRIX_CENTER_X; x++)
    {
      for (int y = 0; y < x; y++)
      {
        leds[XY(7 - y, x)] = leds[XY(7 - x, y)];
      }
    }
  }

  // draw static rainbow triangle pattern (MATRIX_CENTER_XxWIDTH / 2)
  // (just for debugging)
  void RainbowTriangle()
  {
    for (int i = 0; i < MATRIX_CENTER_X; i++)
    {
      for (int j = 0; j <= i; j++)
      {
        Pixel(7 - i, j, i * j * 4);
      }
    }
  }



  void BresenhamLine(int x0, int y0, int x1, int y1, CRGB color)
  {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    for (;;)
    {
      savePixel(x0, y0, color);
      if (x0 == x1 && y0 == y1)
        break;
      e2 = 2 * err;
      if (e2 > dy)
      {
        err += dy;
        x0 += sx;
      }
      if (e2 < dx)
      {
        err += dx;
        y0 += sy;
      }
    }
  }

  // write one pixel with the specified color from the current palette to coordinates
  /*
  void Pixel(int x, int y, uint8_t colorIndex) {
    leds[XY(x, y)] = ColorFromCurrentPalette(colorIndex);
    matrix.drawBackgroundPixelRGB888(x,y, leds[XY(x, y)]); // now draw it?
  }
  */
/*
  CRGB ColorFromCurrentPalette(uint8_t index = 0, uint8_t brightness = 255, TBlendType blendType = LINEARBLEND)
  {
    return ColorFromPalette(currentPalette, index, brightness, currentBlendType);
  }
  */

  CRGB HsvToRgb(uint8_t h, uint8_t s, uint8_t v)
  {
    CHSV hsv = CHSV(h, s, v);
    CRGB rgb;
    hsv2rgb_spectrum(hsv, rgb);
    return rgb;
  }

  void NoiseVariablesSetup()
  {
    noisesmoothing = 200;

    noise_x = random16();
    noise_y = random16();
    noise_z = random16();
    noise_scale_x = 6000;
    noise_scale_y = 6000;
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

  // non leds2 memory version.
  void MoveX(byte delta)
  {

    CRGB tmp = 0;

    for (int y = 0; y < MATRIX_HEIGHT; y++)
    {

      // Shift Left: https://codedost.com/c/arraypointers-in-c/c-program-shift-elements-array-left-direction/
      // Computationally heavier but doesn't need an entire leds2 array

      tmp = leds[XY(0, y)];
      for (int m = 0; m < delta; m++)
      {
        // Do this delta time for each row... computationally expensive potentially.
        for (int x = 0; x < MATRIX_WIDTH; x++)
        {
          leds[XY(x, y)] = leds[XY(x + 1, y)];
        }

        leds[XY(MATRIX_WIDTH - 1, y)] = tmp;
      }

      /*
      // Shift
      for (int x = 0; x < MATRIX_WIDTH - delta; x++) {
        leds2[XY(x, y)] = leds[XY(x + delta, y)];
      }

      // Wrap around
      for (int x = MATRIX_WIDTH - delta; x < MATRIX_WIDTH; x++) {
        leds2[XY(x, y)] = leds[XY(x + delta - MATRIX_WIDTH, y)];
      }
      */
    } // end row loop

    /*
    // write back to leds
    for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
      for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
        leds[XY(x, y)] = leds2[XY(x, y)];
      }
    }
    */
  }

  void MoveY(byte delta)
  {

    CRGB tmp = 0;
    for (int x = 0; x < MATRIX_WIDTH; x++)
    {
      tmp = leds[XY(x, 0)];
      for (int m = 0; m < delta; m++) // moves
      {
        // Do this delta time for each row... computationally expensive potentially.
        for (int y = 0; y < MATRIX_HEIGHT; y++)
        {
          leds[XY(x, y)] = leds[XY(x, y + 1)];
        }

        leds[XY(x, MATRIX_HEIGHT - 1)] = tmp;
      }
    } // end column loop
  }   /// MoveY

  // cube stuff
  void swap_int16_t(int16_t *a, int16_t *b)
  {
    int16_t t = *a;
    a = b;
    *b = t;
  }

  void writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, CRGB color)
  {
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep)
    {
      swap_int16_t(&x0, &y0);
      swap_int16_t(&x1, &y1);
    }

    if (x0 > x1)
    {
      swap_int16_t(&x0, &x1);
      swap_int16_t(&y0, &y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1)
    {
      ystep = 1;
    }
    else
    {
      ystep = -1;
    }

    for (; x0 <= x1; x0++)
    {
      if (steep)
      {

        savePixel(y0, x0, color);
      }
      else
      {
        savePixel(x0, y0, color);
      }
      err -= dy;
      if (err < 0)
      {
        y0 += ystep;
        err += dx;
      }
    }
  }

  void drawFastVLine(int16_t x, int16_t y, int16_t h, CRGB color)
  {
    writeLine(x, y, x, y + h - 1, color);
  }

  void drawFastHLine(int16_t x, int16_t y, int16_t w, CRGB color)
  {
    writeLine(x, y, x + w - 1, y, color);
  }

  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, CRGB color)
  {
    // Update in subclasses if desired!
    if (x0 == x1)
    {
      if (y0 > y1)
        swap_int16_t(&y0, &y1);
      drawFastVLine(x0, y0, y1 - y0 + 1, color);
    }
    else if (y0 == y1)
    {
      if (x0 > x1)
        swap_int16_t(&x0, &x1);
      drawFastHLine(x0, y0, x1 - x0 + 1, color);
    }
    else
    {
      writeLine(x0, y0, x1, y1, color);
    }
  }
};
