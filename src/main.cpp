// //=============================================================================

// #include <Arduino.h>
// #include <SPI.h>
// #include <SD.h>
// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SH1106.h>
// #include "Audio.h"
// #include <WiFi.h>
// #include "time.h"

// // --- WIFI ---
// const char *ssid = "Prashanth_Wifi";
// const char *password = "Prashanth123";

// // --- SETTINGS ---
// #define OLED_RESET -1
// Adafruit_SH1106 display(OLED_RESET);
// Audio audio;

// #define SD_CS 5
// #define I2S_BCLK 26
// #define I2S_LRC 25
// #define I2S_DOUT 32
// #define BTN_UP 4
// #define BTN_DOWN 13
// #define BTN_SELECT 15

// // --- GLOBALS ---
// int totalSongs = 0;
// int currentIndex = 0;
// int currentVolume = 12;
// const int MAX_VOL = 21;
// bool isMenuMode = true, isVolAdjustMode = false;
// int playerOption = 1;
// bool lastUp = HIGH, lastDown = HIGH, lastSel = HIGH;

// // Caching to prevent SD card lag
// String cachedName = "";
// int lastCachedIndex = -1;

// // Task handle for Dual Core Audio
// TaskHandle_t AudioTask;

// // --- Helper: Get a specific song name by index ---
// String getSongName(int index) {
//   File root = SD.open("/");
//   if (!root) return "";
//   File file = root.openNextFile();
//   int count = 0;
//   while (file) {
//     String name = String(file.name());
//     if (name.endsWith(".mp3") || name.endsWith(".MP3")) {
//       if (count == index) {
//         String result = name;
//         file.close();
//         root.close();
//         return result;
//       }
//       count++;
//     }
//     file = root.openNextFile();
//   }
//   root.close();
//   return "";
// }

// void countSongs() {
//   totalSongs = 0;
//   File root = SD.open("/");
//   while (File file = root.openNextFile()) {
//     String name = String(file.name());
//     if (name.endsWith(".mp3") || name.endsWith(".MP3")) totalSongs++;
//   }
//   root.close();
// }

// String getClockTime() {
//   struct tm timeinfo;
//   if (!getLocalTime(&timeinfo)) return "Syncing..";
//   int h = timeinfo.tm_hour;
//   int m = timeinfo.tm_min;
//   bool pm = (h >= 12);
//   if (h > 12) h -= 12;
//   if (h == 0) h = 12;
//   char buf[12];
//   sprintf(buf, "%d:%02d %s", h, m, pm ? "PM" : "AM");
//   return String(buf);
// }

// // --- CORE 0 TASK: Audio priority ---
// void audioTaskLoop(void *pvParameters) {
//   for (;;) {
//     audio.loop();
//     vTaskDelay(1); // Required for ESP32 Watchdog
//   }
// }

// String getScrollingName(String name, int maxLen) {
//   // 1. Clean the name: Remove leading slash if it exists
//   if (name.startsWith("/")) name = name.substring(1);
  
//   // 2. Clean the name: Remove ".mp3" or ".MP3" extension
//   if (name.endsWith(".mp3")) name = name.substring(0, name.length() - 4);
//   if (name.endsWith(".MP3")) name = name.substring(0, name.length() - 4);

//   // If the name is short enough, just return it as is
//   if (name.length() <= maxLen) return name;
  
//   // Create a long string with a separator for seamless looping
//   String spacer = "  * ";
//   String longName = name + spacer;
  
//   // Calculate offset based on time (change 150 to adjust speed)
//   int offset = (millis() / 150) % longName.length();
  
//   String shifted = longName.substring(offset) + longName.substring(0, offset);
//   return shifted.substring(0, maxLen);
// }

// void drawUI() {
//   // Update Cache only when index changes
//   if (currentIndex != lastCachedIndex) {
//     cachedName = getSongName(currentIndex);
//     lastCachedIndex = currentIndex;
//   }

//   display.clearDisplay();
//   display.setTextColor(WHITE);
//   display.setTextSize(1);

