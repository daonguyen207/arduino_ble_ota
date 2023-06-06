#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "IOT47_BLE_OTA.h"

#define SERVICE_UUID "55072829-bc9e-4c53-0003-74a6d4c78751"
String bleServerName;

static BLEUUID BLE_UUID(SERVICE_UUID);
BLECharacteristic ch1_BLECharacteristic(SERVICE_UUID, BLECharacteristic::PROPERTY_NOTIFY 
                                                         | BLECharacteristic::PROPERTY_READ 
                                                         | BLECharacteristic::PROPERTY_WRITE  
                                                         | BLECharacteristic::PROPERTY_NOTIFY
                                                         | BLECharacteristic::PROPERTY_INDICATE);
BLEDescriptor     ch1_Descriptor(BLEUUID((uint16_t)0x2902));

static BLERemoteCharacteristic* ch1_Characteristic;

bool deviceConnected = false;
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Client connect");
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    iot47_stop_ota(); //nên gọi hàm này để hủy ota khi bị mất kết nối đột ngột
    Serial.println("Client disconnect");
    pServer->startAdvertising();
  }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      if(iot47_ota_task((uint8_t *)&(rxValue[0]),rxValue.length()))return; //bắt buộc phải gọi ở đây

      //phần này xử lí nhận data ble của user
      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
          Serial.print(rxValue[i]);

        Serial.println();
        Serial.println("*********");
      }
    }
};
void setup()
{
   Serial.begin(115200);
  bleServerName = "IOT47 BLE OTA TEST";
  
  Serial.println("Starting BLE work!");
  
  BLEDevice::init(bleServerName.c_str());
  BLEDevice::setMTU(512);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  //Create the BLE Service
  BLEService *bmeService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristics and Create a BLE Descriptor
  bmeService->addCharacteristic(&ch1_BLECharacteristic);
  ch1_Descriptor.setValue("BLE chanel 1");
  ch1_BLECharacteristic.addDescriptor(&ch1_Descriptor);

  ch1_BLECharacteristic.setCallbacks(new MyCallbacks());
  
  // Start the service
  bmeService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();

  //ch1_Characteristic = bmeService->getCharacteristic(BLE_UUID);
  //ch1_BLECharacteristic->registerForNotify(notifyCallback);
  Serial.println("Waiting a client connection to notify...");


  iot47_ble_ota_begin(&ch1_BLECharacteristic); //bắt buộc phải goik

  //đăng kí callback, tùy user
  iot47_ble_ota_set_begin_callback([](uint32_t curen, uint32_t totol){
      Serial.println("Begin ota");
    });
  iot47_ble_ota_set_proces_callback([](uint32_t curen, uint32_t totol){
    Serial.print(curen);
    Serial.print("/");
    Serial.println(totol);
  });
  iot47_ble_ota_set_end_callback([](uint32_t curen, uint32_t totol){
   Serial.println("Download done");
  });
  iot47_ble_ota_set_error_callback([](uint32_t curen, uint32_t totol){
   Serial.println("Download error");
  });
}

void loop() 
{

}
