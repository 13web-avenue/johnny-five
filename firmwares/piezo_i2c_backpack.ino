#include <Wire.h>
// Sketch > Include Library > Manage Libraries
// Search for "RingBuf", install "RingBuf"
#include <RingBuf.h>

#define DEBUG_MODE 0


// Address Pins
#define AD0 11
#define AD1 12

// I2C Configuration
#define I2C_DEFAULT_ADDRESS 0x0A
#define BUFFER_SIZE 8

// FUNCTION COMMANDS
#define NO_TONE 0x00
#define TONE 0x01

struct frame {
  uint8_t command;
  uint8_t pin;
  uint16_t hz;
  uint32_t ms;
};

RingBuf *buffer = RingBuf_new(sizeof(struct frame), 8);

int addressPins[] = { AD0, AD1 };
int address = I2C_DEFAULT_ADDRESS;

void setup() {
  // Determine the I2C addresss
  // by reading the designated pins
  int offset = 0;

  for (int i = 0; i < 2; i++) {
    pinMode(addressPins[i], INPUT);
    if (digitalRead(addressPins[i])) {
      offset |= 1 << i;
    }
  }

  address += offset;

  #if DEBUG_MODE
    Serial.begin(9600);
  #endif

  for (int i = 0; i < AD0; i++) {
    pinMode(i, OUTPUT);
  }

  Wire.begin(address);
  Wire.onReceive(onReceive);
}

void loop() {
  uint8_t numElements = buffer->numElements(buffer);

  if (numElements > 0) {
    for (int i = 0; i < numElements; i++) {
      struct frame f;

      buffer->pull(buffer, &f);

      if (f.command == NO_TONE) {
        noTone(f.pin);
      }

      if (f.command == TONE) {
        if (f.ms == 0) {
          tone(f.pin, f.hz);
        } else {
          tone(f.pin, f.hz, f.ms);
          delay((int)(f.ms * 1.5));
          noTone(f.pin);
        }
      }
    }
  }
}

void onReceive(int howMany) {

  if (howMany > BUFFER_SIZE) {
    howMany = BUFFER_SIZE;
  }

  uint8_t received[howMany];

  for (int i = 0; i < howMany; i++) {
    received[i] = Wire.read();
  }

  struct frame f;

  f.command = received[0];
  f.pin = received[1];
  f.hz = (received[2] << 8) | received[3];

  if (howMany == 8) {
    f.ms = (received[4] << 24) | (received[5] << 16) | (received[6] << 8) | received[7];
  }

  buffer->add(buffer, &f);

  #if DEBUG_MODE
    Serial.print("Bytes Received: ");
    Serial.println(howMany);
    Serial.print("command: ");
    Serial.println(f.command);
    Serial.print("pin: ");
    Serial.println(f.pin);
    Serial.print("hz: ");
    Serial.println(f.hz);
    Serial.print("ms: ");
    Serial.println(f.ms);
    Serial.println("--------------------");
  #endif
}
