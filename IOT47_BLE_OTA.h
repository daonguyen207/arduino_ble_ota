#ifndef __IOT47_BLE_OTA__
#define __IOT47_BLE_OTA__

#include <Update.h>
int UpdateRun()
{
  Serial.println("Start update");
  if (Update.end())
  {
      Serial.println("OTA finished!");
      if (Update.isFinished())
      {
          Serial.println("Restart device!");
          delay(2000);
          ESP.restart();
      }
      else
      {
          Serial.println("OTA not fiished");
      }
  }
  else
  {
      Serial.println("Error occured #: " + String(Update.getError()));
  }
}


#define OTA_BEGIN        0
#define OTA_DOWNLOADDING 1
#define OTA_DOWNLOADDONE 2
uint32_t ota_fw_size,ota_fw_counter;
uint32_t ota_download_paket;
uint32_t ota_state;
uint32_t ota_tranfer_mode;
int couter_process = 0;
typedef void (*ota_callback_t)(uint32_t curen, uint32_t totol);
ota_callback_t begin_callback;
ota_callback_t proces_callback;
ota_callback_t end_callback;
ota_callback_t error_callback;

BLECharacteristic *OTA_BLECharacteristic;
void iot47_ble_ota_begin(BLECharacteristic *c)
{
  OTA_BLECharacteristic = c;
}

void iot47_ble_ota_set_begin_callback(ota_callback_t c)
{
  begin_callback = c;
}
void iot47_ble_ota_set_proces_callback(ota_callback_t c)
{
  proces_callback = c;
}
void iot47_ble_ota_set_end_callback(ota_callback_t c)
{
  end_callback = c;
}
void iot47_ble_ota_set_error_callback(ota_callback_t c)
{
  error_callback = c;
}

void iot47_stop_ota()
{
  ota_state = OTA_BEGIN;
}
int iot47_ota_task(uint8_t *rxValue, uint8_t len)
{
  if(ota_state == OTA_BEGIN)
  {
    if (len > 20 && len < 40)  //IOT47_BLE_OTA_BEGIN:1234567\r\n
    {
      if((rxValue[0] == 'I') && (rxValue[1] == 'O') && (rxValue[2] == 'T') && (rxValue[3] == '4') && (rxValue[4] == '7'))
      {
        uint8_t *header = (uint8_t *)malloc(len); 
        for (int i = 0; i < len; i++)header[i] = rxValue[i];
        uint8_t *ota_cmd = (uint8_t *)strstr((const char *)header,(const char *)"IOT47_BLE_OTA_BEGIN:"); //find header
        if(ota_cmd != 0)
        {
          ota_fw_size=0;
          for(int i=0;i<20;i++)
          {
            if((ota_cmd[20 + i] == '\r') || (ota_cmd[20 + i] == '\n'))
            {
              ota_state = OTA_DOWNLOADDING;
              ota_fw_counter = 0;
              ota_download_paket = 0;
              OTA_BLECharacteristic->setValue("OK\r\n");
              OTA_BLECharacteristic->notify();
              if(begin_callback!=0)begin_callback(ota_fw_counter,ota_fw_size); 
              Update.begin(ota_fw_size);
              free(header);    
              return 1;
            }
            ota_fw_size*=10;
            ota_fw_size+= ota_cmd[20 + i]-48;
          }
        }
        free(header);
      }
    }
  }
  else if(ota_state == OTA_DOWNLOADDING)
  {
    // [0][1] = số thứ tự gói tin     |      [2][3] = size payload   |       [4]...[n] play load
    uint16_t packet = ((uint16_t)rxValue[0]<<8) | (uint16_t)rxValue[1];
    if(packet == ota_download_paket)
    {
      ota_download_paket++;
      uint16_t size = ((uint16_t)rxValue[2]<<8) | (uint16_t)rxValue[3];
      Update.write((uint8_t *)&(rxValue[4]),size);
      ota_fw_counter+=size;
      couter_process++;
      if(couter_process==20)
      {
        couter_process=0;
        if(proces_callback!=0)proces_callback(ota_fw_counter,ota_fw_size);
      }
      if(ota_fw_counter == ota_fw_size)
      {
        OTA_BLECharacteristic->setValue("OTA DONE\r\n");
        OTA_BLECharacteristic->notify();
        ota_state = OTA_DOWNLOADDONE;
        if(end_callback!=0)end_callback(ota_fw_counter,ota_fw_size);
        UpdateRun();
        return 3;
      }
    }
    else
    {
      Serial.println("Lỗi khi ota");
      OTA_BLECharacteristic->setValue("Fail\r\n");
      OTA_BLECharacteristic->notify();
      if(error_callback!=0)error_callback(ota_fw_counter,ota_fw_size);
    }
    return 2;
  }
  return 0;
}



#endif
