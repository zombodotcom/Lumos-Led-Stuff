#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "StringSplitter.h"
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
float txValue = 0;

void setSolidColor(uint8_t x, uint8_t y, uint8_t z, String sColorFix)
{
  StringSplitter *splitter = new StringSplitter(sColorFix, ',', 3);
  int itemCount = splitter->getItemCount();
  Serial.println(itemCount);
  int rgb[itemCount];

  for (int i = 0; i < itemCount; i++) {
    String item = splitter->getItemAtIndex(i);
    rgb[i] = atoi(item.c_str());
    Serial.println(rgb[i]);
  }
  rgbR=rgb[0];
  rgbG=rgb[1];
  rgbB=rgb[2];
}
void brightFix(String value) {
  int bFix1;
   Serial.println(bFix1);
  value.remove(0, 11);
  
  bFix1 = value.toInt();
  Serial.println(bFix1);
  FastLED.setBrightness(bFix1);
}
String setPattern(String pat) {
  pattern = pat;
  return pattern;
}

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;


    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      
      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");

        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }

        Serial.println();
        Serial.println("*********");
      }
      // Do stuff based on the command received from the app
      // For some reason using rxValue.compare("A") == 0 doesn't work. Maybe
      // there are hidden characters I'm not seeing?
      if (rxValue.find("rainbow") != -1) {
        Serial.println("Turning ON!");
        pat = "rainbow";
        setPattern(pat);

      }

        if (rxValue.find("brightness") != -1) {
      sBrightFix = (rxValue.c_str());
      brightFix(sBrightFix);
    }
    if (rxValue.find("spectrumWaves") != -1) {
      Serial.println("Turning ON!");
      pat = "spectrumWaves";
      setPattern(pat);

    }
     if (rxValue.find("flex_mono") != -1) {
      Serial.println("Turning ON!");
      pat = "flex_mono";
      setPattern(pat);

    }

    if (rxValue.find("radiate") != -1) {
      Serial.println("Turning ON!");
      pat = "radiate";
      setPattern(pat);

    }
//    if (rxValue.find("battery") != -1) {
//  
////     #define VBATPIN A9
////   
////    float measuredvbat = analogRead(VBATPIN);
////    measuredvbat *= 2;    // we divided by 2, so multiply back
////    measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
////    measuredvbat /= 1024; // convert to voltage
////    Serial.print("VBat: " ); Serial.println(measuredvbat);
////    
////      Serial.println("Turning ON!");
////      pat = "battery";
////      
////      setPattern(pat);
//
//    }

    
     if (rxValue.find("printaudio") != -1) {
      Serial.println("Turning ON!");
        pat = "printaudio";
      setPattern(pat);

    }
    
      if (rxValue.find("juggle") != -1) {
        Serial.println("Turning ON!");
        pat = "juggle";
        setPattern(pat);

      }
      if (rxValue.find("sinelon") != -1) {
        Serial.println("Turning ON!");
        pat = "sinelon";
        setPattern(pat);

      }
      if (rxValue.find("bpm") != -1) {
        Serial.println("Turning ON!");
        pat = "bpm";
        setPattern(pat);

      }

      if (rxValue.find("pride") != -1) {
        Serial.println("Turning ON!");
        pat = "pride";
        setPattern(pat);

      }

     if (rxValue.find("color") != -1) {
      sColorFix = (rxValue.c_str());
      sColorFix.trim();
      sColorFix.replace("color,", "");
      sColorFix.replace(" ", "");
      setSolidColor(rgbR,rgbG,rgbB,sColorFix);
      Serial.println(rgbR);
      pat = "color";
      setPattern(pat);
    }

      else if (rxValue.find("off") != -1) {
      Serial.println("Turning OFF!");
        pat = "off";
        setPattern(pat);
      }


    }
};

void setupBLE(){
   BLEDevice::init("LOONG LOONG BOOARD"); // Give it a name

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
}


