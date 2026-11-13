#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* frequencyCharacteristic = NULL;
BLECharacteristic* resolutionCharacteristic = NULL;
BLECharacteristic* dutyCycleCharacteristic = NULL;
BLECharacteristic* piDataArrayCharacteristic = NULL;
BLECharacteristic* materialClassificationCharacteristic = NULL;
BLECharacteristic* tauValueCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

#define SERVICE_UUID        "3e878e35-49e2-4e5c-8d32-11dd9b0409d8"
#define FREQUENCY_CHARACTERISTIC_UUID "280db291-af91-403c-9a8f-37d83135b49c"
#define RESOLUTION_CHARACTERISTIC_UUID "9034bce6-0e5b-484a-a281-54633a7704f2"
#define DUTY_CYCLE_CHARACTERISTIC_UUID "2e645218-b656-4a16-92e9-c3d51359c0ad"
#define PI_DATA_ARRAY_CHARACTERISTIC_UUID "fda96f74-ad2f-434a-9b01-d7ada3a184fc"
#define MATERIAL_CLASSIFICATION_CHARACTERISTIC_UUID "4ca99c04-2a91-4869-a03a-40c7b09e54e2"
#define TAU_VALUE_CHARACTERISTIC_UUID "16ffa6d9-8e79-4807-ab34-7f9f20b75e99"

String deviceName = "PsycheRoverESP32";

uint32_t targetFreq = 5270;
uint8_t LEDCPin = 18;
uint8_t resolution = 12;
uint32_t maxDutySteps = (1 << resolution) - 1;
uint32_t dutyCycle = maxDutySteps * 0.25;

class MyServerCallbacks: public BLEServerCallbacks 
{
  void onConnect(BLEServer* pServer) 
  {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) 
  {
    deviceConnected = false;
  }
};

void changeDutyCycle(int val)
{
  if(val >= 1 && val <= 100)
  {
    dutyCycle = maxDutySteps * (val / 100.0);
    String msg = String("Duty Cycle changed to " + String(dutyCycle));
    Serial.println(msg);
    ledcWrite(LEDCPin, dutyCycle);
  }
  else
  {
    String msg = "Invalid parameter passed. Enter a number between 1-100 inclusive.";
    Serial.println(msg);
  }
}

void changeFreq(int val)
{
  targetFreq = (val > 0 && val <= 10000) ? val : targetFreq;
  ledcChangeFrequency(LEDCPin, targetFreq, resolution);
  String msg = String("Target Frequency changed to " + String(targetFreq));
  Serial.println(msg);
  if(targetFreq != val)
  {
    Serial.println("Invalid parameter passed. Enter a number between 1-10000 inclusive.");
  }
}

void changeResolution(int val)
{
  int dutyPercent = ( (dutyCycle / (double)maxDutySteps)) * 100;
  resolution = (val >= 10 && val <= 13) ? val : resolution;
  maxDutySteps = (1 << resolution) -1;
  String msg = String("Resolution changed to " + String(resolution));
  Serial.println(msg);
  changeDutyCycle(dutyPercent);
  ledcWrite(LEDCPin, dutyCycle);
  if(resolution != val)
  {
    Serial.println("Invalid parameter passed. Enter a number between 10-13 inclusive.");
  }
}

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks 
{
  void onWrite(BLECharacteristic* pCharacteristic) 
  {
    String value = pCharacteristic->getValue();
    if (value.length() > 0) 
    {
      Serial.print("Characteristic event, written: ");
      Serial.println(value);

      int receivedValue = value.toInt();

      if(pCharacteristic == frequencyCharacteristic)
        changeFreq(receivedValue);

      if(pCharacteristic == dutyCycleCharacteristic)
        changeDutyCycle(receivedValue);

      if(pCharacteristic == resolutionCharacteristic)
        changeResolution(receivedValue);
    }
  }
};

void GenerateRandomPIValues()
{
  String dataString = "";

  float piData[8];
  float initialValue = random(60, 130) / 10.0;
  float baseLineValue = 0.01;
  float decayRate = 0.45;

  for(int i = 0; i < 8; i++)
  {
    piData[i] = baseLineValue + (initialValue - baseLineValue) * exp(-i * decayRate);

    if(piData[i] < 0)
      piData[i] = 0;

    dataString += String(piData[i], 2);
    if(i < 7)
      dataString += ", ";
  }

  piDataArrayCharacteristic->setValue(dataString.c_str());
  piDataArrayCharacteristic->notify();
  Serial.print("New value notified: ");
  Serial.println(dataString);
}

void setup() 
{
  Serial.begin(115200);

  BLEDevice::init(deviceName);

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  frequencyCharacteristic = pService->createCharacteristic(
                      FREQUENCY_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE
                    );

  resolutionCharacteristic = pService->createCharacteristic(
                      RESOLUTION_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE
                    );

  dutyCycleCharacteristic = pService->createCharacteristic(
                      DUTY_CYCLE_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE
                    );

  piDataArrayCharacteristic = pService->createCharacteristic(
                      PI_DATA_ARRAY_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
    
    materialClassificationCharacteristic = pService->createCharacteristic(
                      MATERIAL_CLASSIFICATION_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
    );

    tauValueCharacteristic = pService->createCharacteristic(
                      TAU_VALUE_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
    );

  frequencyCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  resolutionCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  dutyCycleCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  piDataArrayCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  materialClassificationCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  tauValueCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  piDataArrayCharacteristic->addDescriptor(new BLE2902());
  materialClassificationCharacteristic->addDescriptor(new BLE2902());
  tauValueCharacteristic->addDescriptor(new BLE2902());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x00);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");

  ledcAttach(LEDCPin, targetFreq, resolution);
  ledcWrite(LEDCPin, dutyCycle);
  Serial.println("Starting VLF Detector software.");
}

void loop() 
{
  if (deviceConnected) 
  {
    GenerateRandomPIValues();
    delay(5000); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
  }

  if (!deviceConnected && oldDeviceConnected) 
  {
    Serial.println("Device disconnected.");
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("Start advertising");
    oldDeviceConnected = deviceConnected;
  }

  if (deviceConnected && !oldDeviceConnected) 
  {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
    Serial.println("Device Connected");
  }

  yield();
}