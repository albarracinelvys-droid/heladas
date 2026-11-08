#pragma once
#include <stdint.h>
namespace config {
constexpr uint8_t WIND_PIN = 2;
constexpr uint8_t TEMPERATURE_PIN = 4;
constexpr uint8_t GREEN_PIN = 6;
constexpr uint8_t YELLOW_PIN = 7;
constexpr uint8_t RED_PIN = 8;
constexpr uint8_t BUZZER_PIN = 9;
constexpr uint32_t SAMPLE_MS = 2000;
constexpr uint32_t DEBOUNCE_US = 5000;
// EJEMPLO: sustituir por la pendiente de la ficha tecnica o calibracion.
constexpr float WIND_MS_PER_HZ = 1.0f;
constexpr float WIND_OFFSET_MS = 0.0f;
constexpr float WATCH_C = 4.0f;
constexpr float ALERT_C = 2.0f;
constexpr float FROST_C = 0.0f;
constexpr float HYSTERESIS_C = 0.5f;
constexpr float LOW_WIND_MS = 2.0f;
}
