#include <SPI.h>

#include "camera.h"
#include "network.h"

RTC_DATA_ATTR bool after_reset = true;

void setup() {

  Serial.begin(115200);
  delay(1000);

  uint8_t *image_buffer = NULL;
  size_t image_size = 0;

  if (after_reset) {
    camera_config();
    after_reset = false;
    goto sleep;
  }

  if (camera_init() != 0) {
    Serial.println("camera_init() failed");
    goto sleep;
  }

  if (camera(image_buffer, image_size) != 0) {
    Serial.println("camera() failed");
    goto sleep;
  }
  
  if (network_init() != 0) {
    Serial.println("network_init() failed");
    goto sleep;
  }
    
  connect_to_server(image_buffer, image_size);
  camera_free();
  
  sleep:
    Serial.println("Going to sleep...");
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_12, 1);
    esp_deep_sleep_start();
}

void loop() {

}
