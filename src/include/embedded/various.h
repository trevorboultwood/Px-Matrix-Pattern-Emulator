#include<stdint.h>
#include<string.h>
typedef uint8_t   fract8;
typedef uint16_t  fract16;
typedef uint16_t  accum88;
typedef uint32_t TProgmemRGBPalette16[16];
typedef uint32_t TProgmemHSVPalette16[16];
#define saccum87 int16_t
#define FL_PROGMEM
#define FL_PGM_READ_BYTE_NEAR(x)  (*((const  uint8_t*)(x)))
#define FL_PGM_READ_WORD_NEAR(x)  (*((const uint16_t*)(x)))
#define FL_PGM_READ_DWORD_NEAR(x) (*((const uint32_t*)(x)))
#define FL_PGM_READ_DWORD_NEAR(x) (*((const uint32_t*)(x)))
uint8_t scale8( uint8_t i, fract8 scale)
{

    return (((uint16_t)i) * (1+(uint16_t)(scale))) >> 8;
}


uint8_t lerp8by8( uint8_t a, uint8_t b, fract8 frac)
{
    uint8_t result;
    if( b > a) {
        uint8_t delta = b - a;
        uint8_t scaled = scale8( delta, frac);
        result = a + scaled;
    } else {
        uint8_t delta = a - b;
        uint8_t scaled = scale8( delta, frac);
        result = a - scaled;
    }
    return result;
}


uint16_t lerp16by16( uint16_t a, uint16_t b, fract16 frac)
{
    uint16_t result;
    if( b > a ) {
        uint16_t delta = b - a;
        uint16_t scaled = scale16(delta, frac);
        result = a + scaled;
    } else {
        uint16_t delta = a - b;
        uint16_t scaled = scale16( delta, frac);
        result = a - scaled;
    }
    return result;
}


uint16_t scale16( uint16_t i, fract16 scale )
{ 
    uint16_t result;
    result = ((uint32_t)(i) * (1+(uint32_t)(scale))) / 65536;
}
 uint16_t beat88( accum88 beats_per_minute_88, uint32_t timebase = 0)
{
    // BPM is 'beats per minute', or 'beats per 60000ms'.
    // To avoid using the (slower) division operator, we
    // want to convert 'beats per 60000ms' to 'beats per 65536ms',
    // and then use a simple, fast bit-shift to divide by 65536.
    //
    // The ratio 65536:60000 is 279.620266667:256; we'll call it 280:256.
    // The conversion is accurate to about 0.05%, more or less,
    // e.g. if you ask for "120 BPM", you'll get about "119.93".
    printf("THis is not implemnted");
    return 0;//(((//GET_MILLIS()) - timebase) * beats_per_minute_88 * 280) >> 16;
}
uint16_t beat16( accum88 beats_per_minute, uint32_t timebase = 0)
{
    // Convert simple 8-bit BPM's to full Q8.8 accum88's if needed
    if( beats_per_minute < 256) beats_per_minute <<= 8;
    return beat88(beats_per_minute, timebase);
}
uint8_t beat8( accum88 beats_per_minute, uint32_t timebase = 0)
{
    return beat16( beats_per_minute, timebase) >> 8;
}

void memmove8(void* to, void* from, uint16_t size)
{
    memmove(to,from,size);
}
struct CHSV {
    union {
		struct {
		    union {
		        uint8_t hue;
		        uint8_t h; };
		    union {
		        uint8_t saturation;
		        uint8_t sat;
		        uint8_t s; };
		    union {
		        uint8_t value;
		        uint8_t val;
		        uint8_t v; };
		};
		uint8_t raw[3];
	};

    /// Array access operator to index into the chsv object
	inline uint8_t& operator[] (uint8_t x) __attribute__((always_inline))
    {
        return raw[x];
    }

    /// Array access operator to index into the chsv object
    inline const uint8_t& operator[] (uint8_t x) const __attribute__((always_inline))
    {
        return raw[x];
    }

    /// default values are UNITIALIZED
    inline CHSV() __attribute__((always_inline)) = default;

    /// allow construction from H, S, V
    inline CHSV( uint8_t ih, uint8_t is, uint8_t iv) __attribute__((always_inline))
        : h(ih), s(is), v(iv)
    {
    }

