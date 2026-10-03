#include "BluetoothSerial.h"

static bool BT_SCAN_ASYNC = true;
static int NUMBER_OF_COMMANDS = 7;

String deviceName = "PsycheRoverESP32";

String commandList[] = {
  "List of avalible commands:",
  "ChangeDutyCycle value - Will change duty cycle to % value entered. Only values 1-100 inclusive are valid.",
  "ChangeFreq value - Will change the target frequency to value entered. Only values 1-10000 inclusive are valid.",
  "ChangeResolution value - Will change the bit resolution to value entered. Only values 1-13 inclusive are valid.",
  "GetDutyCycle - Will display the current Duty Cycle.",
  "GetFreq - Will display the current Frequency.",
  "GetResolution - Will display the current Resolution."
};

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

BluetoothSerial SerialBT;

#define BT_DISCOVER_TIME 10000

void BTAdvertisedDeviceFound(BTAdvertisedDevice* pDevice)
{
  Serial.printf("Found a device asynchronously: %s\n", pDevice->toString().c_str());
}

void changeDutyCycle(int val)
{
  if(val >= 1 && val <= 100)
  {
    dutyCycle = maxDutySteps * (val / 100.0);
    String msg = String("Duty Cycle changed to " + String(dutyCycle));
    Serial.println(msg);
    SerialBT.println(msg);
    ledcWrite(LEDCPin, dutyCycle);
  }
  else
  {
    String msg = "Invalid parameter passed. Enter a number between 1-100 inclusive.";
    Serial.println(msg);
    SerialBT.println(msg);
  }
}

void changeFreq(int val)
{
  targetFreq = (val > 0 && val <= 10000) ? val : targetFreq;
  ledcChangeFrequency(LEDCPin, targetFreq, resolution);
  String msg = String("Target Frequency changed to " + String(targetFreq));
  Serial.println(msg);
  SerialBT.println(msg);
  if(targetFreq != val)
  {
    Serial.println("Invalid parameter passed. Enter a number between 1-10000 inclusive.");
    SerialBT.println("Invalid parameter passed. Enter a number between 1-10000 inclusive.");
  }
}

void changeResolution(int val)
{
  int dutyPercent = ( (dutyCycle / (double)maxDutySteps)) * 100;
  resolution = (val >= 10 && val <= 13) ? val : resolution;
  maxDutySteps = (1 << resolution) -1;
  String msg = String("Resolution changed to " + String(resolution));
  Serial.println(msg);
  SerialBT.println(msg);
  changeDutyCycle(dutyPercent);
  ledcWrite(LEDCPin, dutyCycle);
  if(resolution != val)
  {
    Serial.println("Invalid parameter passed. Enter a number between 10-13 inclusive.");
    SerialBT.println("Invalid parameter passed. Enter a number between 10-13 inclusive.");
  }
}

void GetData(String data)
{
  String msg;
  if(data == "GETDUTYCYCLE")
    msg = String("Current Duty Cycle: " + String(dutyCycle));
  else if(data == "GETFREQ")
    msg = String("Current Frequency: " + String(targetFreq));
  else if(data == "GETRESOLUTION")
    msg = String("Current Resolution: " + String(resolution));
  else
    msg = "Invalid command, enter 'help' to see a list of commands";

  Serial.println(msg);
  SerialBT.println(msg);
}

void help()
{
  for(int i = 0; i < NUMBER_OF_COMMANDS; i++)
  {
    Serial.println(commandList[i]);
    SerialBT.println(commandList[i]);
  }
}

void ExecutCommand(String command, int value)
{
  if(command == "CHANGEDUTYCYCLE")
  {
    String commandText = String(command + " " + String(value));
    SerialBT.println(commandText);
    changeDutyCycle(value);
  }
  else if(command == "CHANGEFREQ")
  {
      changeFreq(value);
  }
  else if(command == "CHANGERESOLUTION")
  {
      changeResolution(value);
  }
  else
  {
    Serial.println("Unknown command, for a list of commands enter 'help'");
    SerialBT.println("Unknown command, for a list of commands enter 'help'");
  }
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

  if(BT_SCAN_ASYNC)
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
    String input = Serial.readStringUntil('\n');
    input.trim();
    input.toUpperCase();

    int spaceIndex = input.indexOf(' ');
    if(spaceIndex > 1)
    {
      String command = input.substring(0, spaceIndex);
      command.toUpperCase();
      int value = input.substring(spaceIndex + 1).toInt();

      ExecutCommand(command, value);
    }
    else if(input == "HELP")
      help();
    else if(input == "GETDUTYCYCLE" || input == "GETFREQ" || input == "GETRESOLUTION")
      GetData(input);
  }

  if(SerialBT.available())
  {
    String recievedInput = SerialBT.readStringUntil('\n');
    recievedInput.trim();
    recievedInput.toUpperCase();

    int spaceIndex = recievedInput.indexOf(' ');
    if(spaceIndex > 1)
    {
      String command = recievedInput.substring(0, spaceIndex);
      command.toUpperCase();
      int value = recievedInput.substring(spaceIndex + 1).toInt();

      ExecutCommand(command, value);
    }
    else if(recievedInput == "HELP")
      help();
    else if(recievedInput == "GETDUTYCYCLE" || recievedInput == "GETFREQ" || recievedInput == "GETRESOLUTION")
      GetData(recievedInput);
    delay(20);
  }

  yield();
}