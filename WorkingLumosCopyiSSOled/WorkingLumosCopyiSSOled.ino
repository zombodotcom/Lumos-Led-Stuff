#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <TimeLib.h>
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson
//#include "SSD1306.h" //https://github.com/squix78/esp8266-oled-ssd1306

#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>

WiFiMulti wifiMulti;
//Find your Latitude and Longitude here
//https://www.latlong.net/
float mylat = 40.573922;
float mylon = -105.083303;
float isslat, isslon;
int distance, number, count;
String payload;
String name[10], craft[10], risetime[5];
float duration[5];

const String iss = "http://api.open-notify.org/iss-now.json";
const String ppl = "http://api.open-notify.org/astros.json";
String pas = "http://api.open-notify.org/iss-pass.json?";

//SSD1306  display(0x3c, 5, 4);




#include <HTTPClient.h>

#include "FastLED.h"


FASTLED_USING_NAMESPACE

//led stuff
#define DATA_PINLL   12
//#define DATA_PINRR 12
//#define DATA_PINRM   33

//#define CLK_PIN   4
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS   24
CRGB leds[NUM_LEDS];
#define BRIGHTNESS         255
#define FRAMES_PER_SECOND  120
int CENTER_LED = NUM_LEDS/2;
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
String pattern;
String pat;
String sBrightFix;
int brightFixs;
String rxColorFix;
String sColorfix1;
String sColorFix;
uint8_t rgbR=128;
uint8_t rgbG=128;
uint8_t rgbB=128;
CRGB solidColor=CRGB::Blue;
#include "ble.h"
#include "patterns.h"
#include "lcd.h"
#include "audio.h"


#define FASTLED_SHOW_CORE 0

// -- Task handles for use in the notifications
static TaskHandle_t FastLEDshowTaskHandle = 0;
static TaskHandle_t userTaskHandle = 0;

/** show() for ESP32
    Call this function instead of FastLED.show(). It signals core 0 to issue a show,
    then waits for a notification that it is done.
*/
void FastLEDshowESP32()
{
  if (userTaskHandle == 0) {
    // -- Store the handle of the current task, so that the show task can
    //    notify it when it's done
    userTaskHandle = xTaskGetCurrentTaskHandle();

    // -- Trigger the show task
    xTaskNotifyGive(FastLEDshowTaskHandle);

    // -- Wait to be notified that it's done
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS( 200 );
    ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
    userTaskHandle = 0;
  }
}

/** show Task
    This function runs on core 0 and just waits for requests to call FastLED.show()
*/
void FastLEDshowTask(void *pvParameters)
{
  // -- Run forever...
  for (;;) {
    // -- Wait for the trigger
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // -- Do the show (synchronously)
    FastLED.show();

    // -- Notify the calling task
    xTaskNotifyGive(userTaskHandle);
  }
}


void print_chip_info()
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
}

#include "iss.h"
void setup() {
  Serial.begin(115200);
   for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }wifiMulti.addAP("SSID", "Password");
  print_chip_info();
   initDisplay();
  
//  wifiMulti.addAP("SSID", "Password");
//  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  FastLED.addLeds<LED_TYPE,DATA_PINLL,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
//  FastLED.addLeds<LED_TYPE,DATA_PINRR,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
//   FastLED.addLeds<LED_TYPE,DATA_PINRM,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
//  FastLED.addLeds<LED_TYPE, 25, COLOR_ORDER>(leds, NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
//  FastLED.addLeds<LED_TYPE, 26, COLOR_ORDER>(leds, 2 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
//  FastLED.addLeds<LED_TYPE, 27, COLOR_ORDER>(leds, 3 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
//  FastLED.addLeds<LED_TYPE, 32, COLOR_ORDER>(leds, 4 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
//  FastLED.addLeds<LED_TYPE,35, COLOR_ORDER>(leds, 5 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
//  FastLED.addLeds<LED_TYPE, 14, COLOR_ORDER>(leds, 6 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
//  FastLED.addLeds<LED_TYPE, SCL, COLOR_ORDER>(leds, 7 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
 pas = pas + "lat=" + (String)mylat + "&lon=" + (String)mylon;
  
  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  // Create the BLE Device
//  initializeAudio();
 setupBLE();
  Serial.println("Waiting a client connection to notify...");
int core = xPortGetCoreID();
  Serial.print("Main code running on core ");
  Serial.println(core);

  // -- Create the FastLED show task
  xTaskCreatePinnedToCore(FastLEDshowTask, "FastLEDshowTask", 2048, NULL, 2, &FastLEDshowTaskHandle, FASTLED_SHOW_CORE);
}

void loop() {
  EVERY_N_MILLISECONDS( 20 ) {
    gHue++;  // slowly cycle the "base color" through the rainbow
  }
  
  // clear the display
  display.clear();
  // draw the current demo method

   drawHello();
  // write the buffer to the display
  display.display();
  if (deviceConnected) {
    
    
 readAudio();
  
    // Fabricate some arbitrary junk for now...
    if (pattern == "rainbow") {
//      FastLED.show();
      fill_rainbow( leds, NUM_LEDS, gHue, 7);

    }
    if (pattern=="printaudio"){
      
       print_audio();
    }
     if (pattern=="radiate"){
      
       radiate();
    }
     if (pattern=="flexmono"){
      
       flex_mono();
    }

      if (pattern=="spectrumWaves"){
      
      spectrumWaves();
    }
    if (pattern == "off") {
//      FastLED.show();
      fill_solid(leds, NUM_LEDS, CRGB::Black);
    }


    if (pattern == "sinelon") {
//      FastLED.show();
      sinelon();
    }
    if (pattern == "bpm") {
//      FastLED.show();
      bpm();
    }
    if (pattern == "juggle") {
//      FastLED.show();
      juggle();
    }

    if (pattern == "pride") {
//      FastLED.show();
      pride();
    }
if (pattern == "color") {
      FastLED.show();
      fill_solid(leds, NUM_LEDS, CRGB(rgbR,rgbG,rgbB));
    }





  }
  
  if (!deviceConnected) {
iss2();
  }
  
FastLEDshowESP32();
}


