//Libraries for LORA
#include <SPI.h>
#include "LoRa.h"

//Libraries for OLED Display SSD1306
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Prepare OLED Display SSD1306
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

//Prepare LORA
#define BAND    868E6

int counter;

void setup() {
  //Prepare Serial connection
  Serial.begin(115200);
  Serial.println("Initialize");

  //Reset Display
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  Serial.println("Initialize display");
  //Open connection to Display
  Wire.begin(OLED_SDA, OLED_SCL);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(5,10);
  display.setTextSize(2);
  display.print("init");

  display.setTextColor(WHITE);
  display.setCursor(30,35);
  display.setTextSize(3);
  display.print("LoRa");

  display.display();

  //Activate LoRa
  Serial.println("Initialize LoRa");
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST_LoRa,DIO0);
  if (!LoRa.begin(BAND))
  {
    Serial.println("LoRa initialization failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(12);
}

void loop() {

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(5,10);
  display.setTextSize(2);
  display.print("wait for");

  display.setTextColor(WHITE);
  display.setCursor(5,35);
  display.setTextSize(2);
  display.print("package...");

  display.display();

  int packetSize = LoRa.parsePacket();
  if (packetSize) {

    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setCursor(5,10);
    display.setTextSize(2);
    display.print("receive");
    display.display();
    
    // received a packet
    Serial.print("Received packet '");

    // read packet
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());

    delay(1000);
  }
}
