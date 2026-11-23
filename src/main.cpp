#include <Arduino.h>

// void setup() {
//   pinMode(LED_BUILTIN, OUTPUT); // Initialize pin 13 as an output
// }

// void loop() {
//   digitalWrite(LED_BUILTIN, HIGH); // Turn the LED on (HIGH is voltage level)
//   delay(500);                      // Wait for a second (1000 milliseconds)
//   digitalWrite(LED_BUILTIN, LOW);  // Turn the LED off by making the voltage
//   0 delay(500);                      // Wait for a second
// }

/*
  Simple Waveform generator with Arduino Due

  * connect two push buttons to the digital pins 2 and 3
    with a 10 kilohm pulldown resistor to choose the waveform
    to send to the DAC0 and DAC1 channels
  * connect a 10 kilohm potentiometer to A0 to control the
    signal frequency

 */

// #include "Waveforms.h"

// #define oneHzSample                                                            \
//   1000000 / maxSamplesNum // sample for the 1Hz signal expressed in microseconds

// const int button0 = 2, button1 = 3;
// volatile int wave0 = 0, wave1 = 0;

// int i = 0;
// int sample = 10;

// // function hooked to the interrupt on digital pin 2
// void wave0Select() {
//   wave0++;
//   if (wave0 == 4)
//     wave0 = 0;
// }

// // function hooked to the interrupt on digital pin 3
// void wave1Select() {
//   wave1++;
//   if (wave1 == 4)
//     wave1 = 0;
// }

// void setup() {
//   analogWriteResolution(
//       12); // set the analog output resolution to 12 bit (4096 levels)
//   analogReadResolution(12); // set the analog input resolution to 12 bit
// }

// void loop() {
//   // Read the the potentiometer and map the value  between the maximum and
//   the
//   // minimum sample available 1 Hz is the minimum freq for the complete wave
//   170
//   // Hz is the maximum freq for the complete wave. Measured considering the
//   loop
//   // and the analogRead() time
//   // sample = map(analogRead(A0), 0, 4095, 0, oneHzSample);
//   // sample = constrain(sample, 0, oneHzSample);

//   analogWrite(DAC0,
//               waveformsTable[wave0][i]); // write the selected waveform on
//               DAC0

//   i++;
//   if (i >= maxSamplesNum) { // Must be 120 (maxSamplesNum), not 40
//     i = 0;
//   }

//   delayMicroseconds(sample); // Required so the DAC can settle cleanly

//   // delayMicroseconds(sample); // Hold the sample value for the sample time
// }

// const int wavePin = 22;
// const unsigned long highTimeUs = 25000; // 25 ms HIGH (10 Hz total)
// const unsigned long lowTimeUs = 75000;  // 75 ms LOW
// // us LOW(1000 us period = 1 kHz, 25 % duty cycle)

// unsigned long previousMicros = 0;
// bool pinState = LOW;

// void setup() {
//   pinMode(wavePin, OUTPUT);
//   // digitalWrite(wavePin, pinState);
// }

// void loop() {
//   unsigned long currentMicros = micros();
//   unsigned long interval = pinState ? highTimeUs : lowTimeUs;

//   digitalWrite(wavePin, HIGH);
//   delay(1);
//   digitalWrite(wavePin, LOW);
//   delay(1);

//   // if (currentMicros - previousMicros >= interval) {
//   //   previousMicros = currentMicros;
//   //   pinState = !pinState;
//   //   digitalWrite(wavePin, pinState);
//   // }
// }

//////////////////////////////////////////////////////////////////////V2
// const int wavePin = 22;
// const unsigned long highTimeUs = 1; // 250 us HIGH
// const unsigned long lowTimeUs =
//     1; // 750 us LOW (1000 us period = 1 kHz, 25% duty cycle)

// unsigned long previousMicros = 0;
// bool pinState = LOW;

// void setup() {
//   pinMode(wavePin, OUTPUT);
//   digitalWrite(wavePin, pinState);
// }

// void loop() {
//   unsigned long currentMicros = micros();
//   unsigned long interval = pinState ? highTimeUs : lowTimeUs;

//   if (currentMicros - previousMicros >= interval) {
//     previousMicros = currentMicros;
//     pinState = !pinState;
//     digitalWrite(wavePin, pinState);
//   }
// }

////////////////////////////////////////////////////////////// V3

// const int wavePin = 2; // D22 = port B, bit 26 on the Due

// const unsigned long highTimeCyc = 140; // 140 cycles HIGH = 1.667 us
// const unsigned long lowTimeCyc = 140;  // 420 cycles LOW  = 5.000 us
// // 560 cycles at 84 MHz = 6.667 us period = 150 kHz, 25 % duty cycle