//   if (totalSongs == 0) {
//     display.setCursor(30, 30);
//     display.print("NO MEDIA FOUND");
//   } 
//   else if (isMenuMode) {
//     // --- PAGE 1: GLASS LIST ---
//     display.setCursor(0, 0);
//     display.print(getClockTime());
//     String counter = String(currentIndex + 1) + "/" + String(totalSongs);
//     int16_t x1, y1; uint16_t w, h;
//     display.getTextBounds(counter, 0, 0, &x1, &y1, &w, &h);
//     display.setCursor(128 - w, 0); // Right-align the counter
//     display.print(counter);

//     String name = getScrollingName(cachedName, 18);
//     display.getTextBounds(name, 0, 0, &x1, &y1, &w, &h);

//     display.fillRoundRect((128 - w) / 2 - 4, 28, w + 8, 12, 3, WHITE);
//     display.setTextColor(BLACK);
//     display.setCursor((128 - w) / 2, 30);
//     display.print(name);
//     display.setTextColor(WHITE);
//     display.setCursor(45, 52);
//     display.print("BROWSE");
//   } 
//   else {
//     // --- PAGE 2: CYBER-DASH PLAYER ---
//     display.setCursor(2, 0);
//     display.print(getClockTime());

//     String counter = String(currentIndex + 1) + "/" + String(totalSongs);
//     int16_t x1, y1; uint16_t w, h;
//     display.getTextBounds(counter, 0, 0, &x1, &y1, &w, &h);
//     display.setCursor(126 - w, 0); // Right-align the counter
//     display.print(counter);
//     display.drawFastHLine(0, 9, 128, WHITE);
//     // Dashboard Elements (Vinyl & Song Title)
//     display.drawCircle(10, 32, 10, WHITE);
//     display.drawCircle(10, 32, 2, WHITE);
//     // Scrolling Song Title (maxLen 16 for better fit)
//     String n = getScrollingName(cachedName, 16);
//     display.setCursor(30, 24);
//     display.print(n);
//     // Playing/Paused Status & Bars
//     display.setCursor(30, 36);
//     if (audio.isRunning()) {
//       display.print("PLAYING");
//       int barsStartX = display.getCursorX() + 6;
//       for (int i = 0; i < 5; i++) {
//         int barH = random(2, 14);
//         display.fillRect(barsStartX + (i * 5), 45 - barH, 2, barH, WHITE);
//       }
//     } else {
//       display.print("PAUSED");
//       display.drawFastHLine(display.getCursorX() + 6, 42, 20, WHITE);
//     }

//     // FOOTER
//     display.fillRect(0, 50, 128, 14, WHITE);
//     display.setTextColor(BLACK);
//     display.setCursor(3, 54);
//     display.print(playerOption == 0 ? "> LIST" : " LIST");
    
//     display.setCursor(45, 54);
//     if (audio.isRunning()) {
//         display.print(playerOption == 1 ? "> PAUSE" : "  PAUSE");
//     } else {
//         display.print(playerOption == 1 ? "> PLAY" : "  PLAY");
//     }

//     display.setCursor(95, 54);
//     display.print(playerOption == 2 ? "> VOL" : "  VOL");
//   }

//   if (isVolAdjustMode) {
//     display.fillRect(20, 15, 88, 30, BLACK);
//     display.drawRect(20, 15, 88, 30, WHITE);
//     display.setTextColor(WHITE);
//     display.setCursor(45, 20); display.print("VOL");
//     display.drawRect(30, 32, 68, 5, WHITE);
//     display.fillRect(30, 32, map(currentVolume, 0, MAX_VOL, 0, 68), 5, WHITE);
//   }
//   display.display();
// }

// void setup() {
//   Serial.begin(115200);
//   Wire.begin(21, 22);
//   Wire.setClock(400000); 
//   display.begin(SH1106_SWITCHCAPVCC, 0x3C);
//   display.clearDisplay();

//   WiFi.begin(ssid, password);
//   configTime(19800, 0, "time.google.com", "pool.ntp.org");
  
