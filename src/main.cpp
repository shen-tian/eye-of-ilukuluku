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

#define NUM_STRIPS 4
#define NUM_LEDS_PER_STRIP 128
#define NUM_LEDS NUM_STRIPS * NUM_LEDS_PER_STRIP
CRGB leds[NUM_STRIPS * NUM_LEDS_PER_STRIP];

float mapX[NUM_LEDS];
float mapY[NUM_LEDS];
float mapRho[NUM_LEDS];
float mapTheta[NUM_LEDS];

#define BRIGHTNESS          128
#define FRAMES_PER_SECOND  120

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

DEFINE_GRADIENT_PALETTE( iluk_gp ) {
  0,    255,  255,  255,   //white
96,   204, 255, 0,  //lime
160,    212, 2, 2,   // red
255,   235, 73, 127 // pink
};

CRGBPalette256 ilukPal = iluk_gp;

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gNextPatternNumber = 1;

uint8_t gCurrentMix = 0;
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void initXYMap(){
    for(int i = 0; i < NUM_STRIPS; i++) {
    for(int j = 0; j < NUM_LEDS_PER_STRIP; j++) {
      float x = -1000;
      float y = -1000;

      float ringSize;
      float offset;

      int index = (i*NUM_LEDS_PER_STRIP) + j;

      switch(i){
        case 0:
          ringSize = 24;
          offset = 3;
          if (j < ringSize) {
            x = 42.4 + 50 * sin(2 * PI * (j + offset) / ringSize);
            y = 42.4 + 50 * cos(2 * PI * (j + offset) / ringSize);
          }
          break;
        case 1:
          ringSize = 49.5;
          offset = 3;
          if (j < ringSize) {
            x = 140 * sin(2 * PI * (j + offset) / ringSize);
            y = 140 * cos(2 * PI * (j + offset) / ringSize);
          }
          break;
        case 2:
          ringSize = 84.9;
          offset = 2;
          if (j < ringSize) {
            x = 210 * sin(2 * PI * (j + offset) / ringSize);
            y = 210 * cos(2 * PI * (j + offset) / ringSize);
          }
          break;
        case 3:
          ringSize = 99.5;
          offset = 1;
          if (j < ringSize) {
            x = 300 * sin(2 * PI * (j + offset) / ringSize);
            y = 300 * cos(2 * PI * (j + offset) / ringSize);
          }
          break;
      }
      mapX[index] = x;
      mapY[index] = y;
      mapRho[index] = sqrtf(x * x + y * y);
      mapTheta[index] = atan2f(y, x);

      if (x > -1000) {

        Serial.print(x);
        Serial.print(",");
        Serial.print(y);
        Serial.println();
      }
    }
    }
}

#define NOISE_SCALE 100000

float fractalNoise(float x, float y, float z) {
  float r = 0;
  float amp = 1.0;
  for (int octave = 0; octave < 2; octave++) {
    r += ((float)inoise16(x * NOISE_SCALE, y * NOISE_SCALE, z * NOISE_SCALE)) * amp / UINT16_MAX;
    amp /= 2;
    x *= 2;
    y *= 2;
    z *= 2;
  }
  return r;
}

int width = 600;
int height = 600;

float dx, dy, dz;

float noise(float x){
  return ((float)inoise16(x * NOISE_SCALE))/ UINT16_MAX;
}

bool odd = false;

CRGB ringsLeds[NUM_STRIPS * NUM_LEDS_PER_STRIP];

