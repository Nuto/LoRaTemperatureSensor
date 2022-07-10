//Libraries for LORA
#include <SPI.h>
#include "LoRa.h"

//Libraries for WLAN
#include <WiFi.h>

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

//Prepare WLAN
const char* ssid = "xxxxxx"; //replace "xxxxxx" with your WIFI's ssid
const char* password = "xxxxxx"; //replace "xxxxxx" with your WIFI's password

unsigned int receiveCounter;

void displaySmallText(int positionX, int positionY, String text) {
  display.setTextColor(WHITE);
  display.setCursor(positionX, positionY);
  display.setTextSize(1);
  display.print(text);
}

void displayNormalText(int positionX, int positionY, String text) {
  display.setTextColor(WHITE);
  display.setCursor(positionX, positionY);
  display.setTextSize(2);
  display.print(text);
}

void displayLargeText(int positionX, int positionY, String text) {
  display.setTextColor(WHITE);
  display.setCursor(positionX, positionY);
  display.setTextSize(3);
  display.print(text);
}

void displayExtraLargeText(int positionX, int positionY, String text) {
  display.setTextColor(WHITE);
  display.setCursor(positionX, positionY);
  display.setTextSize(4);
  display.print(text);
}

void resetOledDisplay() {
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);
}

void initializeOledDisplay() {
  Serial.println(F("Initialize SSD1306 display"));
  //Open connection to Display
  Wire.begin(OLED_SDA, OLED_SCL);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }

  display.setRotation(2); //Rotate 180°
}

void initializeLoRa() {
  display.clearDisplay();
  displaySmallText(0, 0, "Initialize");
  displayLargeText(0, 20, "LoRa");
  display.display();

  Serial.println(F("Initialize LoRa"));
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST_LoRa,DIO0);
  if (!LoRa.begin(BAND))
  {
    Serial.println(F("LoRa initialization failed!"));
    while (1);
  }
  LoRa.setSpreadingFactor(12);
  LoRa.setTxPower(20, PA_OUTPUT_PA_BOOST_PIN);

  delay(500);
}

void initializeWLAN() {
  display.clearDisplay();
  displaySmallText(0, 0, "Initialize");
  displayLargeText(0, 20, "WLAN");
  display.display();

  WiFi.disconnect(true);
  delay(1000);
  
  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.setHostname("LDTS-Gateway");
  WiFi.begin(ssid, password);

  byte retryCount = 0;
  byte maxRetries = 10;
  while (WiFi.status() != WL_CONNECTED && retryCount < maxRetries) {
    retryCount++;
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    displaySmallText(0, 44, "Connected");
    display.display();
    delay(1000);

    return;
  } else {
    displaySmallText(0, 44, "Cannot connect");
    display.display();

    Serial.println(F("WLAN initialization failed!"));
    while (1);
  }
}

void setup() {
  //Prepare Serial connection
  Serial.begin(115200);
  Serial.println("Initialize system");

  resetOledDisplay();
  initializeOledDisplay();
  initializeLoRa();
  initializeWLAN();
}

void loop() {
  display.clearDisplay();
  
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    receiveCounter++;
    displayNormalText(0, 8, "package");
    displayNormalText(0, 25, "received");
    
    // received a packet
    Serial.print("Received packet '");

    // read packet
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  } else {
    displaySmallText(0, 0, "Received packages");
    displayLargeText(0, 20, String(receiveCounter));
  }

  display.display();
  delay(1000);
}
