#include <FastLED.h>

FASTLED_USING_NAMESPACE

// FastLED "100-lines-of-code" demo reel, showing just a few 
// of the kinds of animation patterns you can quickly and easily 
// compose using FastLED.  
//
// This example also shows one easy way to define multiple 
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    2
//#define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    48
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

#define NOISE_SCALE 100000

float fractalNoise(float x, float y, float z) {
  float r = 0;
  float amp = 1.0;
  for (int octave = 0; octave < 4; octave++) {
    r += ((float)inoise16(x * NOISE_SCALE, y * NOISE_SCALE, z * NOISE_SCALE)) * amp / UINT16_MAX;
    amp /= 2;
    x *= 2;
    y *= 2;
    z *= 2;
  }
  return r;
}

int width = 128;
int height = 128;

float dx, dy, dz;

float noise(float x){
  return ((float)inoise16(x * NOISE_SCALE))/ UINT16_MAX;
}

void rainbow() 
{
    long now = millis();
    float speed = 0.002;
    float zspeed = 0.1;
    float angle = sin(now * 0.001);
    float z = now * 0.00008;
    float hue = now * 0.01;
    float scale = 0.005;

    float saturation = 100 * constrain(pow(1.15 * noise(now * 0.000122), 2.5), 0, 1);
    float spacing = noise(now * 0.000124) * 0.1;

    dx += cos(angle) * speed;
    dy += sin(angle) * speed;
    dz += (noise(now * 0.000014) - 0.5) * zspeed;

    float centerx = noise(now *  0.000125) * 1.25 * width;
    float centery = noise(now * -0.000125) * 1.25 * height;

    for (int i = 0; i < NUM_LEDS; i++){
      float rho = i * PI * 2 / NUM_LEDS;
      float x = (sin(rho) + 1) * 64;
      float y = (cos(rho) + 1) * 64;

      float dist = sqrt(pow(x - centerx, 2) + pow(y - centery, 2));
      float pulse = (sin(dz + dist * spacing) - 0.3) * 0.3;
      
      float n = fractalNoise(dx + x*scale + pulse, dy + y*scale, z) - 0.75;
      float m = fractalNoise(dx + x*scale, dy + y*scale, z + 10.0) - 0.75;

      float hue_final = (hue + 40.0 * m);
      while (hue_final > 100) hue_final -= 100;
      uint8_t i_hue = hue_final * 2.55;
      uint8_t i_sat = saturation * 2.55;
      uint8_t i_val = 255 * constrain(pow(3.0 * n, 1.5), 0, 0.9);

      leds[i] = CHSV(i_hue, i_sat, i_val);
  }

}

void setup() {
  delay(3000); // 3 second delay for recovery
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  delay(500);
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow };
  
void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void loop()
{
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
}