//   pinMode(BTN_UP, INPUT_PULLUP);
//   pinMode(BTN_DOWN, INPUT_PULLUP);
//   pinMode(BTN_SELECT, INPUT_PULLUP);

//   SPI.begin(18, 19, 23, SD_CS);
//   if (SD.begin(SD_CS)) countSongs();

//   audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
//   audio.setVolume(currentVolume);
  
//   WiFi.setSleep(true); // Stop WiFi buzz

//   // Create Audio Task on Core 0
//   xTaskCreatePinnedToCore(audioTaskLoop, "AudioTask", 10000, NULL, 1, &AudioTask, 0);

//   drawUI();
// }

// void loop() {
//   // audio.loop() is now on Core 0!
  
//   bool curUp = digitalRead(BTN_UP);
//   bool curDown = digitalRead(BTN_DOWN);
//   bool curSel = digitalRead(BTN_SELECT);

//   if (curUp == LOW && lastUp == HIGH) {
//     if (isMenuMode) currentIndex = (currentIndex - 1 + totalSongs) % totalSongs;
//     else if (isVolAdjustMode) { if (currentVolume < MAX_VOL) { currentVolume++; audio.setVolume(currentVolume); } }
//     else playerOption = (playerOption - 1 + 3) % 3;
//     drawUI();
//   }
//   if (curDown == LOW && lastDown == HIGH) {
//     if (isMenuMode) currentIndex = (currentIndex + 1) % totalSongs;
//     else if (isVolAdjustMode) { if (currentVolume > 0) { currentVolume--; audio.setVolume(currentVolume); } }
//     else playerOption = (playerOption + 1) % 3;
//     drawUI();
//   }
//   if (curSel == LOW && lastSel == HIGH) {
//     if (isMenuMode && totalSongs > 0) {
//       String path = "/" + getSongName(currentIndex);

//       // --- SMART PLAY LOGIC ---
//       // If the selected song is NOT the one currently playing, start it
//       if (lastCachedIndex != currentIndex || !audio.isRunning()) {
//           audio.connecttoFS(SD, path.c_str());
//       }
//       // If it IS the same song, we do nothing to the audio and just change the page
//       isMenuMode = false; // Switch to the 2nd page
//     } else if (isVolAdjustMode) {
//       isVolAdjustMode = false;
//     } else {
//       if (playerOption == 0) isMenuMode = true; // Go back to List
//       if (playerOption == 1) audio.pauseResume();
//       if (playerOption == 2) isVolAdjustMode = true;
//     }
//     drawUI();
//   }
//   lastUp = curUp; lastDown = curDown; lastSel = curSel;

//   static unsigned long lastRefresh = 0;
//   unsigned long refreshRate = 150; 
//   if (millis() - lastRefresh > refreshRate) {
//     drawUI();
//     lastRefresh = millis();
//   }
// }

// void audio_eof_mp3(const char *info) {
//   currentIndex = (currentIndex + 1) % totalSongs;
//   String path = "/" + getSongName(currentIndex);
//   audio.connecttoFS(SD, path.c_str());
// }





//=============================================================================

//=============================================================================
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include "Audio.h"
#include <WiFi.h>
#include "time.h"

// --- WIFI ---
const char *ssid = "Prashanth_Wifi";
const char *password = "Prashanth123";

// --- SETTINGS ---
#define OLED_RESET -1
Adafruit_SH1106 display(OLED_RESET);
Audio audio;

#define SD_CS 5
#define I2S_BCLK 26
#define I2S_LRC 25
#define I2S_DOUT 32
#define BTN_UP 4
#define BTN_DOWN 13
#define BTN_SELECT 15

// --- GLOBALS ---
int totalSongs = 0;
int currentIndex = 0;   // What the user is looking at in the menu
int playingIndex = -1;  // What is actually currently loaded in the player
int currentVolume = 12;
const int MAX_VOL = 21;
bool isMenuMode = true, isVolAdjustMode = false;
int playerOption = 1;
bool lastUp = HIGH, lastDown = HIGH, lastSel = HIGH;

