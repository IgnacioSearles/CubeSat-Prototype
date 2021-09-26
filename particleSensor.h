#pragma once
#include <Seeed_HM330X.h>

class particleSensor
{
  public:
    void init()
    {
      Serial.println("particle sensor: turning on");
      if (sensor.init())
      {
        while (1) Serial.println("particle sensor: HM330X init failed!!!");
      }
    }

    uint16_t getValueAtPos(int pos) {
      if (sensor.read_sensor_value(buf, 29)) {
        Serial.println("particle sensor: HM330X read result failed!!!");
      }
      uint16_t value = 0;
      for (int i = 2; i < 8; i++) {
        value = (uint16_t) buf[i * 2] << 8 | buf[i * 2 + 1];
        if (i - 2 == pos) return value;
      }
      return value;
    }
  private:
    HM330X sensor;
    uint8_t buf[30];
};
