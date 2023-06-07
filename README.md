# arduino_ble_ota
Thư viện ota qua ble cho esp32 arduino. Tải app BLE OTA trên apk và ios.

APK: https://play.google.com/store/apps/details?id=com.esp32.ble.ota

Phiên bản cho espidf: https://github.com/daonguyen207/espidf_ble_ota

# Khởi tạo:
iot47_ble_ota_begin(&ch1_BLECharacteristic); //bắt buộc phải goi
Gọi trong setup ( ch1_BLECharacteristic ) là đối số

# Đăng kí callback
```
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
```

# Gọi handler
if(iot47_ota_task((uint8_t *)&(rxValue[0]),rxValue.length()))return;
Hàm này gọi trong event rx của ble

# Hủy ota
iot47_stop_ota();
Hàm này gọi ở hàm mất kết nối