    /// allow copy construction
    inline CHSV(const CHSV& rhs) __attribute__((always_inline)) = default;

    inline CHSV& operator= (const CHSV& rhs) __attribute__((always_inline)) = default;

    inline CHSV& setHSV(uint8_t ih, uint8_t is, uint8_t iv) __attribute__((always_inline))
    {
        h = ih;
        s = is;
        v = iv;
        return *this;
    }
};

void fill_solid( struct CRGB * leds, int numToFill,
                 const struct CRGB& color)
{
    for( int i = 0; i < numToFill; ++i) {
        leds[i] = color;
    }
}

void fill_solid( struct CHSV * targetArray, int numToFill,
                 const struct CHSV& hsvColor)
{
    for( int i = 0; i < numToFill; ++i) {
        targetArray[i] = hsvColor;
    }
}

void fill_rainbow( struct CRGB * pFirstLED, int numToFill,
                  uint8_t initialhue,
                  uint8_t deltahue )
{
    CHSV hsv;
    hsv.hue = initialhue;
    hsv.val = 255;
    hsv.sat = 240;
    for( int i = 0; i < numToFill; ++i) {
        pFirstLED[i] = hsv;
        hsv.hue += deltahue;
    }
}

void fill_rainbow( struct CHSV * targetArray, int numToFill,
                  uint8_t initialhue,
                  uint8_t deltahue )
{
    CHSV hsv;
    hsv.hue = initialhue;
    hsv.val = 255;
    hsv.sat = 240;
    for( int i = 0; i < numToFill; ++i) {
        targetArray[i] = hsv;
        hsv.hue += deltahue;
    }
}


void fill_gradient_RGB( CRGB* leds,
                   uint16_t startpos, CRGB startcolor,
                   uint16_t endpos,   CRGB endcolor )
{
    // if the points are in the wrong order, straighten them
    if( endpos < startpos ) {
        uint16_t t = endpos;
        CRGB tc = endcolor;
        endcolor = startcolor;
        endpos = startpos;
        startpos = t;
        startcolor = tc;
    }

    saccum87 rdistance87;
    saccum87 gdistance87;
    saccum87 bdistance87;

    rdistance87 = (endcolor.r - startcolor.r) << 7;
    gdistance87 = (endcolor.g - startcolor.g) << 7;
    bdistance87 = (endcolor.b - startcolor.b) << 7;

    uint16_t pixeldistance = endpos - startpos;
    int16_t divisor = pixeldistance ? pixeldistance : 1;

    saccum87 rdelta87 = rdistance87 / divisor;
    saccum87 gdelta87 = gdistance87 / divisor;
    saccum87 bdelta87 = bdistance87 / divisor;

    rdelta87 *= 2;
    gdelta87 *= 2;
    bdelta87 *= 2;

    accum88 r88 = startcolor.r << 8;
    accum88 g88 = startcolor.g << 8;
    accum88 b88 = startcolor.b << 8;
    for( uint16_t i = startpos; i <= endpos; ++i) {
        leds[i] = CRGB( r88 >> 8, g88 >> 8, b88 >> 8);
        r88 += rdelta87;
        g88 += gdelta87;
        b88 += bdelta87;
    }
}
class CHSVPalette16 {
public:
    CHSV entries[16];
    CHSVPalette16() {};
    CHSVPalette16( const CHSV& c00,const CHSV& c01,const CHSV& c02,const CHSV& c03,
                    const CHSV& c04,const CHSV& c05,const CHSV& c06,const CHSV& c07,
                    const CHSV& c08,const CHSV& c09,const CHSV& c10,const CHSV& c11,
                    const CHSV& c12,const CHSV& c13,const CHSV& c14,const CHSV& c15 )
    {
        entries[0]=c00; entries[1]=c01; entries[2]=c02; entries[3]=c03;
        entries[4]=c04; entries[5]=c05; entries[6]=c06; entries[7]=c07;
        entries[8]=c08; entries[9]=c09; entries[10]=c10; entries[11]=c11;
        entries[12]=c12; entries[13]=c13; entries[14]=c14; entries[15]=c15;
    };

