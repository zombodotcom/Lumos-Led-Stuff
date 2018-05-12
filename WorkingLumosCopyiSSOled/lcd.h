#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" //https://github.com/squix78/esp8266-oled-ssd1306"`

SSD1306  display(0x3c, 5, 4);

void initDisplay() {
  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

}

void drawHello() {
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Lumos Board v0.9");
  display.drawString(0, 11, "zombodotcom labs");
  display.drawString(0, 26, "Pattern: " + pattern);
  String brightDisp = String(BRIGHTNESS);
  display.drawString(0, 52, "Brightness: " + brightDisp);


}

void drawLorem() {
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawStringMaxWidth(0, 0, 128,
                             "Lorem ipsum\n dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore." );
}


void drawUI() {


  display.drawString(10, 128, "Lumos Board");


  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 22, pattern);
}




