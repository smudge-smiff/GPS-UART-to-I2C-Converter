#include <TinyGPSPlus.h>
#include <Wire.h>
#include "uniondef.h"

#define Address 0x03
#define GpsBAUD 57600
#define LED_PIN PC13

HardwareSerial gpsSerial(PA3, PA2);
HardwareSerial debugUART(PA12, PA11);

byte dat[128] = {0};
uint8_t reg_to_read = 0;
TinyGPSPlus gps;
uint32Union_t x;

unsigned long start_time;
unsigned long updateTime;
unsigned long current_time;

void setup()
{
  // put your setup code here, to run once:
  x.value = 20;
  debugUART.begin(57600);
  gpsSerial.begin(GpsBAUD);
  pinMode(LED_PIN, OUTPUT);
  Wire.setSDA(PB7);
  Wire.setSCL(PB6);
  Wire.begin(Address);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  updateTime = 50; // after 10 ms trigger the event
  current_time = millis();
  start_time = current_time;
}

void loop()
{
  while (gpsSerial.available() > 0)
  {
    if (gps.encode(gpsSerial.read()))
    {
    }
  }

  current_time = millis(); // update the timer every cycle

  if (current_time - start_time >= updateTime)
  {
    start_time = current_time; // reset the timer
    hasFix();
    updateDataBuffer();
    digitalWrite(LED_PIN, !hasFix());
  }
}

void receiveEvent(int bytes)
{
  byte recieved[bytes];
  int i = 0;
  while (0 < Wire.available())
  { // Read all bytes
    recieved[i] = Wire.read();
    i++;
  }
  reg_to_read = recieved[0];
  if (sizeof(recieved) >= 2)
  {
    dat[reg_to_read] = recieved[1]; // Update the reigster with the second byte recieved, disgard all other data
  }
}

void requestEvent()
{
  for (int i = reg_to_read; i < sizeof(dat); i++)
  {
    if (i >= sizeof(dat))
    {
      Wire.write(0x00);
    }
    else
    {
      Wire.write(dat[i]);
    }
  }
}

void updateDataBuffer()
{
  dat[0] = 0b00000000; // Reserved for GPS baud rate functionality
  dat[1] = 0b00000000; // Reserved for status information

  doubleUnion_t lat;
  lat.value = gps.location.lat();
  loadBytes(lat.bytes, 2); // bytes 2 - 5

  doubleUnion_t lng;
  lat.value = gps.location.lng();
  loadBytes(lng.bytes, 6); // bytes 6 - 9

  doubleUnion_t alt;
  alt.value = gps.altitude.meters();
  loadBytes(alt.bytes, 10); // bytes 10-13

  doubleUnion_t course;
  course.value = gps.course.deg();
  loadBytes(course.bytes, 14); // bytes 14-17

  uint32Union_t sat;
  sat.value = gps.satellites.value(); // No
  dat[18] = sat.bytes[0];             // No need to send all four bytes, satellites will never be greater than 128

  int32Union_t hdop;
  hdop.value = gps.hdop.value();
  loadBytes(hdop.bytes, 19); // 19 - 22
}

void loadBytes(byte data_to_load[], int start_byte)
{
  int howMany = sizeof(data_to_load);
  int end_byte = start_byte + howMany - 1;
  int j = 0;
  for (int i = start_byte; i <= end_byte; i++)
  {
    dat[i] = data_to_load[j];
    j++;
  }
}
/*
void setIthBit(byte test, int bit){
  byte compare = (0b10000000 >> bit);
  byte temp = test | compare;
  test = temp | test;
  debugUART.println(test, BIN);
}
void statusOfIBit(byte test, int bit){
  byte res = (test << bit) & 0b10000000;
  debugUART.print("Res:");
  debugUART.println(res, BIN);
  if (res == 0b10000000){
    debugUART.println("True");
  }else {
    debugUART.println("False");
  }
}
*/
/*
  Can be assumed, if the age of the relevant positions exceed, 1.5s then the GPS has lost fix and is no longer active
*/
bool hasFix()
{
  uint32_t locationAge = gps.location.age();
  uint32_t altAge = gps.altitude.age();
  uint32_t satellitesAge = gps.satellites.age();
  uint32_t speedAge = gps.speed.age();
  uint32_t courseAge = gps.course.age();
  if (locationAge > 1500 | altAge > 1500 | satellitesAge > 1500 | speedAge > 1500 | courseAge > 1500)
  {
    return false;
  }
  else
  {
    return true;
  }
}