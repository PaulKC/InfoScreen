#include "screen.h"
#include <Arduino.h>

const float weather_display_height = 76;

EinkScreen::EinkScreen() : display(GxEPD2_420(/*CS=D8*/ D2, /*DC=D3*/ D3, /*RST=D4*/ D7, /*BUSY=D2*/ D6))
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

void EinkScreen::printWeatherData(WeatherInfo weatherInfo,int error)
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
    if(error == 0)
    {
        display.print(tempString);
    }
    else
    {
        char buffer [2];
        sprintf(buffer,"E%d",error);
        display.print(buffer);
    }
    display.drawLine(0, 2 * tbh_temp_val + 2 * tbh_temp_desc, displayWidth, 2 * tbh_temp_val + 2 * tbh_temp_desc, GxEPD_BLACK);
    display.drawLine(displayWidth / 2, 0, displayWidth / 2, 2 * tbh_temp_val + 2 * tbh_temp_desc, GxEPD_BLACK);
    if(error==0)
    {
        if (weatherInfo.sunshine > 0 && weatherInfo.rain == 0)
        {
            display.drawBitmap(3 * displayWidth / 4 - 76 / 2, 0, sun, weather_display_height, weather_display_height, GxEPD_BLACK);
        }
        else if (weatherInfo.sunshine == 0 && weatherInfo.rain == 0)
        {
            display.drawBitmap(3 * displayWidth / 4 - 95 / 2, 0, cloud, 95, weather_display_height, GxEPD_BLACK);
        }
        else if (weatherInfo.sunshine == 0 && weatherInfo.rain > 0)
        {
            display.drawBitmap(3 * displayWidth / 4 - 76 / 2, 0, rain, weather_display_height, weather_display_height, GxEPD_BLACK);
        }
        else if (weatherInfo.sunshine > 0 && weatherInfo.rain > 0)
        {
            display.drawBitmap(3 * displayWidth / 4 - 76 / 2, 0, mix, weather_display_height, weather_display_height, GxEPD_BLACK);
        }
    }
    //TODO does not work during the night
}

void EinkScreen::printCalendarData(const char events[], int error)
{
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    //Get height of one Line
    display.setFont(&FreeMonoBold18pt7b);
    display.getTextBounds("T", 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t x = 0;
    uint16_t y = weather_display_height+(tbh/2) + tbh+5;
    display.setCursor(x,y);
    
    if(error==0)
    {
        display.print(events);
    }
    else
    {
        char buffer [2];
        sprintf(buffer,"E%d",error);
        display.print(buffer);
    }
}