    CHSVPalette16( const CHSVPalette16& rhs)
    {
        memmove( (void *) &(entries[0]), &(rhs.entries[0]), sizeof( entries));
    }
    CHSVPalette16& operator=( const CHSVPalette16& rhs)
    {
        memmove( (void *) &(entries[0]), &(rhs.entries[0]), sizeof( entries));
        return *this;
    }

    CHSVPalette16( const TProgmemHSVPalette16& rhs)
    {
        for( uint8_t i = 0; i < 16; ++i) {
            CRGB xyz   =  FL_PGM_READ_DWORD_NEAR( rhs + i);
            entries[i].hue = xyz.red;
            entries[i].sat = xyz.green;
            entries[i].val = xyz.blue;
        }
    }
    CHSVPalette16& operator=( const TProgmemHSVPalette16& rhs)
    {
        for( uint8_t i = 0; i < 16; ++i) {
            CRGB xyz   =  FL_PGM_READ_DWORD_NEAR( rhs + i);
            entries[i].hue = xyz.red;
            entries[i].sat = xyz.green;
            entries[i].val = xyz.blue;
        }
        return *this;
    }

    inline CHSV& operator[] (uint8_t x) __attribute__((always_inline))
    {
        return entries[x];
    }
    inline const CHSV& operator[] (uint8_t x) const __attribute__((always_inline))
    {
        return entries[x];
    }

    inline CHSV& operator[] (int x) __attribute__((always_inline))
    {
        return entries[(uint8_t)x];
    }
    inline const CHSV& operator[] (int x) const __attribute__((always_inline))
    {
        return entries[(uint8_t)x];
    }

    operator CHSV*()
    {
        return &(entries[0]);
    }

    bool operator==( const CHSVPalette16 rhs)
    {
        const uint8_t* p = (const uint8_t*)(&(this->entries[0]));
        const uint8_t* q = (const uint8_t*)(&(rhs.entries[0]));
        if( p == q) return true;
        for( uint8_t i = 0; i < (sizeof( entries)); ++i) {
            if( *p != *q) return false;
            ++p;
            ++q;
        }
        return true;
    }
    bool operator!=( const CHSVPalette16 rhs)
    {
        return !( *this == rhs);
    }
    
    CHSVPalette16( const CHSV& c1)
    {
        fill_solid( &(entries[0]), 16, c1);
    }
    CHSVPalette16( const CHSV& c1, const CHSV& c2)
    {
        fill_gradient( &(entries[0]), 16, c1, c2);
    }
    CHSVPalette16( const CHSV& c1, const CHSV& c2, const CHSV& c3)
    {
        fill_gradient( &(entries[0]), 16, c1, c2, c3);
    }
    CHSVPalette16( const CHSV& c1, const CHSV& c2, const CHSV& c3, const CHSV& c4)
    {
        fill_gradient( &(entries[0]), 16, c1, c2, c3, c4);
    }

};

class CRGBPalette16 {
public:
    CRGB entries[16];
    CRGBPalette16() {};
    CRGBPalette16( const CRGB& c00,const CRGB& c01,const CRGB& c02,const CRGB& c03,
                    const CRGB& c04,const CRGB& c05,const CRGB& c06,const CRGB& c07,
                    const CRGB& c08,const CRGB& c09,const CRGB& c10,const CRGB& c11,
                    const CRGB& c12,const CRGB& c13,const CRGB& c14,const CRGB& c15 )
    {
        entries[0]=c00; entries[1]=c01; entries[2]=c02; entries[3]=c03;
        entries[4]=c04; entries[5]=c05; entries[6]=c06; entries[7]=c07;
        entries[8]=c08; entries[9]=c09; entries[10]=c10; entries[11]=c11;
        entries[12]=c12; entries[13]=c13; entries[14]=c14; entries[15]=c15;
    };

    CRGBPalette16( const CRGBPalette16& rhs)
    {
        memmove( (void *) &(entries[0]), &(rhs.entries[0]), sizeof( entries));
    }
    CRGBPalette16( const CRGB rhs[16])
    {
        memmove( (void *) &(entries[0]), &(rhs[0]), sizeof( entries));
    }
    CRGBPalette16& operator=( const CRGBPalette16& rhs)
    {
        memmove( (void *) &(entries[0]), &(rhs.entries[0]), sizeof( entries));
        return *this;
    }
    CRGBPalette16& operator=( const CRGB rhs[16])
    {
        memmove( (void *) &(entries[0]), &(rhs[0]), sizeof( entries));
        return *this;
    }

