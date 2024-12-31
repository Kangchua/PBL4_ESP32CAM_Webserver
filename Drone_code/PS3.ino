#include <PS2X_lib.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
//oled
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
char master1[] = "TUAN ANH";
char master2[] = "KAY BUI";
//ps2
#define PS2_DAT 2
#define PS2_CMD 3
#define PS2_SEL 4
#define PS2_CLK 5
PS2X ps2x;
int error = 0;
//nrf
RF24 radio(9, 10);  // CE, CSN
const byte address[6] = "20202";
byte data[3];


void setup() {
  Serial.begin(9600);
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, false, false);
  //check connected or not and display
  // end check
  //nrf24l01
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Address 0x3C for 128x32
  display.clearDisplay();
  display.display();
}
//way 0: stop;
//1: up
//2: down
//3: left
//4: right
//5: fr
//6: fl
//7: rr
//8: rl
//9: turn left
//10: turn right
bool whichMaster = true;
int active = 0;
int dir = 0;
int up = 0;
void loop() {
  if (error == 1) return;
  ps2x.read_gamepad();
  if (ps2x.NewButtonState()) {  //will be TRUE if any button changes state (on to off, or off to on)
    if (ps2x.Button(PSB_START)) {
      if(active) active = 0;
      else active = 1;
    }
  }
  if (ps2x.ButtonPressed(PSB_L1)) up = 1;
  if (ps2x.ButtonReleased(PSB_L1)) up = 0;
  if (ps2x.ButtonPressed(PSB_L2)) up = 2;
  if (ps2x.ButtonReleased(PSB_L2)) up = 0;

  if (ps2x.ButtonPressed(PSB_PAD_UP)) dir = 1;
  if (ps2x.ButtonReleased(PSB_PAD_UP)) dir = 0;
  if (ps2x.ButtonPressed(PSB_PAD_RIGHT)) dir = 2;
  if (ps2x.ButtonReleased(PSB_PAD_RIGHT)) dir = 0;
  if (ps2x.ButtonPressed(PSB_PAD_DOWN)) dir = 3;
  if (ps2x.ButtonReleased(PSB_PAD_DOWN)) dir = 0;
  if (ps2x.ButtonPressed(PSB_PAD_LEFT)) dir = 4;
  if (ps2x.ButtonReleased(PSB_PAD_LEFT)) dir = 0;
  if (ps2x.ButtonPressed(PSB_SQUARE)) dir = 5;
  if (ps2x.ButtonReleased(PSB_SQUARE)) dir = 0;
  if (ps2x.ButtonPressed(PSB_CIRCLE)) dir = 6;
  if (ps2x.ButtonReleased(PSB_CIRCLE)) dir = 0;
  data[0] = active;
  data[1] = up;
  data[2] = dir;
  radio.write(&data, sizeof(data));
}
