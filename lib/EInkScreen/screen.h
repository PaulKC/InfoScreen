#ifndef EINKSCREEN_H
#define EINKSCREEN_H

#include "infoData.h"
#define ENABLE_GxEPD2_GFX 0
#include "images.h"
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>

class EinkScreen{
    private:
        GxEPD2_BW<GxEPD2_420, GxEPD2_420::HEIGHT> display;
        int16_t displayWidth;
    public:
        EinkScreen();
        void printWeatherData(WeatherInfo, int error);
        void printCalendarData(const char[], int error);
        void drawBatteryIcon(float batteryVoltage);
        void powerOff();
        void clear();
        void update();
};
#endif /* EINKSCREEN_H */