    CRGBPalette16( const CHSVPalette16& rhs)
    {
        for( uint8_t i = 0; i < 16; ++i) {
    		entries[i] = rhs.entries[i]; // implicit HSV-to-RGB conversion
        }
    }
    CRGBPalette16( const CHSV rhs[16])
    {
        for( uint8_t i = 0; i < 16; ++i) {
            entries[i] = rhs[i]; // implicit HSV-to-RGB conversion
        }
    }
    CRGBPalette16& operator=( const CHSVPalette16& rhs)
    {
        for( uint8_t i = 0; i < 16; ++i) {
    		entries[i] = rhs.entries[i]; // implicit HSV-to-RGB conversion
        }
        return *this;
    }
    CRGBPalette16& operator=( const CHSV rhs[16])
    {
        for( uint8_t i = 0; i < 16; ++i) {
            entries[i] = rhs[i]; // implicit HSV-to-RGB conversion
        }
        return *this;
    }

    CRGBPalette16( const TProgmemRGBPalette16& rhs)
    {
        for( uint8_t i = 0; i < 16; ++i) {
            entries[i] =  FL_PGM_READ_DWORD_NEAR( rhs + i);
        }
    }
    CRGBPalette16& operator=( const TProgmemRGBPalette16& rhs)
    {
        for( uint8_t i = 0; i < 16; ++i) {
            entries[i] =  FL_PGM_READ_DWORD_NEAR( rhs + i);
        }
        return *this;
    }

    bool operator==( const CRGBPalette16 rhs)
    {
        const uint8_t* p = (const uint8_t*)(&(this->entries[0]));
        const uint8_t* q = (const uint8_t*)(&(rhs.entries[0]));
        if( p == q) return true;
        for( uint8_t i = 0; i < (sizeof( entries)); ++i) {
            if( *p != *q) return false;
            ++p;
            ++q;
        }
        return true;
    }
    bool operator!=( const CRGBPalette16 rhs)
    {
        return !( *this == rhs);
    }
    
    inline CRGB& operator[] (uint8_t x) __attribute__((always_inline))
    {
        return entries[x];
    }
    inline const CRGB& operator[] (uint8_t x) const __attribute__((always_inline))
    {
        return entries[x];
    }

    inline CRGB& operator[] (int x) __attribute__((always_inline))
    {
        return entries[(uint8_t)x];
    }
    inline const CRGB& operator[] (int x) const __attribute__((always_inline))
    {
        return entries[(uint8_t)x];
    }

    operator CRGB*()
    {
        return &(entries[0]);
    }

    CRGBPalette16( const CHSV& c1)
    {
        fill_solid( &(entries[0]), 16, c1);
    }
    CRGBPalette16( const CHSV& c1, const CHSV& c2)
    {
        fill_gradient( &(entries[0]), 16, c1, c2);
    }
    CRGBPalette16( const CHSV& c1, const CHSV& c2, const CHSV& c3)
    {
        fill_gradient( &(entries[0]), 16, c1, c2, c3);
    }
    CRGBPalette16( const CHSV& c1, const CHSV& c2, const CHSV& c3, const CHSV& c4)
    {
        fill_gradient( &(entries[0]), 16, c1, c2, c3, c4);
    }

    CRGBPalette16( const CRGB& c1)
    {
        fill_solid( &(entries[0]), 16, c1);
    }
    CRGBPalette16( const CRGB& c1, const CRGB& c2)
    {
        fill_gradient_RGB( &(entries[0]), 16, c1, c2);
    }
    CRGBPalette16( const CRGB& c1, const CRGB& c2, const CRGB& c3)
    {
        fill_gradient_RGB( &(entries[0]), 16, c1, c2, c3);
    }
    CRGBPalette16( const CRGB& c1, const CRGB& c2, const CRGB& c3, const CRGB& c4)
    {
        fill_gradient_RGB( &(entries[0]), 16, c1, c2, c3, c4);
    }