// #define CYCCNT (*(volatile uint32_t *)0xE0001004) // CPU cycle counter (84
// MHz)

// unsigned long previousCyc = 0;
// bool pinState = LOW;

// void setup() {
//   pinMode(wavePin, OUTPUT);
//   digitalWrite(wavePin, pinState);

//   *(volatile uint32_t *)0xE000EDFC |= (1UL << 24); // enable the cycle
//   counter
//   *(volatile uint32_t *)0xE0001000 |= 1UL;
//   previousCyc = CYCCNT;
// }

// void loop() {
//   __disable_irq(); // Disable interrupts
//                    // Critical code

//   for (;;) {
//     unsigned long interval = pinState ? highTimeCyc : lowTimeCyc;

//     if (CYCCNT - previousCyc >= interval) {
//       previousCyc += interval; // fixed time grid -> no drift
//       pinState = !pinState;
//       if (pinState)
//         PIOB->PIO_SODR = PIO_PB26; // pin HIGH (fast digitalWrite)
//       else
//         PIOB->PIO_CODR = PIO_PB26; // pin LOW
//     }
//   }
//   __enable_irq(); // Re-enable interrupts
// }

////////////////////////////////////////////////////////////// v4

// Arduino Due Pin Mappings:
// Digital Pin 22 -> Port B, Bit 26 (PIO_PB26) - Output
// Digital Pin 23 -> Port A, Bit 14 (PIO_PA14) - Filtered Input

const int wavePin = 22;
const int inputPin = 23;

const uint32_t WAVE_PIN_MASK = PIO_PB26;
const uint32_t INPUT_PIN_MASK = PIO_PA14;

const unsigned long highTimeCyc = 140; // 140 cycles HIGH = 1.667 us
const unsigned long lowTimeCyc = 140;  // 140 cycles LOW  = 1.667 us
// Total = 280 cycles at 84 MHz = 3.333 us period = 300 kHz, 50% duty cycle

#define CYCCNT (*(volatile uint32_t *)0xE0001004) // CPU cycle counter (84 MHz)

unsigned long previousCyc = 0;
bool pinState = false;

void setup() {
  // 1. Configure Output Pin 22 (PB26) for direct atomic PIO control
  pinMode(wavePin, OUTPUT);
  PIOB->PIO_PER = WAVE_PIN_MASK; // Enable direct PIO control
  PIOB->PIO_OER = WAVE_PIN_MASK; // Enable Output
  PIOB->PIO_CODR =
      WAVE_PIN_MASK; // Initialize LOW atomically (replaces digitalWrite)

  // 2. Configure Input Pin 23 (PA14) with Hardware Glitch Filter
  pinMode(inputPin, INPUT);
  PIOA->PIO_PER = INPUT_PIN_MASK;    // Enable direct PIO control
  PIOA->PIO_ODR = INPUT_PIN_MASK;    // Set as Input
  PIOA->PIO_IFER = INPUT_PIN_MASK;   // Enable Input Glitch Filter
  PIOA->PIO_SCIFSR = INPUT_PIN_MASK; // Select System Clock filter (blocks fast
                                     // reflection spikes)

  // 3. Enable ARM Cortex-M3 DWT Cycle Counter
  *(volatile uint32_t *)0xE000EDFC |= (1UL << 24); // DEMCR: Enable trace/DWT
  *(volatile uint32_t *)0xE0001000 |= 1UL;         // DWT_CTRL: Enable CYCCNT
  previousCyc = CYCCNT;
}

void loop() {
  __disable_irq(); // Disable all interrupts for zero-jitter timing

  // Continuous 300 kHz generation loop
  for (;;) {
    unsigned long interval = pinState ? highTimeCyc : lowTimeCyc;

    if ((CYCCNT - previousCyc) >= interval) {
      previousCyc += interval; // Fixed time grid -> zero cumulative drift
      pinState = !pinState;

      // Atomic Set/Clear writes (single bus cycle, no read-modify-write
      // glitches)
      if (pinState) {
        PIOB->PIO_SODR = WAVE_PIN_MASK; // Pin 22 HIGH
      } else {
        PIOB->PIO_CODR = WAVE_PIN_MASK; // Pin 22 LOW
      }
    }

    // Optional: Fast, glitch-filtered read of Pin 23 without breaking timing
    // bool filteredState = (PIOA->PIO_PDSR & INPUT_PIN_MASK) != 0;
  }

  // Note: Unreachable unless you add a break condition to the for(;;) loop
  __enable_irq();
}
