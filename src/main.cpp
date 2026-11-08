#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <util/atomic.h>
#include "config.h"
#include "risk.h"

OneWire bus(config::TEMPERATURE_PIN);
DallasTemperature thermometer(&bus);
DeviceAddress address;
bool sensorFound = false;
volatile uint32_t pulseCount = 0;
volatile uint32_t lastPulseUs = 0;
volatile bool hasPulse = false;
uint32_t lastSample = 0;
Risk state = Risk::SensorError;

void onWindPulse() {
    const uint32_t now = micros();
    if (!hasPulse || uint32_t(now - lastPulseUs) >= config::DEBOUNCE_US) {
        ++pulseCount;
        lastPulseUs = now;
        hasPulse = true;
    }
}

void requestTemperature() {
    // Buscar de nuevo permite recuperar un sensor reconectado.
    thermometer.begin();
    sensorFound = thermometer.getAddress(address, 0);
    if (sensorFound) thermometer.setResolution(address, 12);
    thermometer.setWaitForConversion(false);
    thermometer.requestTemperatures();
}

void indicators(uint32_t now) {
    digitalWrite(config::GREEN_PIN, state == Risk::Normal);
    digitalWrite(config::YELLOW_PIN, state == Risk::Watch || state == Risk::Alert);
    digitalWrite(config::RED_PIN, state == Risk::Frost || (state == Risk::SensorError && (now / 250) % 2));
    const bool alarm = state == Risk::Alert || state == Risk::Frost || state == Risk::SensorError;
    if (alarm && now % 2000 < 200) tone(config::BUZZER_PIN, state == Risk::SensorError ? 500 : 1800);
    else noTone(config::BUZZER_PIN);
}

void setup() {
    Serial.begin(9600);
    pinMode(config::WIND_PIN, INPUT_PULLUP);
    const uint8_t outputs[] = {config::GREEN_PIN, config::YELLOW_PIN, config::RED_PIN, config::BUZZER_PIN};
    for (uint8_t pin : outputs) pinMode(pin, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(config::WIND_PIN), onWindPulse, FALLING);
#if DEMO_MODE
    Serial.println(F("# DEMOSTRACION: datos simulados, no usar para vigilancia real"));
#else
    requestTemperature();
#endif
    Serial.println(F("# Viento sin calibrar: ajustar WIND_MS_PER_HZ en config.h"));
    Serial.println(F("tiempo_ms,temperatura_c,viento_ms,pulsos,hz,estado,viento_debil"));
    lastSample = millis();
}

void loop() {
    uint32_t now = millis();
    indicators(now);
    const uint32_t elapsed = now - lastSample;
    if (elapsed < config::SAMPLE_MS) return;
    lastSample = now;
    uint32_t pulses;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { pulses = pulseCount; pulseCount = 0; }
#if DEMO_MODE
    static uint8_t step = 0;
    const float examples[] = {8, 4, 2, 0, -2, 0.2f, 2.5f, 4.5f, NAN};
    const float temperature = examples[step++ % 9];
    pulses = 2;
#else
    const float temperature = sensorFound ? thermometer.getTempC(address) : NAN;
#endif
    const float wind = windSpeed(pulses, elapsed);
    state = classify(temperature, state);
    Serial.print(now); Serial.print(',');
    if (state == Risk::SensorError) Serial.print(F("NA")); else Serial.print(temperature, 2);
    Serial.print(','); Serial.print(wind, 2);
    Serial.print(','); Serial.print(pulses);
    Serial.print(','); Serial.print(pulses * 1000.0f / elapsed, 3);
    Serial.print(','); Serial.print(riskName(state));
    Serial.print(','); Serial.println(wind < config::LOW_WIND_MS ? 1 : 0);
#if !DEMO_MODE
    requestTemperature();
#endif
    indicators(millis());
}
