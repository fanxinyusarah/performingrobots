#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h>
#endif

#define PIN        6      
#define NUM_PER_RING 16   
#define NUM_FRONT (NUM_PER_RING * 2)
#define NUM_BACK 24 
#define NUMPIXELS (NUM_FRONT + NUM_BACK)

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define DELAYVAL 30           
#define BRIGHTNESS_HIGH 80 
#define BRIGHTNESS_DECAY 15 
#define COLOR_SPEED 300       

int currentPixel = 0;
int direction = 1; 
int cyclePhase = 1; 

int getPathPos(int pixel) {
  if (pixel < NUM_PER_RING) {
    return pixel;                       
  } else if (pixel < NUM_FRONT) {
    return NUM_FRONT - 1 - (pixel - NUM_PER_RING);
  } else {
    return 0;
  }
}

uint32_t Wheel(uint16_t pos) {
  pos = 65535 - pos;
  if (pos < 21845) { 
    return pixels.Color(255 - pos * 3 / 256, pos * 3 / 256, 0);
  } else if (pos < 43690) { 
    pos -= 21845;
    return pixels.Color(0, 255 - pos * 3 / 256, pos * 3 / 256);
  } else { 
    pos -= 43690;
    return pixels.Color(pos * 3 / 256, 0, 255 - pos * 3 / 256);
  }
}

uint32_t color_fade(uint32_t color, int decay) {
  uint8_t r = (uint8_t)(color >> 16);
  uint8_t g = (uint8_t)(color >> 8);
  uint8_t b = (uint8_t)color;

  r = (r <= decay) ? 0 : r - decay;
  g = (g <= decay) ? 0 : g - decay;
  b = (b <= decay) ? 0 : b - decay;
  
  return pixels.Color(r, g, b);
}

void setup() {
  pixels.begin();
  pixels.setBrightness(80);  
  pixels.clear();
  pixels.show();
}

void loop() {

  for(int i = 0; i < NUM_FRONT; i++) {
    uint32_t oldColor = pixels.getPixelColor(i);
    pixels.setPixelColor(i, color_fade(oldColor, BRIGHTNESS_DECAY));
  }

  int pos = getPathPos(currentPixel);
  uint32_t brightColor = Wheel(((pos * 65535) / NUM_FRONT + millis() / COLOR_SPEED) % 65536);

  uint8_t r = (uint8_t)(brightColor >> 16);
  uint8_t g = (uint8_t)(brightColor >> 8);
  uint8_t b = (uint8_t)brightColor;

  r = (r * BRIGHTNESS_HIGH) / 255;
  g = (g * BRIGHTNESS_HIGH) / 255;
  b = (b * BRIGHTNESS_HIGH) / 255;

  pixels.setPixelColor(currentPixel, r, g, b);

  for (int i = NUM_FRONT; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(100, 100, 100)); 
  }

  pixels.show();
  delay(DELAYVAL);

  currentPixel += direction; 

  if (cyclePhase == 1) {
    if (currentPixel >= NUM_PER_RING) {
      currentPixel = NUM_FRONT - 1;
      direction = -1;
      cyclePhase = 2;
    }
  } else if (cyclePhase == 2) {
    if (currentPixel < NUM_PER_RING) {
      currentPixel = 0;
      direction = 1;
      cyclePhase = 1;
    }
  }
}
