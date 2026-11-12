#include <Arduino.h>
#include "soc/gpio_reg.h"
#include "soc/soc.h"

const int TX_PIN = 25;
const uint32_t PULSE_US = 400;

uint32_t pulseDuration;
uint32_t cpu;

uint32_t cyc(){
    return ESP.getCycleCount();
}

void shoot(){
    REG_WRITE(GPIO_OUT_W1TS_REG, 1 << TX_PIN);  // Set TX_PIN high
    //digitalWrite(TX_PIN, HIGH);
    uint32_t t = cyc();                         // Take timestamp of pulse beginning
    Serial.print("start");
    while (cyc() - t < pulseDuration) {}        // Wait for the pulse duration
    Serial.print("end");
    REG_WRITE(GPIO_OUT_W1TC_REG, 1 << TX_PIN);  // Clear TX_PIN
    //digitalWrite(TX_PIN, LOW);
}

void setup(){
    Serial.begin(9600);
    pinMode(TX_PIN, OUTPUT);
    cpu = ESP.getCpuFreqMHz();                  // Get CPU frequency in Mhz
    pulseDuration = PULSE_US * cpu;              // Calculate pulse duration in cycles
    digitalWrite(TX_PIN, LOW);
}

void loop(){
    delay(50);  // Wait for 1 second before shooting the pulse
    shoot();
    delay(50);
}