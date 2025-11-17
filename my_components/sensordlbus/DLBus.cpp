#include "DLBus.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "DLBus";

DLBus *DLBus::instance = nullptr;

DLBus::DLBus(gpio_num_t pin) : dl_input_pin(pin) {
  instance = this;
  for (int i = 0; i < DL_Bus_PacketLength; i++)
    DL_Bus_Buffer[i] = 0xFF;

  timeSincelastEdge = 0;
  timeOfActEdge = 0;
  nextBit = false;
  curBit  = false;
}

void DLBus::init() {
  gpio_config_t cfg = {};
  cfg.intr_type = GPIO_INTR_ANYEDGE;
  cfg.mode = GPIO_MODE_INPUT;
  cfg.pin_bit_mask = (1ULL << dl_input_pin);
  cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
  cfg.pull_up_en   = GPIO_PULLUP_DISABLE;
  gpio_config(&cfg);

  gpio_install_isr_service(0);
  gpio_isr_handler_add(dl_input_pin, DLBus::gpio_isr_handler, this);

  timeSincelastEdge = esp_timer_get_time();
}

void IRAM_ATTR DLBus::gpio_isr_handler(void *arg) {
  DLBus *self = reinterpret_cast<DLBus*>(arg);
  if (self) self->handleInterrupt();
}

void DLBus::handleInterrupt() {
  uint64_t now = esp_timer_get_time();
  actData.edgetime = (uint32_t)(now - timeSincelastEdge);
  actData.pinState = (gpio_get_level(dl_input_pin) == 0);
  edgeTimeBuffer.push(actData);
  timeSincelastEdge = now;
}

bool DLBus::loadBitFromEdgeTimeBuffer() {
  int i = 0;
  while (edgeTimeBuffer.count() == 0) {
    vTaskDelay(pdMS_TO_TICKS(2));
    if (++i == 3) return false;
  }
  edgeTimeBuffer.pop(newData);
  return true;
}

int DLBus::captureBit() {
  if (!loadBitFromEdgeTimeBuffer()) return 2;

  edgetime = newData.edgetime;

  if (edgetime > Tmin && edgetime < Tmax) {
    if (!loadBitFromEdgeTimeBuffer()) return 2;
    edgetime = newData.edgetime;
    if (edgetime > Tmin && edgetime < Tmax)
      nextBit = curBit;
    else
      return 2;
  }
  else if (edgetime > (2*Tmin) && edgetime < (2*Tmax))
    nextBit = !curBit;
  else
    return 2;

  curBit = nextBit;
  return nextBit ? 1 : 0;
}

unsigned char DLBus::receiveByte() {
  unsigned char rxByte = 0;

  if (captureBit() == 0) {
    for (int i=0; i<8; i++) {
      int b = captureBit();
      if (b == 2) return 0x00;
      if (b == 1) SET_BIT(rxByte, i);
    }
    if (captureBit() == 1) return rxByte;
  }
  return 0x00;
}

bool DLBus::testChecksum() {
  unsigned char checksum = 1;
  for (int i=0; i < DL_Bus_PacketLength-1; i++)
    checksum += DL_Bus_Buffer[i];

  return (checksum == DL_Bus_Buffer[DL_Bus_PacketLength-1]);
}

int16_t DLBus::processTemperature(char low, char high) {
  if ((high & 0x80) == 0)
    high &= 0x8F;
  else {
    high |= 0x40;
    high |= 0x20;
    high |= 0x10;
  }
  return (int16_t)low + 256 * (int16_t)high;
}

void DLBus::processData() {
  lastFrame.DeviceID = DL_Bus_Buffer[1];
  lastFrame.Sec      = DL_Bus_Buffer[3];
  lastFrame.Min      = DL_Bus_Buffer[4];
  lastFrame.Hour     = DL_Bus_Buffer[5];
  lastFrame.Day      = DL_Bus_Buffer[6];
  lastFrame.Month    = DL_Bus_Buffer[7];
  lastFrame.Year     = DL_Bus_Buffer[8] + 2000;

  lastFrame.Sensor1 = processTemperature(DL_Bus_Buffer[9],  DL_Bus_Buffer[10]);
  lastFrame.Sensor2 = processTemperature(DL_Bus_Buffer[11], DL_Bus_Buffer[12]);
  lastFrame.Sensor3 = processTemperature(DL_Bus_Buffer[13], DL_Bus_Buffer[14]);
  lastFrame.Sensor4 = processTemperature(DL_Bus_Buffer[15], DL_Bus_Buffer[16]);
  lastFrame.Sensor5 = processTemperature(DL_Bus_Buffer[17], DL_Bus_Buffer[18]);
  lastFrame.Sensor6 = processTemperature(DL_Bus_Buffer[19], DL_Bus_Buffer[20]);

  lastFrame.Outputs = DL_Bus_Buffer[41] + 256 * DL_Bus_Buffer[42];
}

bool DLBus::captureSinglePacket() {
  edgeTimeBuffer.clear();

  uint64_t start = esp_timer_get_time();

  while (edgeTimeBuffer.count() < 100) {
    vTaskDelay(pdMS_TO_TICKS(10));
    if ((esp_timer_get_time() - start)/1000 > timeout) return false;
  }

  while (true) {
    edgeTimeBuffer.pop(newData);
    edgetime = newData.edgetime;
    if (edgetime > 2*Tmin && edgetime < 2*Tmax)
      break;

    if ((esp_timer_get_time() - start)/1000 > timeout)
      return false;
  }

  curBit = newData.pinState;

  while (edgeTimeBuffer.count() < 32) {
    vTaskDelay(pdMS_TO_TICKS(10));
    if ((esp_timer_get_time() - start)/1000 > timeout)
      return false;
  }

  int sync = 0;

  while (sync <= 16) {
    if (captureBit() == 1)
      sync++;
    else {
      sync = 0;
      while (edgeTimeBuffer.count() < 32) {
        vTaskDelay(pdMS_TO_TICKS(6));
        if ((esp_timer_get_time() - start)/1000 > timeout)
          return false;
      }
    }
  }

  DL_Bus_Buffer[0] = 0xFF;
  for (int i=1; i<DL_Bus_PacketLength; i++)
    DL_Bus_Buffer[i] = receiveByte();

  if (!testChecksum()) return false;

  processData();
  return true;
}
