#include "sensordlbus.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
//#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace sensordlbus {

static const char *TAG = "sensordlbus";
//DLBus dlBus;  // Erzeugt eine Instanz der Klasse DLBus
// DLBus dlBus(GPIO_NUM_5);

SensorDLBus::SensorDLBus(InternalGPIOPin *dl_pin)  : PollingComponent(50000) , dl_pin_(dl_pin), dlbus_(nullptr) {
}


void SensorDLBus::setup() {
  ESP_LOGD(TAG, "Setting up SensorDLBus");
//  dlBus.init(static_cast<gpio_num_t>(this->DL_->get_pin())); // Initialisiert das Objekt, z. B. setzt interne Zustände
//    this->DL_->get_pin(); // Nur um sicherzustellen, dass der Pin initialisiert ist

  // récupérer le pin configuré
  gpio_num_t pin = static_cast<gpio_num_t>(dl_pin_->get_pin());
  // (si tu préfères l'instancier dans le constructeur, tu peux; ici on fait dans setup pour être sûr)
  if (!dlbus_) {
    dlbus_ = new DLBus(pin,model_);
    dlbus_->init();
  } 
}

void SensorDLBus::update() {
    int i = 0;
    while (i < 3) {
        if (dlbus_->captureSinglePacket()) {
            ESP_LOGD("DL-Bus", "Received Packet");
              auto &lastFrame = dlbus_->lastFrame;

            if (this->deviceTypeSensor_ != nullptr) 
                this->deviceTypeSensor_->publish_state(lastFrame.DeviceID);
            if (this->tempSensor1_ != nullptr) 
                this->tempSensor1_->publish_state(lastFrame.Sensor1 / 10.0);
            if (this->tempSensor2_ != nullptr) 
                this->tempSensor2_->publish_state(lastFrame.Sensor2 / 10.0);
            if (this->tempSensor3_ != nullptr) 
                this->tempSensor3_->publish_state(lastFrame.Sensor3 / 10.0);
            if (this->tempSensor4_ != nullptr) 
                this->tempSensor4_->publish_state(lastFrame.Sensor4 / 10.0);
            if (this->tempSensor5_ != nullptr) 
                this->tempSensor5_->publish_state(lastFrame.Sensor5 / 10.0);
            if (this->tempSensor6_ != nullptr) 
                this->tempSensor6_->publish_state(lastFrame.Sensor6 / 10.0);

            if (this->outputA1Sensor_ != nullptr) 
                this->outputA1Sensor_->publish_state((lastFrame.Outputs >> 0) & 0b1);
            if (this->outputA2Sensor_ != nullptr) 
                this->outputA2Sensor_->publish_state((lastFrame.Outputs >> 1) & 0b1);
            if (this->outputA3Sensor_ != nullptr) 
                this->outputA3Sensor_->publish_state((lastFrame.Outputs >> 2) & 0b1);
            if (this->outputA4Sensor_ != nullptr) 
                this->outputA4Sensor_->publish_state((lastFrame.Outputs >> 3) & 0b1);
            if (this->outputA5Sensor_ != nullptr) 
                this->outputA5Sensor_->publish_state((lastFrame.Outputs >> 4) & 0b1);
            if (this->outputA6Sensor_ != nullptr) 
                this->outputA6Sensor_->publish_state((lastFrame.Outputs >> 5) & 0b1);
            if (this->outputA7Sensor_ != nullptr) 
                this->outputA7Sensor_->publish_state((lastFrame.Outputs >> 6) & 0b1);
            
            break;
        } else {
            ESP_LOGD("DL-Bus", "Signal-Error");
            i++;
            // delay(50);
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}



}  // namespace sensor_dl_bus
}  // namespace esphome
