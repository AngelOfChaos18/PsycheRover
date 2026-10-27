const int SIG_PIN  = 99;   // detector output (placeholder)
const int WAKE_PIN = 99;   // turns on the PI stage (placeholder)

const float THRESHOLD = 40;     // mV away from baseline that counts as metal
const int   PI_ON_MS  = 200;    // how long the PI stage stays on
const int   COOLDOWN  = 500;    // ms before it can trigger again

float baseline;
unsigned long lastTrigger = 0;

// take 16 samples and average (eliminate some of the noise)
float readMv() {
  long sum = 0;
  for (int i = 0; i < 16; i++) {
    sum += analogReadMilliVolts(SIG_PIN);
  }
  return sum / 16.0;
}

void setup() {
  Serial.begin(115200);
  pinMode(WAKE_PIN, OUTPUT);

  // sets adc range to 0-3.1V
  analogSetPinAttenuation(SIG_PIN, ADC_11db);

  // set the baseline with the coil in the air
  baseline = readMv();
}

void loop() {
  float v = readMv();

  if (fabs(v - baseline) > THRESHOLD) { // get absolute value
    if (millis() - lastTrigger > COOLDOWN) { // debounce (had bad experiences without it)
      digitalWrite(WAKE_PIN, HIGH);
      delay(PI_ON_MS);
      digitalWrite(WAKE_PIN, LOW);
      lastTrigger = millis();
    }
  } else {
    baseline += 0.002 * (v - baseline);   // account for drift that might happen. We can eliminate this if need
  }

  Serial.println(v);
  delay(5);
}
