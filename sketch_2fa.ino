#include <WiFi.h>
#include "time.h"
#include <sys/time.h>
#include "TOTP.h"
#include <Arduino_GFX_Library.h>
#include "secrets.h"

// Activa esta opción únicamente después de instalar RTClib y conectar un DS3231.
#define USE_DS3231 0
#if USE_DS3231
#include <Wire.h>
#include <RTClib.h>
RTC_DS3231 rtc;
#endif

// Pantalla TFT ST7789 (Deskbuddy).
#define TFT_BL 22
Arduino_DataBus *bus = new Arduino_ESP32SPI(15, 14, 7, 6, GFX_NOT_DEFINED, 20000000L);
Arduino_GFX *gfx = new Arduino_ST7789(bus, 21, 3, true, 172, 320, 34, 0, 34, 0);

constexpr uint32_t WIFI_TIMEOUT_MS = 15000;
constexpr uint32_t NTP_TIMEOUT_MS = 15000;
constexpr uint32_t RESYNC_INTERVAL_MS = 6UL * 60UL * 60UL * 1000UL;
constexpr uint32_t RETRY_INTERVAL_MS = 60UL * 1000UL;
constexpr time_t MIN_VALID_EPOCH = 1704067200;  // 2024-01-01 UTC
constexpr time_t MAX_CLOCK_DRIFT_SECONDS = 5;
const char *ntpServer = "pool.ntp.org";

uint8_t hmacKey[] = TOTP_HMAC_KEY;
TOTP totp(hmacKey, sizeof(hmacKey));

bool timeIsTrusted = false;
uint32_t lastSyncAttemptMs = 0;
uint32_t lastSuccessfulSyncMs = 0;
uint32_t clockAnchorMs = 0;
time_t clockAnchorEpoch = 0;
int lastDisplayedSecond = -1;
time_t lastDisplayedPeriod = -1;
bool lastCodeWasWarning = false;
bool codeScreenReady = false;
bool startupActive = true;

void showMessage(const char *line1, const char *line2 = nullptr, uint16_t color = 0x07FF) {
  codeScreenReady = false;
  gfx->fillScreen(0x0000);
  gfx->setTextColor(color);
  gfx->setTextSize(2);
  gfx->setCursor(12, 55);
  gfx->print(line1);
  if (line2 != nullptr) {
    gfx->setCursor(12, 90);
    gfx->print(line2);
  }
}

void drawStartup() {
  // Estilo de arranque: logotipo cian y firma pequeña, sin barra.
  codeScreenReady = false;
  gfx->fillScreen(0x0000);

  constexpr int screenWidth = 320;
  constexpr int titleSize = 6;
  constexpr int titleWidth = 3 * 6 * titleSize;  // 3 caracteres, fuente de 6 px.
  constexpr int subtitleSize = 1;
  constexpr int subtitleWidth = 7 * 6 * subtitleSize;
  constexpr int titleY = 46;
  constexpr int subtitleY = 105;

  gfx->setTextColor(0x07FF);
  gfx->setTextSize(titleSize);
  gfx->setCursor((screenWidth - titleWidth) / 2, titleY);
  gfx->print("2FA");

  gfx->setTextColor(0xFFFF);
  gfx->setTextSize(subtitleSize);
  gfx->setCursor((screenWidth - subtitleWidth) / 2, subtitleY);
  gfx->print("BY MAVD");
}

void disableWiFi() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

bool connectWiFi() {
  if (startupActive) drawStartup();
  else showMessage("Conectando", "Wi-Fi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  const uint32_t started = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - started < WIFI_TIMEOUT_MS) {
    delay(100);
  }
  return WiFi.status() == WL_CONNECTED;
}

bool epochIsReasonable(time_t value) {
  return value >= MIN_VALID_EPOCH;
}

#if USE_DS3231
bool seedTimeFromRtc() {
  if (!rtc.begin() || rtc.lostPower()) return false;
  time_t rtcEpoch = rtc.now().unixtime();
  if (!epochIsReasonable(rtcEpoch)) return false;
  timeval tv = {rtcEpoch, 0};
  settimeofday(&tv, nullptr);
  return true;
}
#endif

