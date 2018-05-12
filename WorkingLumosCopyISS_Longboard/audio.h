/*
   ESP8266 + FastLED + IR Remote: https://github.com/jasoncoon/esp8266-fastled-audio
   Copyright (C) 2015-2016 Jason Coon
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Portions of this file are adapted from the work of Stefan Petrick:
// https://plus.google.com/u/0/115124694226931502095

// Portions of this file are adapted from RGB Shades Audio Demo Code by Garrett Mace:
// https://github.com/macetech/RGBShadesAudio

// Pin definitions
#define MSGEQ7_STROBE_PIN 26 
#define MSGEQ7_RESET_PIN  25
#define ADC1_TEST_CHANNEL (ADC1_CHANNEL_6)      // GPIO 34

#define AUDIODELAY 0

// Smooth/average settings
#define SPECTRUMSMOOTH 0.08
#define PEAKDECAY 0.01
#define NOISEFLOOR 65

// AGC settings
#define AGCSMOOTH 0.004
#define GAINUPPERLIMIT 15.0
#define GAINLOWERLIMIT 0.1

// Global variables
unsigned int spectrumValue[7];  // holds raw adc values
float spectrumDecay[7] = {0};   // holds time-averaged values
float spectrumPeaks[7] = {0};   // holds peak values
float audioAvg = 270.0;
float gainAGC = 0.0;
int read_raw;
uint8_t spectrumByte[7];        // holds 8-bit adjusted adc values

uint8_t spectrumAvg;

unsigned long currentMillis; // store current loop's millis value
unsigned long audioMillis; // store time of last audio update

void initializeAudio() {
 
  pinMode(MSGEQ7_RESET_PIN, OUTPUT);
  pinMode(MSGEQ7_STROBE_PIN, OUTPUT);

  digitalWrite(MSGEQ7_RESET_PIN, LOW);
  digitalWrite(MSGEQ7_STROBE_PIN, HIGH);
}

void readAudio() {
  static PROGMEM const byte spectrumFactors[7] = {9, 11, 13, 13, 12, 12, 13};
  
    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(ADC1_TEST_CHANNEL, ADC_ATTEN_11db);
  // reset MSGEQ7 to first frequency bin
  digitalWrite(MSGEQ7_RESET_PIN, HIGH);
  delayMicroseconds(5);
  digitalWrite(MSGEQ7_RESET_PIN, LOW);

  // store sum of values for AGC
  int analogsum = 0;

  // cycle through each MSGEQ7 bin and read the analog values
  for (int i = 0; i < 7; i++) {

    // set up the MSGEQ7
    digitalWrite(MSGEQ7_STROBE_PIN, LOW);
    delayMicroseconds(50); // to allow the output to settle

    // read the analog value
    
    spectrumValue[i] = read_raw;
    digitalWrite(MSGEQ7_STROBE_PIN, HIGH);

    // noise floor filter
    if (spectrumValue[i] < NOISEFLOOR) {
      spectrumValue[i] = 0;
    } else {
      spectrumValue[i] -= NOISEFLOOR;
    }

    // apply correction factor per frequency bin
    spectrumValue[i] = (spectrumValue[i] * pgm_read_byte_near(spectrumFactors + i)) / 10;

    // prepare average for AGC
    analogsum += spectrumValue[i];

    // apply current gain value
    spectrumValue[i] *= gainAGC;

    // process time-averaged values
    spectrumDecay[i] = (1.0 - SPECTRUMSMOOTH) * spectrumDecay[i] + SPECTRUMSMOOTH * spectrumValue[i];

    // process peak values
    if (spectrumPeaks[i] < spectrumDecay[i]) spectrumPeaks[i] = spectrumDecay[i];
    spectrumPeaks[i] = spectrumPeaks[i] * (1.0 - PEAKDECAY);

    spectrumByte[i] = spectrumValue[i] / 4;
  }

  // Calculate audio levels for automatic gain
  audioAvg = (1.0 - AGCSMOOTH) * audioAvg + AGCSMOOTH * (analogsum / 7.0);

  spectrumAvg = (analogsum / 7.0) / 4;

  // Calculate gain adjustment factor
  gainAGC = 270.0 / audioAvg;
  if (gainAGC > GAINUPPERLIMIT) gainAGC = GAINUPPERLIMIT;
  if (gainAGC < GAINLOWERLIMIT) gainAGC = GAINLOWERLIMIT;
}

// Attempt at beat detection
byte beatTriggered = 0;
#define beatLevel 20.0
#define beatDeadzone 30.0
#define beatDelay 50
float lastBeatVal = 0;
byte beatDetect() {
  static float beatAvg = 0;
  static unsigned long lastBeatMillis;
  float specCombo = (spectrumDecay[0] + spectrumDecay[1]) / 2.0;
  beatAvg = (1.0 - AGCSMOOTH) * beatAvg + AGCSMOOTH * specCombo;

  if (lastBeatVal < beatAvg) lastBeatVal = beatAvg;
  if ((specCombo - beatAvg) > beatLevel && beatTriggered == 0 && currentMillis - lastBeatMillis > beatDelay) {
    beatTriggered = 1;
    lastBeatVal = specCombo;
    lastBeatMillis = currentMillis;
    return 1;
  } else if ((lastBeatVal - specCombo) > beatDeadzone) {
    beatTriggered = 0;
    return 0;
  } else {
    return 0;
  }
}

void fade_down(uint8_t value) {
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i].fadeToBlackBy(value);
  }
}

void spectrumWaves()
{
  fade_down(2);

  CRGB color = CRGB(spectrumByte[6], spectrumByte[5] / 8, spectrumByte[1] / 2);

  leds[CENTER_LED] = color;
  leds[CENTER_LED].fadeToBlackBy(spectrumByte[3] / 12);

  leds[CENTER_LED - 1] = color;
  leds[CENTER_LED - 1].fadeToBlackBy(spectrumByte[3] / 12);

  //move to the left
  for (int i = NUM_LEDS - 1; i > CENTER_LED; i--) {
    leds[i] = leds[i - 1];
  }
  // move to the right
  for (int i = 0; i < CENTER_LED; i++) {
    leds[i] = leds[i + 1];
  }
}

void spectrumWaves2()
{
  fade_down(2);

  CRGB color = CRGB(spectrumByte[5] / 8, spectrumByte[6], spectrumByte[1] / 2);

  leds[CENTER_LED] = color;
  leds[CENTER_LED].fadeToBlackBy(spectrumByte[3] / 12);

  leds[CENTER_LED - 1] = color;
  leds[CENTER_LED - 1].fadeToBlackBy(spectrumByte[3] / 12);

  //move to the left
  for (int i = NUM_LEDS - 1; i > CENTER_LED; i--) {
    leds[i] = leds[i - 1];
  }
  // move to the right
  for (int i = 0; i < CENTER_LED; i++) {
    leds[i] = leds[i + 1];
  }
}

void spectrumWaves3()
{
  fade_down(2);

  CRGB color = CRGB(spectrumByte[1] / 2, spectrumByte[5] / 8, spectrumByte[6]);

  leds[CENTER_LED] = color;
  leds[CENTER_LED].fadeToBlackBy(spectrumByte[3] / 12);

  leds[CENTER_LED - 1] = color;
  leds[CENTER_LED - 1].fadeToBlackBy(spectrumByte[3] / 12);

  //move to the left
  for (int i = NUM_LEDS - 1; i > CENTER_LED; i--) {
    leds[i] = leds[i - 1];
  }
  // move to the right
  for (int i = 0; i < CENTER_LED; i++) {
    leds[i] = leds[i + 1];
  }
}

void analyzerColumns()
{
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  const uint8_t columnSize = NUM_LEDS / 7;

  for (uint8_t i = 0; i < 7; i++) {
    uint8_t columnStart = i * columnSize;
    uint8_t columnEnd = columnStart + columnSize;

    if (columnEnd >= NUM_LEDS) columnEnd = NUM_LEDS - 1;

    uint8_t columnHeight = map8(spectrumByte[i], 1, columnSize);

    for (uint8_t j = columnStart; j < columnStart + columnHeight; j++) {
      if (j >= NUM_LEDS || j >= columnEnd)
        continue;

      leds[j] = CHSV(i * 40, 255, 255);
    }
  }
}

void analyzerPeakColumns()
{
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  const uint8_t columnSize = NUM_LEDS / 7;

  for (uint8_t i = 0; i < 7; i++) {
    uint8_t columnStart = i * columnSize;
    uint8_t columnEnd = columnStart + columnSize;

    if (columnEnd >= NUM_LEDS) columnEnd = NUM_LEDS - 1;

    uint8_t columnHeight = map(spectrumValue[i], 0, 1023, 0, columnSize);
    uint8_t peakHeight = map(spectrumPeaks[i], 0, 1023, 0, columnSize);

    for (uint8_t j = columnStart; j < columnStart + columnHeight; j++) {
      if (j < NUM_LEDS && j <= columnEnd) {
        leds[j] = CHSV(i * 40, 255, 128);
      }
    }

    uint8_t k = columnStart + peakHeight;
    if (k < NUM_LEDS && k <= columnEnd)
      leds[k] = CHSV(i * 40, 255, 255);
  }
}

void beatWaves()
{
  fade_down(2);

  if (beatDetect()) {
    leds[CENTER_LED] = CRGB::Red;
  }

  //move to the left
  for (int i = NUM_LEDS - 1; i > CENTER_LED; i--) {
    leds[i] = leds[i - 1];
  }
  // move to the right
  for (int i = 0; i < CENTER_LED; i++) {
    leds[i] = leds[i + 1];
  }
}



void print_audio() {
  for (int band = 0; band < 7; band++) {
    Serial.print(spectrumByte[band]);
    Serial.print("\t");
  }
  Serial.println();
}


void radiate() {

  //HALF_POS = beatsin8(40, 30 + SPEED, 80 + SPEED);
  int MILLISECONDS  = 0;
  //  EVERY_N_MILLISECONDS(50) {
  //    hue++;
  //    if ( hue > 255) hue = 0;
  //  }
uint8_t zero_l, three_l, six_l, zero_r, three_r, six_r;

  zero_l  = spectrumByte[0];
  three_l = spectrumByte[3];
  six_l   = spectrumByte[6];

  zero_r  = spectrumByte[0];
  three_r = spectrumByte[3];
  six_r   = spectrumByte[6];

  leds[CENTER_LED] = CRGB(zero_l, three_l, six_l);
  leds[CENTER_LED + 1] = CRGB(zero_r, three_r, six_r);
  //leds[HALF_POS].fadeToBlackBy(30);


  EVERY_N_MILLISECONDS(11) {
    for (int i = NUM_LEDS - 1; i > CENTER_LED + 1; i--) {
      leds[i].blue = leds[i - 1].blue;
    }
    for (int i = 0; i < CENTER_LED; i++) {
      leds[i].blue = leds[i + 1].blue;
    }
  }
  EVERY_N_MILLISECONDS(27) {
    for (int i = NUM_LEDS - 1; i > CENTER_LED + 1; i--) {
      leds[i].green = leds[i - 1].green;
    }
    for (int i = 0; i < CENTER_LED; i++) {
      leds[i].green = leds[i + 1].green;
    }
  }
  EVERY_N_MILLISECONDS(52) {
    for (int i = NUM_LEDS - 1; i > CENTER_LED + 1; i--) {
      leds[i].red = leds[i - 1].red;
    }
    for (int i = 0; i < CENTER_LED; i++) {
      leds[i].red = leds[i + 1].red;
    }
  }

  EVERY_N_MILLISECONDS(2) {
    //blur1d(leds, NUM_LEDS, 1);
  }
}

void flex_mono() {
uint8_t left_pos, right_pos, left_point, right_point;
uint8_t current_hue, current_brightness, next_hue, next_brightness, pos, point;
uint8_t left_current_brightness, left_next_brightness, right_current_brightness, right_next_brightness;
float fhue;
  left_pos    = CENTER_LED;
  left_point  = CENTER_LED;
  right_pos   = CENTER_LED;
  right_point = CENTER_LED;

  int MILLISECONDS = 10;
  int spectrumWidth = 32;

  //fhue += mono[0] * .05;
  //  EVERY_N_MILLISECONDS(200){
  //    fhue++;
  //  }

  for (int band = 0; band < 7; band++) {

    left_current_brightness = map(spectrumByte[band], 0, 255, 0, 255);
    right_current_brightness = left_current_brightness;

    if (band < 6) {
      left_next_brightness  = map(spectrumByte[band + 1], 0, 255, 0, 255);
      right_next_brightness = left_next_brightness;
    } else {
      left_next_brightness  = 0;
      right_next_brightness = 0;
    }

    current_hue = fhue + (band * spectrumWidth);
    next_hue = fhue + ((band + 1) * spectrumWidth);

    left_point -=  spectrumByte[band];
    right_point += spectrumByte[band];


    //if (band == 6) (left_point = 0) && (right_point = NUM_LEDS - 1) && (next_hue = band * 35) /*&& (left_next_brightness = 0) && (right_next_brightness = 0)*/;

    fill_gradient(leds, left_pos, CHSV(current_hue, 255, left_current_brightness), left_point, CHSV(next_hue, 255, right_next_brightness), SHORTEST_HUES);
    fill_gradient(leds, right_pos, CHSV(current_hue, 255, right_current_brightness), right_point, CHSV(next_hue, 255, right_next_brightness), SHORTEST_HUES);

    //fill_gradient(leds, left_pos, ColorFromPalette(gCurrentPalette, current_hue, left_current_brightness, LINEARBLEND), left_point, ColorFromPalette(gCurrentPalette, next_hue, right_next_brightness, LINEARBLEND), SHORTEST_HUES);
    //fill_gradient(leds, right_pos, ColorFromPalette(gCurrentPalette, current_hue, right_current_brightness, LINEARBLEND), right_point, ColorFromPalette(gCurrentPalette, next_hue, right_next_brightness, LINEARBLEND), SHORTEST_HUES);

    //current_hue += 12;
    //next_hue += 12;

    left_pos  = left_point;
    right_pos = right_point;
  }

  //fadeToBlackBy(leds,NUM_LEDS,1);
  blur1d(leds, NUM_LEDS, 130);
  //nblend(leds, NUM_LEDS, 130);
}

