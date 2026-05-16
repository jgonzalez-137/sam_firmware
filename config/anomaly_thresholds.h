#pragma once

namespace config {

// Aceleracion anomala
constexpr float ACCEL_ALERT_G = 2.5f;

// Deteccion simple de latido por IR del MAX30102
constexpr unsigned int PPG_IR_BEAT_THRESHOLD = 20000U;
constexpr unsigned int BEAT_REFRACTORY_MS = 300U;

// Limites de ritmo cardiaco
constexpr float BRADY_BPM = 45.0f;
constexpr float TACHY_BPM = 120.0f;

// Ventanas temporales
constexpr unsigned int HR_PAUSE_MS = 2500U;
constexpr unsigned int HR_SUSTAIN_MS = 8000U;

} // namespace config
