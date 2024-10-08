// TFT_eSPI_memory
//
// Example sketch which shows how to display an
// animated GIF image stored in FLASH memory
//
// written by Larry Bank
// bitbank@pobox.com
//
// Adapted by Bodmer for the TFT_eSPI Arduino library:
// https://github.com/Bodmer/TFT_eSPI
//
// To display a GIF from memory, a single callback function
// must be provided - GIFDRAW
// This function is called after each scan line is decoded
// and is passed the 8-bit pixels, RGB565 palette and info
// about how and where to display the line. The palette entries
// can be in little-endian or big-endian order; this is specified
// in the begin() method.
//
// The AnimatedGIF class doesn't allocate or free any memory, but the
// instance data occupies about 22.5K of RAM.

// Modified by Stephen Kraus (skraus86) to add sleep timer and execute esp32 deep sleep

//#define USE_DMA       // ESP32 ~1.25x single frame rendering performance boost for badgers.h
                        // Note: Do not use SPI DMA if reading GIF images from SPI SD card on same bus as TFT
  #define NORMAL_SPEED  // Comment out for rame rate for render speed test
// Sleepmode Value Established
unsigned long previousMillis = 0;
long interval = 15000; // 15 seconds
// Load GIF library
#include <AnimatedGIF.h>
AnimatedGIF gif;

// Example AnimatedGIF library images
#include "out.h"


                                // ESP32 40MHz SPI single frame rendering performance
                                // Note: no DMA performance gain on smaller images or transparent pixel GIFs
#define GIF_IMAGE blinky   //  No DMA  63 fps, DMA:  71fps


#include <SPI.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);

  tft.begin();
#ifdef USE_DMA
  tft.initDMA();
#endif
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  gif.begin(BIG_ENDIAN_PIXELS);
}

#ifdef NORMAL_SPEED // Render at rate that is GIF controlled
void loop()
{
  unsigned long currentMillis = millis(); //Timer Start
  // Put your main code here, to run repeatedly:
  if (gif.open((uint8_t *)GIF_IMAGE, sizeof(GIF_IMAGE), GIFDraw))
  {
    Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", gif.getCanvasWidth(), gif.getCanvasHeight());
    tft.startWrite(); // The TFT chip slect is locked low
    while (gif.playFrame(true, NULL))
    {
      yield();
    }
    gif.close();
    tft.endWrite();  // Release TFT chip select for other SPI devices
    if(currentMillis - previousMillis >= interval){ //Sleep Mode Timer
      esp_deep_sleep_start(); 
    } else {} //Continue if timer value not met
  }
}
#else // Test maximum rendering speed
void loop()
{
  long lTime = micros();
  int iFrames = 0;

  if (gif.open((uint8_t *)GIF_IMAGE, sizeof(GIF_IMAGE), GIFDraw))
  {
    tft.startWrite(); // For DMA the TFT chip slect is locked low
    while (gif.playFrame(false, NULL))
    {
      // Each loop renders one frame
      iFrames++;
      yield();
    }
    gif.close();
    tft.endWrite(); // Release TFT chip select for other SPI devices
    lTime = micros() - lTime;
    Serial.print(iFrames / (lTime / 1000000.0));
    Serial.println(" fps");
  }
}
#endif
