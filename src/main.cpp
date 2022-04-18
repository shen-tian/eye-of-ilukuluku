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
#define NUM_STRIPS 4
#define NUM_LEDS_PER_STRIP 128
#define NUM_LEDS NUM_STRIPS * NUM_LEDS_PER_STRIP
CRGB leds[NUM_STRIPS * NUM_LEDS_PER_STRIP];

float mapX[NUM_LEDS];
float mapY[NUM_LEDS];

#define BRIGHTNESS          255
#define FRAMES_PER_SECOND  120

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
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
            x = 42 + 40 * sin(2 * PI * (j + offset) / ringSize);
            y = 42 + 40 * cos(2 * PI * (j + offset) / ringSize);
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
            x = 270 * sin(2 * PI * (j + offset) / ringSize);
            y = 270 * cos(2 * PI * (j + offset) / ringSize);
          }
          break;
      }
      mapX[index] = x;
      mapY[index] = y;

      if (x > -1000) {
        
        Serial.print(x);
        Serial.print(",");
        Serial.print(y);
        Serial.println();
      }
    }
    }
}

#define NOISE_SCALE 1000000

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

int width = 600;
int height = 600;

float dx, dy, dz;

float noise(float x){
  return ((float)inoise16(x * NOISE_SCALE))/ UINT16_MAX;
}

void rings() 
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
      if (mapX[i] > -1000){
        float x = mapX[i] + 300;
        float y = mapX[i] + 300;

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

}

void rainbow()
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

void box()
{
  long t = millis() / 10;

  for (int i = 0; i < NUM_LEDS; i++){
      if (mapX[i] > -1000){
        leds[i] = CRGB(mapX[i], mapY[i], 0);
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
  rainbow();
  FastLED.show();
  delay(500);
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rings, rainbow, box };
  
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
  // FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
}