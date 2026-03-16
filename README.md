# ESP32_MP3_Player
An MP3 player build with Esp32 Dev Board, 1.96 inch oled display, UDA1334A DAC, and SD card reader module 

------------------------------- Pin Layout ------------------------------------- 
Here is the exact pin layout for your ESP32 MP3 Player.
1. 1.3" OLED Display (I2C Interface)
The SH1106 OLED uses the I2C protocol.

| OLED Pin | ESP32 Pin | Notes |
| -------  |  -------  |  ---  |
| VCC      | 3.3V      |       |
| GND      | GND       |       |
| SCL      | GPIO 22   | SCL   |
| SDA      | GPIO 21   | SDA   |

2. UDA1334A DAC (I2S Interface)
This module converts the digital signal from the ESP32 into high-quality analog audio.

| UDA1334A Pin | ESP32 Pin  | Notes                     |
| -----------  | ---------  | ------------------------  |
| VIN          | 3.3V or 5V | Check your specific board |
| GND          | GND        |                           |
| WSEL (LRCK)  | GPIO 25    | Left/Right Clock          |
| BCLK (BCK)   | GPIO 26    | Bit Clock                 |
| DIN (DATA)   | GPIO 32    | Data Out                  |

Note: for Ground(GND) you need to use a dedicated GND connection from ESP32, Don't use common ground.

3. SD Card Reader Module (SPI Interface)
The SD card holds your MP3 files. Note that standard hardware SPI pins are used.

| SD Module Pin | ESP32 Pin | Notes                  |
| ------------- | --------- | ---------------------- |
| VCC           | 5V / 3.3V | Most modules prefer 5V |
| GND           | GND       |                        |
| CS            | GPIO 5    | Chip Select            |
| SCK           | GPIO 18   | Clock                  |
| MOSI          | GPIO 23   | Data In                |
| MISO          | GPIO 19   | Data Out               |
Here i have used 3.3V

4. Navigation Buttons
These buttons are configured in the code as INPUT_PULLUP. This means you connect one side of the button to the GPIO and the other side to GND. No external resistors are needed.
Button Function	ESP32 Pin	Other Side of Button
UP	GPIO 4	GND
DOWN	GPIO 13	GND
SELECT	GPIO 15	GND
Summary Table (ESP32 Side)
GPIO	Function
4	Button Up
5	SD Card CS
13	Button Down
15	Button Select
18	SD Card SCK
19	SD Card MISO
21	OLED SDA
22	OLED SCL
23	SD Card MOSI
25	DAC WSEL (LRC)
26	DAC BCLK
32	DAC DIN (DOUT)

Important Tips for this Build:
Common Ground: Ensure the ESP32, DAC, SD Card, and OLED all share a common Ground (GND).
SD Card Formatting: Ensure your SD card is formatted to FAT32. Files should be in the root directory (e.g., /song1.mp3).
WiFi Noise: You have WiFi.setSleep(true) in your code, which is great. I2S audio can sometimes pick up "buzzing" sounds from the WiFi chip. If you still hear noise, try powering the DAC from a very clean 3.3V source.
UDA1334A Jumpers: On many UDA1334A breakout boards, ensure the SCLK pin is tied to Ground if you aren't using an external system clock (the ESP32 doesn't need it for this code)