void rain() {
  uint8_t hue;

  // increase raindrop hue value - there are other ways to color the rain, though
  EVERY_N_MILLISECONDS(200){ hue++; }

  // alternatively, pick a random color from a color palette that changes over time
  //changePalette();
  //INDEX = random(10, 200);
  // could be cool to instead oscillation around certain areas of a color palette using beatsin8

  // TODO far more natural feeling rain rhythm and placement
  // using EVERY_N_MILLISECONDS is too metronomic for this function
  // maybe somthing like multiplying a value from beatsin8 with a sin wave,
  // first attempt was to feed EVERY_N_MILLISECONDS with an oscillating value..
  //MILLISECONDS = beatsin8(50,20,400);
  // does not work well, have not yet bedugged

  // TODO in addition to natural rhythm, consider only raining in a smaller portion of the strip,
  // and moving that portion around,
  // and occasionally briefly expanding it
  // to simulate an overall more random feel to the raindrop placement

  // together, the natural feel of rhythm, position, color (consider random saturation values?),
  // "rain" could be a nice compliment to the feel of Mark Kriegsman's "Five Elements" light sculpture
  // https://youtu.be/knWiGsmgycY

  // draw a "raindrop" of random width every 300 milliseconds
  EVERY_N_MILLISECONDS(300) {

    int pos = random16(NUM_LEDS - 1);
    int left_pos  = pos - random(2, 15);
    int right_pos = pos + random(2, 15);

    if (left_pos < 0) { // not exactly a wraparound, but prevent values from extending beyond
      left_pos = 0;     // the edges of the strip. faster way to do this?
    }
    if (right_pos > NUM_LEDS - 1) {
      right_pos = NUM_LEDS - 1;
    }

    // instead of drawing this raindrop the exact value of "hue",
    // just add that color to the previous color. makes for a nice
    // "pool of color" effect. does require eventually fading to black,
    // or the entire strip will quickly become white
    for (int i = left_pos; i < right_pos; i++) {
      leds[i] += CHSV(hue, 255, 200);
      // leds[i] += ColorFromPalette(gCurrentPalette, INDEX, 255, LINEARBLEND);
    }
    // TODO consider result of drawing only in one portion of the strip,
    // or drawing at a slower/faster rate as described above.
    // these fluctuations will require adjusting fadeToBlack.
    // drawing very slowly, with a slow fade to black could look very nice

    // same issues apply to blur1d, which is used to blur the edges of drops into
    // one another, or into the black around each drop
  }

  EVERY_N_MILLISECONDS(30) {
    fadeToBlackBy(leds, NUM_LEDS, 1);
    blur1d(leds, NUM_LEDS, 120);
  }

}


