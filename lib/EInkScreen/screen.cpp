#include "screen.h"
#include <Arduino.h>

EinkScreen::EinkScreen() : display(GxEPD2_420(/*CS=D8*/ 5, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 4))
{
    display.init(115200);
    display.setRotation(3);
    display.setTextColor(GxEPD_BLACK);
    displayWidth = display.width();
    display.setFullWindow();
    display.setTextSize(1);
}

void EinkScreen::powerOff()
{
    display.powerOff();
}

void EinkScreen::clear()
{
    display.powerOff();
    display.firstPage();
    display.fillScreen(GxEPD_WHITE);
}

void EinkScreen::update()
{
    display.nextPage();
}

void EinkScreen::printWeatherData(WeatherInfo weatherInfo)
{
    int size = 3;
    if (weatherInfo.temp < 0)
    {
        size = 4;
    }
    char tempString[size];
    dtostrf(weatherInfo.temp, size, 1, tempString);
    const char *temperaturDesc = "Temperatur";
    int16_t tbx_temp_desc, tby_temp_desc, tbx_temp_val, tby_temp_val;
    uint16_t tbw_temp_desc, tbh_temp_desc, tbw_temp_val, tbh_temp_val;
    display.setFont(&FreeMonoBold9pt7b);
    display.getTextBounds(temperaturDesc, 0, 0, &tbx_temp_desc, &tby_temp_desc, &tbw_temp_desc, &tbh_temp_desc);
    display.setFont(&FreeMonoBold18pt7b);
    display.getTextBounds(tempString, 0, 0, &tbx_temp_val, &tby_temp_val, &tbw_temp_val, &tbh_temp_val);
    uint16_t x = displayWidth / 4 - tbw_temp_desc / 2;
    uint16_t y = 0 + tbh_temp_desc;
    display.setCursor(x, y);
    display.setFont(&FreeMonoBold9pt7b);
    display.print(temperaturDesc);
    y = y + tbh_temp_desc + tbh_temp_val;
    x = x + tbw_temp_desc / 2 - tbw_temp_val / 2;
    display.setCursor(x, y);
    display.setFont(&FreeMonoBold18pt7b);
    display.print(tempString);
    display.drawLine(0, 2 * tbh_temp_val + 2 * tbh_temp_desc, displayWidth, 2 * tbh_temp_val + 2 * tbh_temp_desc, GxEPD_BLACK);
    display.drawLine(displayWidth / 2, 0, displayWidth / 2, 2 * tbh_temp_val + 2 * tbh_temp_desc, GxEPD_BLACK);
    if (weatherInfo.sunshine > 0 && weatherInfo.rain == 0)
    {
        display.drawBitmap(3 * displayWidth / 4 - 76 / 2, 0, sun, 76, 76, GxEPD_BLACK);
    }
    else if (weatherInfo.sunshine == 0 && weatherInfo.rain == 0)
    {
        display.drawBitmap(3 * displayWidth / 4 - 95 / 2, 0, cloud, 95, 76, GxEPD_BLACK);
    }
    else if (weatherInfo.sunshine == 0 && weatherInfo.rain > 0)
    {
        display.drawBitmap(3 * displayWidth / 4 - 76 / 2, 0, rain, 76, 76, GxEPD_BLACK);
    }
    else if (weatherInfo.sunshine > 0 && weatherInfo.rain > 0)
    {
        display.drawBitmap(3 * displayWidth / 4 - 76 / 2, 0, mix, 76, 76, GxEPD_BLACK);
    }
    //TODO does not work during the night
}

void EinkScreen::printCalendarData(const char events[])
{
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    //Get height of one Line
    display.getTextBounds("T", 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t x = 0;
    uint16_t y = 76 + tbh+5;
    display.setCursor(x,y);
    display.setFont(&FreeMonoBold18pt7b);
    display.print(events);
}