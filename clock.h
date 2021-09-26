#pragma once
#include "RTClib.h"

class clock
{
public:
    void init()
    {
        if (!rtc.begin())
        {
            Serial.println("clock: DS3231 init failed");
        }

        if (rtc.lostPower())
        {
            Serial.println("clock: RTC lost power, let's set the time!");
            rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        }
    }

    String getDatetimeString()
    {
        String datetimeString;
        DateTime now = rtc.now();
        datetimeString += String(now.year(), DEC);
        datetimeString += "/";
        datetimeString += String(now.month(), DEC);
        datetimeString += "/";
        datetimeString += String(now.day(), DEC);

        datetimeString += " - ";
        datetimeString += String(now.hour(), DEC);
        datetimeString += ":";
        datetimeString += String(now.minute(), DEC);
        datetimeString += ":";
        datetimeString += String(now.second(), DEC);

        return datetimeString;
    }

private:
    RTC_DS3231 rtc;
};
