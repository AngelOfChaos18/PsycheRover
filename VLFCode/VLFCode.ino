#include "BluetoothSerial.h"

String deviceName = "PsycheRoverESP32";
uint32_t targetFreq = 5270;
uint8_t LEDCPin = 18;
uint8_t resolution = 12;
uint32_t maxDutySteps = (1 << resolution) - 1;
uint32_t dutyCycle = maxDutySteps * 0.25;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
  #error Bluetooth is not enabled! Please run 'make menuconfig' to enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
  #error Serial Port Profile for Bluetooth is not avilable or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT

#define BT_DISCOVER_TIME 10000

static bool BT_SCAN_ASYNC = true;

void BTAdvertisedDeviceFound(BTAdvertisedDevice* pDevice)
{
  Serial.printf("Found a device asynchronously: %s\n", pDevice->toString().c_str());
}

void changeDutyCycle(int val)
{
  if(val >= 1 && val <= 100)
  {
    dutyCycle = maxDutySteps * (val / 100.0);
    Serial.print("Duty Cycle changed to ");
    Serial.println(dutyCycle);
    ledcWrite(LEDCPin, dutyCycle);
  }
  else
    Serial.println("Invalid parameter passed. Enter a number between 1-100 inclusive.");
}

void changeFreq(int val)
{
  targetFreq = (val > 0 && val <= 10000) ? val : targetFreq;
  ledcChangeFrequency(LEDCPin, targetFreq, resolution);
  Serial.print("Target Frequency changed to ");
  Serial.println(targetFreq);
  if(targetFreq != val)
    Serial.println("Invalid parameter passed. Enter a number between 1-10000 inclusive.");
}

void changeResolution(int val)
{
  int dutyPercent = ( (dutyCycle / (double)maxDutySteps)) * 100;
  resolution = (val >= 10 && val <= 13) ? val : resolution;
  maxDutySteps = (1 << resolution) -1;
  Serial.print("Resolution changed to ");
  Serial.println(resolution);
  changeDutyCycle(dutyPercent);
  ledcWrite(LEDCPin, dutyCycle);
  if(resolution != val)
    Serial.println("Invalid parameter passed. Enter a number between 10-13 inclusive.");
}

void help()
{
  Serial.println("List of avalible commands:");
  Serial.println("ChangeDutyCycle value - Will change duty cycle to % value entered. Only values 1-100 inclusive are valid.");
  Serial.println("ChangeFreq value - Will change the target frequency to value entered. Only values 1-10000 inclusive are valid.");
  Serial.println("ChangeResolution value - Will change the bit resolution to value entered. Only values 1-13 inclusive are valid.");
}

void setup() 
{
  Serial.begin(115200);
  Serial.setTimeout(50);
  SerialBT.begin(deviceName);
  //SerialBT.deleteAllBondedDevices();
  Serial.printf("The device with name \"%s\" is started.\nNow you can pair it with Bluetooth!\n", deviceName.c_str());

  ledcAttach(LEDCPin, targetFreq, resolution);
  ledcWrite(LEDCPin, dutyCycle);
  Serial.println("Starting VLF Detector software.");

  if(btScanAsync)
  {
    Serial.print("Starting asynchronous discovery...\n");
    if(SerialBT.discoverAsync(BTAdvertisedDeviceFound))
    {
      delay(10000);
      Serial.print("Stopping discoverAsync...   ");
      SerialBT.discoverAsyncStop();
      Serial.println("Stopped.");
    }
    else
      Serial.println("Error on discoverAsync f.e. not working after a \"connect\"");
  }
}

void loop() 
{
  if(Serial.available() > 0)
  {
    SerialBT.write(Serial.read());
    String input = Serial.readStringUntil('\n');
    input.trim();
    input.toUpperCase();

    int spaceIndex = input.indexOf(' ');
    if(spaceIndex > 1)
    {
      String command = input.substring(0, spaceIndex);
      command.toUpperCase();
      int value = input.substring(spaceIndex + 1).toInt();

      if(command == "CHANGEDUTYCYCLE")
          changeDutyCycle(value);
      else if(command == "CHANGEFREQ")
          changeFreq(value);
      else if(command == "CHANGERESOLUTION")
          changeResolution(value);
      else
        Serial.println("Unknown command, for a list of commands enter 'help'");
    }
    else if(input == "HELP")
      help();
  }

  if(SerialBT.available())
  {
    Serial.write(SerialBT.read());
    delay(20);
  }

  yield();
}