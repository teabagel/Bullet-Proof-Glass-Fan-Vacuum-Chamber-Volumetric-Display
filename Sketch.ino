/*
*This sketch outputs images to persistence of vision led strips
*It uses FastLed to drive APA102 leds, sending colour values from
*arrays held in flash memory (designated by 'const'). You need to
*set the number of slices you have made your image into, 
*e.g. bmp image of 60 pixels high by 150 wide
* would give 60 num_leds and 
* 150 slices (number of slices you have made your image into) 
*/

#include "FastLED.h"

#define NUM_LEDS 100 //number of leds in strip length on one side
#define DATA_PIN 2//7 = second hardware spi data
#define CLOCK_PIN 3//14 = second hardware spi clock
CRGB leds[NUM_LEDS];
int numberOfSlices = 150;

void setup() {

  delay(200);
    FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN>(leds, NUM_LEDS);
 }

const unsigned int array0[] = {0x80d06, 0x100,  0x0, 0x0, 0x10000, 0x20000, 0x10000, 0x4, 0x102, 0x100, 0x10001, 0x20801, 0x15370b, 0x468e31, 0x408a27, 0xd2405, 0x300, 0x0, 0x0, 0x0, 0x0
};//holly
const unsigned int array0[] = {0x0, 0x0, 0x0, 0x0, , 0x0, 0x0, 0x10100, 0x40000, 0x20002, 0x4, 0…….
};//snow