// Caching to prevent SD card lag
String cachedName = "";
int lastCachedIndex = -1;

// Task handle for Dual Core Audio
TaskHandle_t AudioTask;

// --- Helper: Get a specific song name by index ---
String getSongName(int index) {
  File root = SD.open("/");
  if (!root) return "";
  File file = root.openNextFile();
  int count = 0;
  while (file) {
    String name = String(file.name());
    if (name.endsWith(".mp3") || name.endsWith(".MP3")) {
      if (count == index) {
        String result = name;
        file.close();
        root.close();
        return result;
      }
      count++;
    }
    file = root.openNextFile();
  }
  root.close();
  return "";
}

void countSongs() {
  totalSongs = 0;
  File root = SD.open("/");
  while (File file = root.openNextFile()) {
    String name = String(file.name());
    if (name.endsWith(".mp3") || name.endsWith(".MP3")) totalSongs++;
  }
  root.close();
}

String getClockTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "Syncing..";
  int h = timeinfo.tm_hour;
  int m = timeinfo.tm_min;
  bool pm = (h >= 12);
  if (h > 12) h -= 12;
  if (h == 0) h = 12;
  char buf[12];
  sprintf(buf, "%d:%02d %s", h, m, pm ? "PM" : "AM");
  return String(buf);
}

// --- CORE 0 TASK: Audio priority ---
void audioTaskLoop(void *pvParameters) {
  for (;;) {
    audio.loop();
    vTaskDelay(1); 
  }
}

String getScrollingName(String name, int maxLen) {
  if (name.startsWith("/")) name = name.substring(1);
  if (name.endsWith(".mp3")) name = name.substring(0, name.length() - 4);
  if (name.endsWith(".MP3")) name = name.substring(0, name.length() - 4);
  if (name.length() <= maxLen) return name;
  
  String spacer = "  * ";
  String longName = name + spacer;
  int offset = (millis() / 150) % longName.length();
  String shifted = longName.substring(offset) + longName.substring(0, offset);
  return shifted.substring(0, maxLen);
}

