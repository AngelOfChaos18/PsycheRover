const int TX_PIN   = 26;   
const int SH_PIN   = 27;   
const int COMP_PIN = 14;   // comparator for indicating flat part of the signal
const int ADC_PIN  = 34;  

const int PULSE_US        = 100;   // check pi pulse length
const int GATE_US         = 10;     
const int FLAT_TIMEOUT_US = 300;   // placeholder
const int PULSES_PER_GATE = 64;

const int N_GATES = 8;  // # of samples
const uint32_t DELAY_US[N_GATES] = {0, 5, 10, 20, 40, 80, 160, 320};  // after the flat part

uint32_t cpu;   // CPU cycles per us

uint32_t cyc() { 
  return ESP.getCycleCount(); 
}

void waitCycles(uint32_t start, uint32_t n) { 
  while (cyc() - start < n) {} 
  return;
}

// Literally "shoots" a pulse. i.e. triggers PI
bool shoot(int k, float &flatUs, float &mv) {
  // Stop all other processes
  noInterrupts();

  REG_WRITE(GPIO_OUT_W1TS_REG, 1 << TX_PIN); // Set TX high for PULSE_US microseconds
  waitCycles(cyc(), PULSE_US * cpu);
  REG_WRITE(GPIO_OUT_W1TC_REG, 0 << TX_PIN); // Turn off, start counting
  uint32_t t0 = cyc();                       

  // Measure the "flat part" of the signal. Wait for comparator trigger.
  while (!((REG_READ(GPIO_IN_REG) >> COMP_PIN) & 1)) {
    // If flat part takes too long, timeout. Enable interrupts again
    if (cyc() - t0 > FLAT_TIMEOUT_US * cpu) { 
      interrupts(); 
      return false; 
    }
  }
  uint32_t tFlat = cyc() - t0;

  /*

    Fire S/H after flat part ends. Decide if use DEMUX or different pins
    Do something like this, but demultiplexing. Not implemented yet.
      waitCycles(t0 + tFlat, DELAY_US[k] * cpu);
      REG_WRITE(GPIO_OUT_W1TS_REG, 1 << SH_PIN);
      waitCycles(cyc(), GATE_US * cpu);
      REG_WRITE(GPIO_OUT_W1TC_REG, 1 << SH_PIN);
  */
  

  // Enable interrupts again.
  interrupts();

  // Divide cycles by frequency to get microseconds. Read the held voltage.
  flatUs = tFlat / (float)cpu;

  /*
    Read voltages. Implement binary counter for muxxing
  */
  
}

void setup() {
  Serial.begin(115200);
  pinMode(TX_PIN, OUTPUT);
  pinMode(SH_PIN, OUTPUT);
  pinMode(COMP_PIN, INPUT);
  analogSetPinAttenuation(ADC_PIN, ADC_11db);
  cpu = getCpuFrequencyMhz();
}

void loop() {
  float flatSum = 0, volts[N_GATES];
  int flatCount = 0;


  /*
    Copilot suggestion (??). Don't like the pulses_per_gate loop
  */
  for (int k = 0; k < N_GATES; k++) {
    float vSum = 0;
    int n = 0;
    for (int p = 0; p < PULSES_PER_GATE; p++) {
      float f, v;
      if (shoot(k, f, v)) { flatSum += f; flatCount++; vSum += v; n++; }
      delay(2);                               // let the coil settle
    }
    volts[k] = n ? vSum / n / 1000.0 : NAN;
  }

  // SEND DATA VIA BT

  delay(2000);
}