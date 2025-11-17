#ifndef DLBus_h
#define DLBus_h

#include <array>
#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_timer.h"

// Macro inchangée
#define SET_BIT(byte, bit) ((byte) |= (1UL << (bit)))

constexpr size_t FIFO_SIZE = 256;

template<typename T, size_t SIZE>
class RingBuf {
 public:
  bool push(const T item) {
    if (is_full())
      return false;
    this->rb_[this->head_] = item;
    this->head_ = (this->head_ + 1) % SIZE;
    this->count_++;
    return true;
  }

  bool pop(T &item) {
    if (is_empty())
      return false;
    item = this->rb_[this->tail_];
    this->tail_ = (this->tail_ + 1) % SIZE;
    this->count_--;
    return true;
  }

  bool peek(T &item) {
    if (is_empty())
      return false;
    item = this->rb_[this->tail_];
    return true;
  }

  inline bool is_empty() { return (this->count_ == 0); }
  inline bool is_full()  { return (this->count_ == SIZE); }
  inline size_t count()  { return this->count_; }
  inline size_t free()   { return SIZE - this->count_; }
  inline void clear()    { this->head_ = this->tail_ = this->count_ = 0; }

 protected:
  std::array<T, SIZE> rb_{0};
  int tail_{0};
  int head_{0};
  size_t count_{0};
};


class DLBus {
public:
  static const int Ringbuffersize = 2000;
  static const int DL_Bus_PacketLength = 36;
  static const int Tmin = 800;     // µs
  static const int Tmax = 1200;    // µs
  static const int timeout = 3000; // ms timeout

  struct InterruptData {
    uint32_t edgetime;
    bool pinState;
  };

  struct DL_Bus_Frame {
    uint8_t  DeviceID;
    uint8_t  Sec, Min, Hour, Day, Month;
    uint16_t Year;
    int16_t  Sensor1, Sensor2, Sensor3, Sensor4, Sensor5, Sensor6;
    uint16_t Outputs;
  };

  // ⭐ CONSTRUCTEUR AVEC PIN CONFIGURABLE
  DLBus(gpio_num_t pin);

  void init();
  bool captureSinglePacket();

  DL_Bus_Frame lastFrame;

  // ISR statique
  static void IRAM_ATTR gpio_isr_handler(void *arg);

private:
  // ⭐ Le pin est MAINTENANT une valeur d'instance, configurable
  gpio_num_t dl_input_pin;

  // Pointeur instance
  static DLBus *instance;

  InterruptData actData;
  InterruptData newData;

  uint64_t timeSincelastEdge;
  uint64_t timeOfActEdge;

  bool nextBit;
  bool curBit;
  uint32_t edgetime;

  RingBuf<InterruptData, Ringbuffersize> edgeTimeBuffer;
  RingBuf<bool, Ringbuffersize> bitBuffer;

  unsigned char DL_Bus_Buffer[DL_Bus_PacketLength];

  void handleInterrupt();
  bool loadBitFromEdgeTimeBuffer();
  int  captureBit();
  unsigned char receiveByte();
  bool testChecksum();
  int16_t processTemperature(char lowByte, char highbyte);
  void processData();
};

#endif