void drawUI() {
  // Update Cache for display purposes
  if (currentIndex != lastCachedIndex) {
    cachedName = getSongName(currentIndex);
    lastCachedIndex = currentIndex;
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);

  if (totalSongs == 0) {
    display.setCursor(30, 30);
    display.print("NO MEDIA FOUND");
  } 
  else if (isMenuMode) {
    display.setCursor(0, 0);
    display.print(getClockTime());
    String counter = String(currentIndex + 1) + "/" + String(totalSongs);
    int16_t x1, y1; uint16_t w, h;
    display.getTextBounds(counter, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(128 - w, 0);
    display.print(counter);

    String name = getScrollingName(cachedName, 18);
    display.getTextBounds(name, 0, 0, &x1, &y1, &w, &h);

    display.fillRoundRect((128 - w) / 2 - 4, 28, w + 8, 12, 3, WHITE);
    display.setTextColor(BLACK);
    display.setCursor((128 - w) / 2, 30);
    display.print(name);
    display.setTextColor(WHITE);
    display.setCursor(45, 52);
    display.print("BROWSE");
  } 
  else {
    // --- PLAYER PAGE ---
    display.setCursor(2, 0);
    display.print(getClockTime());

    // On player page, we show the name of what is PLAYING, not what is selected in menu
    String pName = getSongName(playingIndex);
    
    String counter = String(playingIndex + 1) + "/" + String(totalSongs);
    int16_t x1, y1; uint16_t w, h;
    display.getTextBounds(counter, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(126 - w, 0);
    display.print(counter);
    display.drawFastHLine(0, 9, 128, WHITE);

    display.drawCircle(10, 32, 10, WHITE);
    display.drawCircle(10, 32, 2, WHITE);

    String n = getScrollingName(pName, 16);
    display.setCursor(30, 24);
    display.print(n);

    display.setCursor(30, 36);
    if (audio.isRunning()) {
      display.print("PLAYING");
      int barsStartX = display.getCursorX() + 6;
      for (int i = 0; i < 5; i++) {
        int barH = random(2, 14);
        display.fillRect(barsStartX + (i * 5), 45 - barH, 2, barH, WHITE);
      }
    } else {
      display.print("PAUSED");
      display.drawFastHLine(display.getCursorX() + 6, 42, 20, WHITE);
    }

    display.fillRect(0, 50, 128, 14, WHITE);
    display.setTextColor(BLACK);
    display.setCursor(3, 54);
    display.print(playerOption == 0 ? "> LIST" : " LIST");
    
    display.setCursor(45, 54);
    if (audio.isRunning()) display.print(playerOption == 1 ? "> PAUSE" : "  PAUSE");
    else display.print(playerOption == 1 ? "> PLAY" : "  PLAY");

    display.setCursor(95, 54);
    display.print(playerOption == 2 ? "> VOL" : "  VOL");
  }

  if (isVolAdjustMode) {
    display.fillRect(20, 15, 88, 30, BLACK);
    display.drawRect(20, 15, 88, 30, WHITE);
    display.setTextColor(WHITE);
    display.setCursor(45, 20); display.print("VOL");
    display.drawRect(30, 32, 68, 5, WHITE);
    display.fillRect(30, 32, map(currentVolume, 0, MAX_VOL, 0, 68), 5, WHITE);
  }
  display.display();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  Wire.setClock(400000); 
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  WiFi.begin(ssid, password);
  configTime(19800, 0, "time.google.com", "pool.ntp.org");
  
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);

  SPI.begin(18, 19, 23, SD_CS);
  if (SD.begin(SD_CS)) countSongs();

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(currentVolume);
  WiFi.setSleep(true); 

  xTaskCreatePinnedToCore(audioTaskLoop, "AudioTask", 10000, NULL, 1, &AudioTask, 0);
  drawUI();
}

void loop() {
  bool curUp = digitalRead(BTN_UP);
  bool curDown = digitalRead(BTN_DOWN);
  bool curSel = digitalRead(BTN_SELECT);

  if (curUp == LOW && lastUp == HIGH) {
    if (isMenuMode) currentIndex = (currentIndex - 1 + totalSongs) % totalSongs;
    else if (isVolAdjustMode) { if (currentVolume < MAX_VOL) { currentVolume++; audio.setVolume(currentVolume); } }
    else playerOption = (playerOption - 1 + 3) % 3;
    drawUI();
  }
  if (curDown == LOW && lastDown == HIGH) {
    if (isMenuMode) currentIndex = (currentIndex + 1) % totalSongs;
    else if (isVolAdjustMode) { if (currentVolume > 0) { currentVolume--; audio.setVolume(currentVolume); } }
    else playerOption = (playerOption + 1) % 3;
    drawUI();
  }
  if (curSel == LOW && lastSel == HIGH) {
    if (isMenuMode && totalSongs > 0) {
      // Logic: Only change audio if the user selected a DIFFERENT song 
      // or if nothing is currently playing.
      if (currentIndex != playingIndex) {
          String path = "/" + getSongName(currentIndex);
          audio.connecttoFS(SD, path.c_str());
          playingIndex = currentIndex; // Update the tracking variable
      }
      isMenuMode = false; 
    } else if (isVolAdjustMode) {
      isVolAdjustMode = false;
    } else {
      if (playerOption == 0) {
          isMenuMode = true; 
          currentIndex = playingIndex; // Sync menu highlight to what is currently playing
      }
      if (playerOption == 1) audio.pauseResume();
      if (playerOption == 2) isVolAdjustMode = true;
    }
    drawUI();
  }
  lastUp = curUp; lastDown = curDown; lastSel = curSel;

  static unsigned long lastRefresh = 0;
  if (millis() - lastRefresh > 150) {
    drawUI();
    lastRefresh = millis();
  }
}

void audio_eof_mp3(const char *info) {
  playingIndex = (playingIndex + 1) % totalSongs;
  currentIndex = playingIndex; 
  String path = "/" + getSongName(playingIndex);
  audio.connecttoFS(SD, path.c_str());
}