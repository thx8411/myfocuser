
#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, 4);

class Logger {
    public :
        void begin() {
            if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
                Serial.println(F("SSD1306 allocation failed"));
                for(;;); // Don't proceed, loop forever
            }
            // Clear the buffer
            display.clearDisplay();
            display.setTextSize(2);
            display.setTextColor(SSD1306_WHITE);
        }

        void write(String s) {
            display.clearDisplay();
            display.setCursor(0, 0);     // Start at top-left corner

            for(int i=0; i<s.length(); i++) {
                display.write(s.charAt(i));
            }
            display.display();
        }

        void write(int i) {
            String s(i);
            write(s);
        }

} logger;

#endif
