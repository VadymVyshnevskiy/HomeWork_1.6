#include <Arduino.h>

constexpr uint8_t LDR_PIN = 2;
constexpr uint8_t LED_PIN = 17;
constexpr uint16_t DARK_THRESHOLD_RAW = 2048;
constexpr uint16_t HYSTERESIS_RAW = 100;
constexpr uint32_t SAMPLE_PERIOD_MS = 100;
constexpr float ADC_MAX = 4095.0f;
constexpr float U_REF_MV = 3100.0f;
constexpr uint8_t HEADER_EVERY_ROWS = 100;

void printHeader() {
  Serial.println();
  Serial.println("+--------+----------+-------------+-------------+-----------+-----+");
  Serial.println("|   #    |   RAW    | U_calc (mV) | U_meas (mV) | Error (%) | LED |");
  Serial.println("+--------+----------+-------------+-------------+-----------+-----+");
}

bool isDark(uint16_t raw, bool ledOn) {
  const uint16_t threshold = ledOn ? DARK_THRESHOLD_RAW + HYSTERESIS_RAW : DARK_THRESHOLD_RAW - HYSTERESIS_RAW;
  return raw < threshold;
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetPinAttenuation(LDR_PIN, ADC_11db);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  delay(500);
  printHeader();
}

void loop() {
  static uint32_t lastSampleMs = 0;
  static uint32_t sampleNo = 0;
  static bool ledOn = false;

  const uint32_t now = millis();
  if (now - lastSampleMs < SAMPLE_PERIOD_MS) {
    return;
  }
  lastSampleMs = now;

  const uint16_t raw = analogRead(LDR_PIN);
  const float uCalcMv = raw / ADC_MAX * U_REF_MV;
  const uint32_t uMeasMv = analogReadMilliVolts(LDR_PIN);
  const float errorPct = uMeasMv > 0 ? fabsf(uCalcMv - uMeasMv) / uMeasMv * 100.0f : 0.0f;

  ledOn = isDark(raw, ledOn);
  digitalWrite(LED_PIN, ledOn ? HIGH : LOW);

  if (sampleNo > 0 && sampleNo % HEADER_EVERY_ROWS == 0) {
    printHeader();
  }
  sampleNo++;

  Serial.printf("| %6lu | %8u | %11.1f | %11lu | %9.2f | %-3s |\n",
                sampleNo, raw, uCalcMv, uMeasMv, errorPct, ledOn ? "ON" : "OFF");
}
