#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Arduino_MKRENV.h>
#include <WiFi101.h>

#include "camera.h"
#include "particleSensor.h"
#include "clock.h"

#define SD_CS 4
const int SPI_CS = 5;

camera cam(SPI_CS);
particleSensor particle_sensor;
clock clk;

unsigned long lastDataRead;

char ssid[] = "cubeSat";
char pass[] = "cube_sat";

void setup()
{
  Wire.begin();
  Serial.begin(115200);

  Serial.println("CubeSAT");

  // initialize SPI:
  SPI.begin();

  ENV.begin();

  cam.init();
  particle_sensor.init();
  clk.init();

  //Initialize SD Card
  while (!SD.begin(SD_CS))
  {
    Serial.println(F("SD Card Error!"));
    delay(1000);
  }
  Serial.println(F("SD Card detected."));

  WiFi.begin(ssid, pass);

  lastDataRead = millis();
}

void loop()
{
  cam.timelapse(500);

  if (millis() - lastDataRead >= 1000)
  {
    Serial.println("datalog: Writing data to SD");
    
    File dataFile = SD.open("datos.csv", FILE_WRITE);
    dataFile.print(clk.getDatetimeString());
    dataFile.print(",");

    if (WiFi.status() == WL_CONNECTED) dataFile.print(WiFi.RSSI());
    else dataFile.print("0");
    dataFile.print(",");
    
    dataFile.print(ENV.readTemperature());
    dataFile.print(",");
    dataFile.print(ENV.readHumidity());
    dataFile.print(",");
    dataFile.print(ENV.readPressure());
    dataFile.print(",");

    for (int particleDataType = 0; particleDataType < 6; particleDataType++)
    {
      dataFile.print(particle_sensor.getValueAtPos(particleDataType));
      dataFile.print(",");
    }

    dataFile.println("");
    dataFile.close();

    lastDataRead = millis();
  }
}
