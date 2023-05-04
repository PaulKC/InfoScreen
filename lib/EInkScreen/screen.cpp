#include "screen.h"
#include <Arduino.h>

const float weather_display_height = 76;
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

EinkScreen::EinkScreen() : display(GxEPD2_420(/*CS=D8*/ D2, /*DC=D3*/ D3, /*RST=D4*/ D7, /*BUSY=D2*/ D6))
{
    display.init(115200);
    u8g2Fonts.begin(display);
    display.setRotation(3);
    u8g2Fonts.setForegroundColor(GxEPD_BLACK); // apply Adafruit GFX color
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE); // apply Adafruit GFX color
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

void EinkScreen::printWeatherData(WeatherInfo weatherInfo, int error)
{
    int size = 3;
    if (weatherInfo.temp < 0)
    {
        size = 4;
    }
    char tempString[size];
    dtostrf(weatherInfo.temp, size, 1, tempString);
    const char *temperaturDesc = "Temperatur";
    uint16_t width_temp_desc, height_temp_desc, width_temp_val, height_temp_val;
    u8g2Fonts.setFont(u8g2_font_helvB10_tf);
    width_temp_desc = u8g2Fonts.getUTF8Width(temperaturDesc);
    height_temp_desc = u8g2Fonts.getFontAscent() - u8g2Fonts.getFontDescent();
    u8g2Fonts.setFont(u8g2_font_helvB24_tf);
    width_temp_val = u8g2Fonts.getUTF8Width(tempString);
    height_temp_val = u8g2Fonts.getFontAscent() - u8g2Fonts.getFontDescent();
    uint16_t x = displayWidth / 4 - width_temp_desc / 2;
    uint16_t y = height_temp_desc;
    u8g2Fonts.setCursor(x, y);
    u8g2Fonts.setFont(u8g2_font_helvB10_tf);
    u8g2Fonts.print(temperaturDesc);
    y = (weather_display_height - 2 * height_temp_desc) / 2 + height_temp_val;
    x = displayWidth / 4 - width_temp_val / 2;
    u8g2Fonts.setCursor(x, y);
    u8g2Fonts.setFont(u8g2_font_helvB24_tf);
    if (error == 0)
    {
        u8g2Fonts.print(tempString);
    }
    else
    {
        char buffer[2];
        sprintf(buffer, "E%d", error);
        u8g2Fonts.print(buffer);
    }
    // Draw grid
    display.drawLine(0, weather_display_height, displayWidth, weather_display_height, GxEPD_BLACK);
    display.drawLine(displayWidth / 2, 0, displayWidth / 2, weather_display_height, GxEPD_BLACK);
    if (error == 0)
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
    // TODO does not work during the night
}

void EinkScreen::printCalendarData(const char events[], int error)
{
    // Get height of one Line
    u8g2Fonts.setFont(u8g2_font_helvB18_tf);
    int16_t ta = u8g2Fonts.getFontAscent();  // positive
    int16_t td = u8g2Fonts.getFontDescent(); // negative; in mathematicians view
    int16_t th = ta - td;
    uint16_t x = 0;
    uint16_t y = weather_display_height + (th / 2) + th + 5;
    u8g2Fonts.setCursor(x, y);

    if (error == 0)
    {
        u8g2Fonts.print(events);
    }
    else
    {
        char buffer[2];
        sprintf(buffer, "E%d", error);
        u8g2Fonts.print(buffer);
    }
}

void EinkScreen::drawBatteryIcon(float batteryVoltage)
{
    u8g2Fonts.setFont(u8g2_font_battery19_tn);
    int16_t ta = u8g2Fonts.getFontAscent();  // positive
    int16_t td = u8g2Fonts.getFontDescent(); // negative; in mathematicians view
    int16_t iconHeight = ta - td;
    int16_t iconWidth = u8g2Fonts.getUTF8Width("5");

    int16_t x = displayWidth-iconWidth-3;
    int16_t y = display.height()-3;
    if (batteryVoltage > 4.1)
    {
        u8g2Fonts.drawGlyph(x, y, '5');
    }
    else if (batteryVoltage > 3.9)
    {
        u8g2Fonts.drawGlyph(x, y, '4');
    }
    else if (batteryVoltage > 3.7)
    {
        u8g2Fonts.drawGlyph(x, y, '3');
    }
    else if (batteryVoltage > 3.4)
    {
        u8g2Fonts.drawGlyph(x, y, '2');
    }
    else
    {
        u8g2Fonts.drawGlyph(x, y, '1');
    }
}