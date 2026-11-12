const int LEDCPin = 12;
const int STROBE_PIN = 27;            // HIGH = selected S/H tracks

uint32_t targetFreq = 100;
uint8_t resolution = 12;
uint32_t maxDutySteps = (1 << resolution) - 1;
uint32_t dutyCycle = maxDutySteps * 0.25;

const uint32_t GATE_US = 10;

uint32_t cpu;                                   // CPU cycles per microsecond
uint32_t gateCyc;

uint32_t cyc() { 
  return ESP.getCycleCount(); 
}

void sample() {
    REG_WRITE(GPIO_OUT_W1TS_REG, 1 << STROBE_PIN); 
    uint32_t t0 = cyc();                            // Take timestamp of strobe starting
    while (cyc() - t0 < gateCyc) {}                 // Wait for strobe duration
    REG_WRITE(GPIO_OUT_W1TC_REG, 1 << STROBE_PIN);  // Clear strobe pin
}

void setup() {
  Serial.begin(115200);
  pinMode(STROBE_PIN, OUTPUT);

  ledcAttach(LEDCPin, targetFreq, resolution);
  ledcWrite(LEDCPin, dutyCycle);

  cpu = ESP.getCpuFreqMHz();  // CPU frequency in MHz
  gateCyc = GATE_US * cpu;     // Convert gate time from microseconds
}

void loop() {
  // The square wave runs in hardware; no code needed here.
  if(Serial.available() > 0)
  {
    String input = Serial.readStringUntil('\n');
    input.trim();
    input.toUpperCase();

      if(input == "TAKESAMPLE"){
        sample();
        Serial.println("Sample taken");
      }
  }
}
