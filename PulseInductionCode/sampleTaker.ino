#include <Arduino.h>
#include "soc/gpio_reg.h"
#include "soc/soc.h"

const int TX_PIN     = 26;            // Pin to drive PI coil
const int STROBE_PIN = 27;            // Pin to strobe S/Hs
const int COMP_PIN   = 14;            // Should turn high when flat part of response ends
const int ADC_PIN    = 34;            // read mux output
const int VLF_PIN    = 25;            // VLF input 
const int SEL_PIN[3] = {18, 19, 23};  // Channel select pins (shared by both muxes)

const int      N_GATES = 8;
const uint32_t DELAY_US[N_GATES] = {0, 5, 10, 20, 40, 80, 160, 320};  // after flat part
const uint32_t GATE_US         = 5;    // strobe width
const uint32_t PULSE_US        = 100;  // TX on time
const uint32_t FLAT_TIMEOUT_US = 1000000;  // give up if the comparator never flips
const int  READS     = 4;      // ADC reads per channel 
const long REPEAT_MS = 2000;   // time between measurements

uint32_t cpu;                                   // CPU cycles per microsecond
uint32_t delayCyc[N_GATES], gateCyc, pulseCyc, timeoutCyc;
uint32_t selSet[N_GATES], selClr[N_GATES];      // gpio masks

inline uint32_t cyc() { 
  return ESP.getCycleCount(); 
}

inline void setChannel(int k) {
  REG_WRITE(GPIO_OUT_W1TC_REG, selClr[k]);
  REG_WRITE(GPIO_OUT_W1TS_REG, selSet[k]);
}

// Shoot TX pulse. Return false if comparator never flips (flat part never ends).
bool shoot(float &flatUs) {
  setChannel(0);                              // Select first gate
  noInterrupts();                             // Disable interrupts to avoid other processes messing up timing

  REG_WRITE(GPIO_OUT_W1TS_REG, 1 << TX_PIN);  // Set PI TX high
  uint32_t t = cyc();                         // Take timestamp of pulse beginning
  while (cyc() - t < pulseCyc) {}             // Wait for pulse duration
  REG_WRITE(GPIO_OUT_W1TC_REG, 1 << TX_PIN);  // Set PI TX low
  uint32_t t0 = cyc();                        // Take timestamp of pulse ending

  // Wait for the end of the flat part. Don't leave loop until comparator pin goes high.
  while (!((REG_READ(GPIO_IN_REG) >> COMP_PIN) & 1)) { 
    if (cyc() - t0 > timeoutCyc) { 
      interrupts();                           // Re-enable interrupts and go back to main loop
      return false;                           
    }
  }
  uint32_t flatEnd = cyc();                   // Take timestamp of flat part ending

  // Strobe each S/H at its delay after the flat part
  for (int k = 0; k < N_GATES; k++) {
    setChannel(k);                                  // Select gate k for sampling
    while (cyc() - flatEnd < delayCyc[k]) {}        // Wait for the delay of gate k
    REG_WRITE(GPIO_OUT_W1TS_REG, 1 << STROBE_PIN);  // Send pulse to gate k
    uint32_t s = cyc();                             // Take timestamp of strobe beginning
    while (cyc() - s < gateCyc) {}                  // Wait for the strobe duration (5 us, probably increase to 10us)
    REG_WRITE(GPIO_OUT_W1TC_REG, 1 << STROBE_PIN);  // Clear strobe pin
  }

  interrupts();                                     // Re-enable interrupts     
  flatUs = (flatEnd - t0) / (float)cpu;             // Duration of flat part
  return true;
}

// Read all 8 held voltages (mV) through the read mux.
void readAll(float mv[N_GATES]) {
  for (int k = 0; k < N_GATES; k++) {
    setChannel(k);                                  // Select gate k for reading
    delayMicroseconds(2);                           // Wait for mux to settle
    long sum = 0;                 
    for (int i = 0; i < READS; i++) {               // Read ADC a few times to eliminate errors
      sum += analogReadMilliVolts(ADC_PIN);             
    }
    mv[k] = sum / (float)READS;                     // Average ADC readings
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(TX_PIN, OUTPUT);
  pinMode(STROBE_PIN, OUTPUT);
  pinMode(COMP_PIN, INPUT);

  uint32_t all = 0;
  for (int b = 0; b < 3; b++) { 
    pinMode(SEL_PIN[b], OUTPUT); 
    all |= 1 << SEL_PIN[b]; 
  }
  for (int k = 0; k < N_GATES; k++) {
    selSet[k] = 0;
    for (int b = 0; b < 3; b++) {
      if (k & (1 << b)) {
        selSet[k] |= 1 << SEL_PIN[b];
      }
    }
    selClr[k] = all & ~selSet[k];
  }

  analogSetPinAttenuation(ADC_PIN, ADC_11db);

  cpu = getCpuFrequencyMhz();
  for (int k = 0; k < N_GATES; k++) delayCyc[k] = DELAY_US[k] * cpu;
  gateCyc = GATE_US * cpu;
  pulseCyc = PULSE_US * cpu;
  timeoutCyc = FLAT_TIMEOUT_US * cpu;
}

void loop() {
  float flat;
  float mv[N_GATES];

  if (!shoot(flat)) {                       // Take pulse
    Serial.println("no flat-end detected"); // If decay never happens, failure message
    return;
  }

  readAll(mv);                              // Read the held voltages

  Serial.printf("flat_us=%.2f\n", flat);
  for (int k = 0; k < N_GATES; k++)         // time since TX-off (us), volts
    Serial.printf("%.2f,%.4f\n", flat + DELAY_US[k] + GATE_US, mv[k] / 1000.0);
  Serial.println("---");

  delay(REPEAT_MS);
}