bool synchronizeTime() {
  lastSyncAttemptMs = millis();
  if (!connectWiFi()) {
    disableWiFi();
    return false;
  }
  if (startupActive) drawStartup();
  else showMessage("Sincronizando", "hora...");
  configTime(0, 0, ntpServer);
  struct tm timeInfo;
  const uint32_t ntpStarted = millis();
  bool receivedTime = false;
  while (!receivedTime && millis() - ntpStarted < NTP_TIMEOUT_MS) {
    receivedTime = getLocalTime(&timeInfo, 100);
    if (!receivedTime) delay(50);
  }
  time_t now;
  time(&now);
  disableWiFi();
  if (!receivedTime || !epochIsReasonable(now)) return false;

#if USE_DS3231
  rtc.adjust(DateTime(static_cast<uint32_t>(now)));
#endif

  clockAnchorEpoch = now;
  clockAnchorMs = millis();
  lastSuccessfulSyncMs = clockAnchorMs;
  timeIsTrusted = true;
  return true;
}

bool clockStillTrusted() {
  if (!timeIsTrusted) return false;
  time_t now;
  time(&now);
  const time_t expected = clockAnchorEpoch + ((millis() - clockAnchorMs) / 1000UL);
  time_t difference = now - expected;
  if (difference < 0) difference = -difference;
  if (!epochIsReasonable(now) || difference > MAX_CLOCK_DRIFT_SECONDS) timeIsTrusted = false;
  return timeIsTrusted;
}

void drawCode() {
  time_t now;
  time(&now);
  int remaining = 30 - static_cast<int>(now % 30);
  if (codeScreenReady && remaining == lastDisplayedSecond) return;

  constexpr int timerBarX = 35;
  constexpr int timerBarY = 137;
  constexpr int timerBarWidth = 250;
  constexpr int timerBarHeight = 13;

  // Los elementos fijos se dibujan una sola vez para evitar parpadeo.
  if (!codeScreenReady) {
    gfx->fillScreen(0x0000);
    gfx->drawRoundRect(5, 5, 310, 162, 12, 0x07FF);
    gfx->setTextSize(2);
    gfx->setTextColor(0x07FF);
    gfx->setCursor(20, 25);
    gfx->print("CODIGO 2FA");
    gfx->drawRoundRect(timerBarX, timerBarY, timerBarWidth, timerBarHeight, 3, 0xFFFF);
    codeScreenReady = true;
    lastDisplayedPeriod = -1;
    lastDisplayedSecond = -1;
  }

  const time_t currentPeriod = now / 30;
  const bool warning = remaining <= 5;
  if (currentPeriod != lastDisplayedPeriod || warning != lastCodeWasWarning) {
    char *code = totp.getCode(now);
    // Solo se limpia el rectángulo ocupado por los seis dígitos.
    gfx->fillRect(15, 62, 290, 64, 0x0000);
    gfx->setTextSize(6);
    gfx->setTextColor(warning ? 0xF800 : 0xFFFF);
    gfx->setCursor(20, 70);
    gfx->print(code);
    lastDisplayedPeriod = currentPeriod;
    lastCodeWasWarning = warning;
  }

  // Cada segundo se actualiza únicamente el interior de la barra.
  gfx->fillRect(timerBarX + 3, timerBarY + 3,
                timerBarWidth - 6, timerBarHeight - 6, 0x0000);
  const int timerFill = ((timerBarWidth - 6) * remaining) / 30;
  if (timerFill > 0) {
    gfx->fillRect(timerBarX + 3, timerBarY + 3, timerFill, timerBarHeight - 6,
                  warning ? 0xF800 : 0x07FF);
  }
  lastDisplayedSecond = remaining;
}

void setup() {
  Serial.begin(115200);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  gfx->begin();
  drawStartup();
  delay(700);

#if USE_DS3231
  timeIsTrusted = seedTimeFromRtc();
  if (timeIsTrusted) {
    time(&clockAnchorEpoch);
    clockAnchorMs = millis();
  }
#endif

  if (!synchronizeTime() && !timeIsTrusted) {
    startupActive = false;
    showMessage("Hora no valida", "Reintento en 60s", 0xF800);
  } else {
    startupActive = false;
    lastDisplayedSecond = -1;
    drawCode();
  }
}

void loop() {
  const uint32_t nowMs = millis();
  if (timeIsTrusted &&
      nowMs - lastSuccessfulSyncMs >= RESYNC_INTERVAL_MS &&
      nowMs - lastSyncAttemptMs >= RETRY_INTERVAL_MS) {
    synchronizeTime();
    lastDisplayedSecond = -1;
  } else if (!timeIsTrusted && nowMs - lastSyncAttemptMs >= RETRY_INTERVAL_MS) {
    if (synchronizeTime()) lastDisplayedSecond = -1;
    else showMessage("Sin hora fiable", "Reintento en 60s", 0xF800);
  }

  if (clockStillTrusted()) {
    drawCode();
  } else if (lastDisplayedSecond != -2) {
    lastDisplayedSecond = -2;
    showMessage("Hora no valida", "Sin codigo 2FA", 0xF800);
  }
  delay(25);
}
