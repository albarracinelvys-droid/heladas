#pragma once
#include <math.h>
#include "config.h"
enum class Risk { Normal, Watch, Alert, Frost, SensorError };
inline Risk classify(float c, Risk previous) {
    if (!isfinite(c) || c < -55.0f || c > 125.0f || c == 85.0f)
        return Risk::SensorError;
    if (c <= config::FROST_C || (previous == Risk::Frost && c < config::FROST_C + config::HYSTERESIS_C))
        return Risk::Frost;
    if (c <= config::ALERT_C || ((previous == Risk::Frost || previous == Risk::Alert) && c < config::ALERT_C + config::HYSTERESIS_C))
        return Risk::Alert;
    if (c <= config::WATCH_C || ((previous == Risk::Frost || previous == Risk::Alert || previous == Risk::Watch) && c < config::WATCH_C + config::HYSTERESIS_C))
        return Risk::Watch;
    return Risk::Normal;
}
inline float windSpeed(uint32_t pulses, uint32_t elapsedMs) {
    if (!pulses || !elapsedMs) return 0;
    const float value = pulses * 1000.0f / elapsedMs * config::WIND_MS_PER_HZ + config::WIND_OFFSET_MS;
    return value > 0 ? value : 0;
}
inline const char* riskName(Risk risk) {
    switch (risk) {
        case Risk::Normal: return "NORMAL";
        case Risk::Watch: return "VIGILANCIA";
        case Risk::Alert: return "ALERTA";
        case Risk::Frost: return "HELADA";
        default: return "ERROR_SENSOR";
    }
}
