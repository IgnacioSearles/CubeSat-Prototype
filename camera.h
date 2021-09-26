#pragma once
#include <ArduCAM.h>
#include "memorysaver.h"
#include <SD.h>

class camera
{
public:
    camera(int camCS)
    {
        cameraCS = camCS;
        myCAM = ArduCAM(OV5642, cameraCS);
    }

    void init()
    {
        uint8_t vid, pid;
        uint8_t temp;

        //set the CS as an output:
        pinMode(cameraCS, OUTPUT);
        digitalWrite(cameraCS, HIGH);

        //Reset the CPLD
        myCAM.write_reg(0x07, 0x80);
        delay(100);
        myCAM.write_reg(0x07, 0x00);
        delay(100);

        while (1)
        {
            //Check if the ArduCAM SPI bus is OK
            myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
            temp = myCAM.read_reg(ARDUCHIP_TEST1);

            if (temp != 0x55)
            {
                Serial.println(F("SPI interface Error!"));
                delay(1000);
                continue;
            }
            else
            {
                Serial.println(F("SPI interface OK."));
                break;
            }
        }

        while (1)
        {
            //Check if the camera module type is OV5642
            myCAM.wrSensorReg16_8(0xff, 0x01);
            myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
            myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
            if ((vid != 0x56) || (pid != 0x42))
            {
                Serial.println(F("camera: Can't find OV5642 module!"));
                delay(1000);
                continue;
            }
            else
            {
                Serial.println(F("camera: OV5642 detected."));
                break;
            }
        }

        myCAM.set_format(JPEG);
        myCAM.InitCAM();

        myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK); //VSYNC is active HIGH
        myCAM.OV5642_set_JPEG_size(OV5642_1024x768);

        delay(1000);

        lastFrameTakenTime = millis();
    }

    void takePictureAndSaveToSD()
    {
        char str[8];
        byte buf[256];
        static int i = 0;
        static int k = 0;
        uint8_t temp = 0, temp_last = 0;
        uint32_t length = 0;
        bool is_header = false;
        File outFile;
        //Flush the FIFO
        myCAM.flush_fifo();
        //Clear the capture done flag
        myCAM.clear_fifo_flag();
        //Start capture
        myCAM.start_capture();
        while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));
        
        length = myCAM.read_fifo_length();
        if (length >= MAX_FIFO_SIZE) //384K
        {
            Serial.println(F("camera: Over size."));
            return;
        }
        if (length == 0) //0 kb
        {
            Serial.println(F("camera: Size is 0."));
            return;
        }
        //Construct a file name
        k = k + 1;
        itoa(k, str, 10);
        strcat(str, ".jpg");
        //Open the new file
        outFile = SD.open(str, O_WRITE | O_CREAT | O_TRUNC);
        if (!outFile)
        {
            Serial.println(F("camera: File open faild"));
            return;
        }
        myCAM.CS_LOW();
        myCAM.set_fifo_burst();
        while (length--)
        {
            temp_last = temp;
            temp = SPI.transfer(0x00);
            //Read JPEG data from FIFO
            if ((temp == 0xD9) && (temp_last == 0xFF)) //If find the end ,break while,
            {
                buf[i++] = temp; //save the last  0XD9
                //Write the remain bytes in the buffer
                myCAM.CS_HIGH();
                outFile.write(buf, i);
                //Close the file
                outFile.close();
                Serial.println(F("camera: Image save OK."));
                is_header = false;
                i = 0;
            }
            if (is_header == true)
            {
                //Write image data to buffer if not full
                if (i < 256)
                    buf[i++] = temp;
                else
                {
                    //Write 256 bytes image data to file
                    myCAM.CS_HIGH();
                    outFile.write(buf, 256);
                    i = 0;
                    buf[i++] = temp;
                    myCAM.CS_LOW();
                    myCAM.set_fifo_burst();
                }
            }
            else if ((temp == 0xD8) & (temp_last == 0xFF))
            {
                is_header = true;
                buf[i++] = temp_last;
                buf[i++] = temp;
            }
        }
    }

    void timelapse(int msBetweenFrames) {
      if (millis() - lastFrameTakenTime >= msBetweenFrames) {
        takePictureAndSaveToSD();
        lastFrameTakenTime = millis();
      }
    }

private:
    int cameraCS;
    unsigned long lastFrameTakenTime;
    ArduCAM myCAM;
};