    // Gradient palettes are loaded into CRGB16Palettes in such a way
    // that, if possible, every color represented in the gradient palette
    // is also represented in the CRGBPalette16.
    // For example, consider a gradient palette that is all black except
    // for a single, one-element-wide (1/256th!) spike of red in the middle:
    //     0,   0,0,0
    //   124,   0,0,0
    //   125, 255,0,0  // one 1/256th-palette-wide red stripe
    //   126,   0,0,0
    //   255,   0,0,0
    // A naive conversion of this 256-element palette to a 16-element palette
    // might accidentally completely eliminate the red spike, rendering the
    // palette completely black.
    // However, the conversions provided here would attempt to include a
    // the red stripe in the output, more-or-less as faithfully as possible.
    // So in this case, the resulting CRGBPalette16 palette would have a red
    // stripe in the middle which was 1/16th of a palette wide -- the
    // narrowest possible in a CRGBPalette16.
    // This means that the relative width of stripes in a CRGBPalette16
    // will be, by definition, different from the widths in the gradient
    // palette.  This code attempts to preserve "all the colors", rather than
    // the exact stripe widths at the expense of dropping some colors.
    CRGBPalette16( TProgmemRGBGradientPalette_bytes progpal )
    {
        *this = progpal;
    }
    CRGBPalette16& operator=( TProgmemRGBGradientPalette_bytes progpal )
    {
        TRGBGradientPaletteEntryUnion* progent = (TRGBGradientPaletteEntryUnion*)(progpal);
        TRGBGradientPaletteEntryUnion u;

        // Count entries
        uint16_t count = 0;
        do {
            u.dword = FL_PGM_READ_DWORD_NEAR(progent + count);
            ++count;
        } while ( u.index != 255);

        int8_t lastSlotUsed = -1;

        u.dword = FL_PGM_READ_DWORD_NEAR( progent);
        CRGB rgbstart( u.r, u.g, u.b);

        int indexstart = 0;
        uint8_t istart8 = 0;
        uint8_t iend8 = 0;
        while( indexstart < 255) {
            ++progent;
            u.dword = FL_PGM_READ_DWORD_NEAR( progent);
            int indexend  = u.index;
            CRGB rgbend( u.r, u.g, u.b);
            istart8 = indexstart / 16;
            iend8   = indexend   / 16;
            if( count < 16) {
                if( (istart8 <= lastSlotUsed) && (lastSlotUsed < 15)) {
                    istart8 = lastSlotUsed + 1;
                    if( iend8 < istart8) {
                        iend8 = istart8;
                    }
                }
                lastSlotUsed = iend8;
            }
            fill_gradient_RGB( &(entries[0]), istart8, rgbstart, iend8, rgbend);
            indexstart = indexend;
            rgbstart = rgbend;
        }
        return *this;
    }
    CRGBPalette16& loadDynamicGradientPalette( TDynamicRGBGradientPalette_bytes gpal )
    {
        TRGBGradientPaletteEntryUnion* ent = (TRGBGradientPaletteEntryUnion*)(gpal);
        TRGBGradientPaletteEntryUnion u;

        // Count entries
        uint16_t count = 0;
        do {
            u = *(ent + count);
            ++count;
        } while ( u.index != 255);

        int8_t lastSlotUsed = -1;


        u = *ent;
        CRGB rgbstart( u.r, u.g, u.b);

        int indexstart = 0;
        uint8_t istart8 = 0;
        uint8_t iend8 = 0;
        while( indexstart < 255) {
            ++ent;
            u = *ent;
            int indexend  = u.index;
            CRGB rgbend( u.r, u.g, u.b);
            istart8 = indexstart / 16;
            iend8   = indexend   / 16;
            if( count < 16) {
                if( (istart8 <= lastSlotUsed) && (lastSlotUsed < 15)) {
                    istart8 = lastSlotUsed + 1;
                    if( iend8 < istart8) {
                        iend8 = istart8;
                    }
                }
                lastSlotUsed = iend8;
            }
            fill_gradient_RGB( &(entries[0]), istart8, rgbstart, iend8, rgbend);
            indexstart = indexend;
            rgbstart = rgbend;
        }
        return *this;
    }

};



