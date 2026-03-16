
//========================    MP3 PLAYER Code =====================================
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include "Audio.h"
#include <WiFi.h>
#include "time.h"
#include <vector> // Added for efficient song list storage

// --- WIFI ---
const char *ssid = "Your_Wifi";
const char *password = "Your_Password";

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
std::vector<String> songList; // Stores all song names in RAM
int totalSongs = 0;
int currentIndex = 0;   
int playingIndex = -1;  
int currentVolume = 12;
const int MAX_VOL = 21;
bool isMenuMode = true, isVolAdjustMode = false;
int playerOption = 1;
bool lastUp = HIGH, lastDown = HIGH, lastSel = HIGH;

// Task handle for Dual Core Audio
TaskHandle_t AudioTask;

// --- Optimized: Get song name from RAM instead of SD ---
String getSongName(int index) {
  if (index >= 0 && index < songList.size()) {
    return songList[index];
  }
  return "";
}

// --- Optimized: Scan SD card once and store names in RAM ---
void loadSongList() {
  songList.clear();
  File root = SD.open("/");
  if (!root) return;
  
  File file = root.openNextFile();
  while (file) {
    String name = String(file.name());
    if (name.endsWith(".mp3") || name.endsWith(".MP3")) {
      // Remove leading slash if present
      if (name.startsWith("/")) name = name.substring(1);
      songList.push_back(name);
    }
    file = root.openNextFile();
  }
  root.close();
  totalSongs = songList.size();
  Serial.printf("Loaded %d songs into RAM\n", totalSongs);
}

String getClockTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "Syncing..";
  char buf[12];
  strftime(buf, sizeof(buf), "%I:%02M %p", &timeinfo);
  return String(buf);
}

void audioTaskLoop(void *pvParameters) {
  for (;;) {
    audio.loop();
    if (!audio.isRunning()) vTaskDelay(5 / portTICK_PERIOD_MS); 
    else vTaskDelay(1);
  }
}

String getScrollingName(String name, int maxLen) {
  // Clean extension for display
  if (name.endsWith(".mp3")) name = name.substring(0, name.length() - 4);
  if (name.endsWith(".MP3")) name = name.substring(0, name.length() - 4);

  if (name.length() <= maxLen) return name;
  
  String spacer = "  * ";
  String longName = name + spacer;
  int offset = (millis() / 200) % longName.length();
  String shifted = longName.substring(offset) + longName.substring(0, offset);
  return shifted.substring(0, maxLen);
}

void drawUI() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);

  if (totalSongs == 0) {
    display.setCursor(30, 30);
    display.print("NO MEDIA FOUND");
  } 
  else if (isMenuMode) {
    // --- LIST PAGE ---
    display.setCursor(0, 0);
    display.print(getClockTime());
    
    String counter = String(currentIndex + 1) + "/" + String(totalSongs);
    display.setCursor(128 - (counter.length() * 6), 0);
    display.print(counter);

    String name = getScrollingName(getSongName(currentIndex), 18);
    int16_t x1, y1; uint16_t w, h;
    display.getTextBounds(name, 0, 0, &x1, &y1, &w, &h);

    display.fillRoundRect((128 - w) / 2 - 4, 28, w + 8, 14, 3, WHITE);
    display.setTextColor(BLACK);
    display.setCursor((128 - w) / 2, 31);
    display.print(name);
    
    display.setTextColor(WHITE);
    display.setCursor(45, 52);
    display.print("BROWSE");
  } 
  else {
    // --- PLAYER PAGE ---
    display.setCursor(2, 0);
    display.print(getClockTime());

    String counter = String(playingIndex + 1) + "/" + String(totalSongs);
    display.setCursor(126 - (counter.length() * 6), 0);
    display.print(counter);
    display.drawFastHLine(0, 9, 128, WHITE);

    display.drawCircle(10, 32, 10, WHITE);
    display.drawCircle(10, 32, 2, WHITE);

    String n = getScrollingName(getSongName(playingIndex), 16);
    display.setCursor(30, 24);
    display.print(n);

    display.setCursor(30, 36);
    if (audio.isRunning()) {
      display.print("PLAYING");
      for (int i = 0; i < 5; i++) {
        int barH = random(2, 12);
        display.fillRect(80 + (i * 5), 45 - barH, 2, barH, WHITE);
      }
    } else {
      display.print("PAUSED");
    }

    // FOOTER
    display.fillRect(0, 50, 128, 14, WHITE);
    display.setTextColor(BLACK);
    display.setCursor(3, 54);
    display.print(playerOption == 0 ? "> LIST" : "  LIST");
    
    display.setCursor(45, 54);
    display.print(playerOption == 1 ? (audio.isRunning() ? "> PAUSE" : "> PLAY") : (audio.isRunning() ? "  PAUSE" : "  PLAY"));

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
  display.setTextColor(WHITE);
  display.setCursor(20, 30);
  display.print("Mounting SD...");
  display.display();

  SPI.begin(18, 19, 23, SD_CS);
  if (SD.begin(SD_CS)) {
    loadSongList();
  }

  WiFi.begin(ssid, password);
  configTime(19800, 0, "time.google.com", "pool.ntp.org");
  
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(currentVolume);
  WiFi.setSleep(true); 

  xTaskCreatePinnedToCore(audioTaskLoop, "AudioTask", 10000, NULL, 1, &AudioTask, 0);
}

void loop() {
  bool curUp = digitalRead(BTN_UP);
  bool curDown = digitalRead(BTN_DOWN);
  bool curSel = digitalRead(BTN_SELECT);

  if (curUp == LOW && lastUp == HIGH) {
    if (isMenuMode) currentIndex = (currentIndex - 1 + totalSongs) % totalSongs;
    else if (isVolAdjustMode) { if (currentVolume < MAX_VOL) { currentVolume++; audio.setVolume(currentVolume); } }
    else playerOption = (playerOption - 1 + 3) % 3;
  }
  if (curDown == LOW && lastDown == HIGH) {
    if (isMenuMode) currentIndex = (currentIndex + 1) % totalSongs;
    else if (isVolAdjustMode) { if (currentVolume > 0) { currentVolume--; audio.setVolume(currentVolume); } }
    else playerOption = (playerOption + 1) % 3;
  }
  if (curSel == LOW && lastSel == HIGH) {
    if (isMenuMode && totalSongs > 0) {
      if (currentIndex != playingIndex) {
          String path = "/" + getSongName(currentIndex);
          audio.connecttoFS(SD, path.c_str());
          playingIndex = currentIndex;
      }
      isMenuMode = false; 
    } else if (isVolAdjustMode) {
      isVolAdjustMode = false;
    } else {
      if (playerOption == 0) {
          isMenuMode = true; 
          currentIndex = playingIndex;
      }
      if (playerOption == 1) audio.pauseResume();
      if (playerOption == 2) isVolAdjustMode = true;
    }
  }

  lastUp = curUp; lastDown = curDown; lastSel = curSel;

  static unsigned long lastRefresh = 0;
  if (millis() - lastRefresh > 120) {
    drawUI();
    lastRefresh = millis();
  }
}

void audio_eof_mp3(const char *info) {
  playingIndex = (playingIndex + 1) % totalSongs;
  String path = "/" + getSongName(playingIndex);
  audio.connecttoFS(SD, path.c_str());
}