void rings(uint8_t mix)
{
    long t0 = millis();

    long now = millis();
    float speed = 0.002;
    float zspeed = 0.1;
    float angle = sin(now * 0.001);
    float z = now * 0.00008;
    float hue = now * 0.01;
    float scale = 0.005;

    float saturation = constrain(pow(1.15 * noise(now * 0.000122), 2.5), 0, 1);
    float spacing = noise(now * 0.000124) * 0.1;

    dx += cos(angle) * speed;
    dy += sin(angle) * speed;
    dz += (noise(now * 0.000014) - 0.5) * zspeed;

    float centerx = noise(now *  0.000125) * 1.25 * width;
    float centery = noise(now * -0.000125) * 1.25 * height;

    long t1 = millis();

    float lastN = 0;

    for (int i = 0; i < NUM_LEDS; i++){
      bool doRender = odd == ((i % 2) == 1);
      if (mapX[i] > -1000 && doRender){
        float x = cos(mapTheta[i] + now * 0.0002) * mapRho[i] + 300;
        float y = sin(mapTheta[i] + now * 0.0002) * mapRho[i] + 300;

        float dist = sqrt(pow(x - centerx, 2) + pow(y - centery, 2));
        float pulse = (sin(dz + dist * spacing) - 0.3) * 0.3;

        float n = fractalNoise(dx + x*scale + pulse, dy + y*scale, z) - 0.75;

        // float m = fractalNoise(dx + x*scale, dy + y*scale, z + 10.0) - 0.75;
        float m = 0.5;

        int hue_final = ((int)(hue + 40.0 * m) % 100);
        uint8_t i_hue = hue_final * 255;
        uint8_t i_sat_l = saturation * 255;
        uint8_t i_lum = 255 * constrain(pow(3.0 * n, 0.5), 0, 0.9);

        uint8_t i_val = i_lum + saturation * 0.01 * min(i_lum, 255 - i_lum);
        uint8_t i_sat_v = (i_val == 0) ? 0 : 511 * (1 - (float)i_lum / (float)i_val);

        uint8_t index = abs(255 - 2 * i_hue);

        CRGB col = ColorFromPalette(ilukPal, index, i_lum); // CHSV(i_hue, i_sat_l, i_lum); 
        ringsLeds[i] = blend(ringsLeds[i], col, 64);
      }

      leds[i] = blend(leds[i], ringsLeds[i], mix);
  }

  odd = !odd;

  long t2 = millis();
  Serial.print(t1 - t0);
  Serial.print(",");
  Serial.println(t2 - t1);

}

void test()
{
  long t = millis() / 10;

  for (int i = 0; i < NUM_LEDS; i++){
    float x = mapX[i];
    float y = mapY[i];
    if (x > -1000) {
      if (x > t % 1000 - 300) {
        if (y > t % 600 - 300)
          leds[i] = CRGB::Red;
        else
          leds[i] = CRGB::Purple;
        } else {
          if (y > t % 600 - 300)
            leds[i] = CRGB::Green;
          else
            leds[i] = CRGB::Blue;
        }
      } else {
        leds[i] = CRGB::Green;
      }
  }
}

void roll(uint8_t mix)
{
  long t = millis() / 10;

  for (int i = 0; i < NUM_LEDS; i++){
      if (mapX[i] > -1000){
        float x = mapX[i];
        float y = mapY[i];

        uint8_t hueIdx = (mapTheta[i] + PI) / (2 * PI) * 255 + (t/2);
        hueIdx = abs(255 - 2 * hueIdx);
        // 400 to always leave a bit of colour in
        uint8_t val = (1 - mapRho[i] / 600) * 255;
        CRGB col = ColorFromPalette(ilukPal, hueIdx, val); 
        leds[i] = blend(leds[i], col, mix);
      }
  }
}

void zoom(uint8_t mix)
{
  long t = millis() / 10;

  for (int i = 0; i < NUM_LEDS; i++){
      if (mapX[i] > -1000){

        uint8_t hueIdx = (mapRho[i] / 300) * 255 + t / 8;
        hueIdx = abs(255 - 2 * hueIdx);
        // 400 to always leave a bit of colour in
        // uint8_t val = (1 - mapRho[i] / 600) * 255;
        CRGB col = ColorFromPalette(ilukPal, hueIdx, 255);
        leds[i] = blend(leds[i], col, mix);
      }
  }
}

void setup() {
  delay(1000); // 1 second delay for recovery

  Serial.begin(9600);

  LEDS.addLeds<WS2811_PORTD,NUM_STRIPS>(leds, NUM_LEDS_PER_STRIP);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  fill_solid(leds, NUM_STRIPS * NUM_LEDS_PER_STRIP, CRGB::Black);
  FastLED.show();
  delay(500);

  initXYMap();
  test();
  FastLED.show();
  delay(500);
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])(uint8_t);
SimplePatternList gPatterns = { zoom, rings, roll };

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
  gNextPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns); 
}

long lastT = 0;

void loop()
{

  long t = millis();
  Serial.println(t - lastT);
  lastT = t;
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber](255);
  gPatterns[gNextPatternNumber](gCurrentMix);

  // send the 'leds' array out to the actual LED strip
  // make sure inner ring is not too bright;
  // for (int i = 0; i < 24; i++){
  //   leds[i].fadeLightBy(128);
  // }
  FastLED.show();
  // insert a delay to keep the framerate modest
  // FastLED.delay(1000/FRAMES_PER_SECOND);

  // do some periodic updates
  EVERY_N_SECONDS( 120 ) { 
      nextPattern();
      gCurrentMix = 0;
  } // change patterns periodically
  EVERY_N_MILLIS( 20 ) { 
    gCurrentMix = constrain( gCurrentMix + 1, 0, 255); 
  }
  
}
