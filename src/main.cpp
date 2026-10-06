#include <WiFi.h>
#include <time.h>
#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_NeoPixel.h>

// ==================== CONFIGURATION ====================
#define LED_PIN        18      // GPIO Pin connected to Data In (DI) of the LED ring
#define NUM_LEDS       60      // Total number of LEDs in the ring
#define BRIGHTNESS     40      // Global brightness level (0 - 255)

// Wi-Fi Credentials
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Timezone & NTP Settings (Europe/Berlin with daylight saving support)
const char* NTP_SERVER    = "pool.ntp.org";
const char* TIME_ZONE     = "CET-1CEST,M3.5.0,M10.5.0/3"; 
// =======================================================

RTC_DS3231 rtc;
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- Initializing ESP32 LED Ring Clock ---");

  // 1. Initialize NeoPixel Ring
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show(); // Clear ring on startup

  // 2. Initialize RTC
  Wire.begin(21, 22); // SDA = GPIO 21, SCL = GPIO 22
  if (!rtc.begin()) {
    Serial.println("Error: DS3231 RTC module not detected on I2C bus!");
  } else {
    Serial.println("RTC module detected.");
  }

  // 3. Connect to Wi-Fi and Synchronize Time via NTP
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 15) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected successfully!");
    Serial.println("Fetching NTP time and syncing RTC...");
    
    configTzTime(TIME_ZONE, NTP_SERVER);
    
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      // Sync hardware RTC with updated NTP time
      rtc.adjust(DateTime(timeinfo.tm_year + 1900, 
                          timeinfo.tm_mon + 1, 
                          timeinfo.tm_mday,
                          timeinfo.tm_hour, 
                          timeinfo.tm_min, 
                          timeinfo.tm_sec));
      Serial.println("RTC successfully synchronized with NTP!");
    } else {
      Serial.println("Failed to obtain NTP time. Falling back to RTC.");
    }

    // Turn off Wi-Fi to reduce power consumption and heat
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  } else {
    Serial.println("\nWi-Fi connection failed. Operating in offline mode using RTC time.");
  }
}

void loop() {
  DateTime now = rtc.now();

  int hour   = now.hour();
  int minute = now.minute();
  int second = now.second();

  // Calculate LED positions (0 to 59)
  int secondPixel = second;
  int minutePixel = minute;
  // Hour hand advances every 12 minutes (minute / 12)
  int hourPixel   = ((hour % 12) * 5) + (minute / 12);

  // Clear all pixels
  strip.clear();

  // Optional: Draw faint hour markers (every 5 LEDs) for dial layout
  for (int i = 0; i < 60; i += 5) {
    strip.setPixelColor(i, strip.Color(3, 3, 3));
  }

  // Set Hand Colors with Color Blending on Overlaps:
  
  // 1. Hour Hand (Red)
  strip.setPixelColor(hourPixel, strip.Color(255, 0, 0));

  // 2. Minute Hand (Green or Blended)
  if (minutePixel == hourPixel) {
    strip.setPixelColor(minutePixel, strip.Color(255, 255, 0)); // Red + Green = Yellow
  } else {
    strip.setPixelColor(minutePixel, strip.Color(0, 255, 0));
  }

  // 3. Second Hand (Blue or Blended)
  if (secondPixel == hourPixel) {
    strip.setPixelColor(secondPixel, strip.Color(255, 0, 255)); // Red + Blue = Magenta
  } else if (secondPixel == minutePixel) {
    strip.setPixelColor(secondPixel, strip.Color(0, 255, 255)); // Green + Blue = Cyan
  } else {
    strip.setPixelColor(secondPixel, strip.Color(0, 0, 255));
  }

  strip.show();
  delay(200); // 200ms refresh rate for smooth display
}
