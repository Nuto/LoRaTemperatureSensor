//Secrets
#include "secrets.h"

//Bitmaps
#include "bitmaps.h"

//Libraries for LORA
#include <SPI.h>
#include "LoRa.h"

//Libraries for WLAN
#include <WiFi.h>

//Prepare WLAN (please enter your sensitive data in the tab -> arduino_secrets.h)
const char* ssid = SECRET_WLAN_SSID;
const char* password = SECRET_WLAN_PASS;

//Libraries for WebRequests
#include <HTTPClient.h>

//Prepare HttpClient
HTTPClient httpClient;

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

void showLogo() {
  display.clearDisplay();
  display.drawBitmap(
    (display.width()  - BOOT_LOGO_WIDTH ) / 2,
    (display.height() - BOOT_LOGO_HEIGHT) / 2,
    bitmap_nager, BOOT_LOGO_WIDTH, BOOT_LOGO_HEIGHT, 1);
  display.display();
  delay(2000);
}

void setup() {
  //Prepare Serial connection
  Serial.begin(115200);
  Serial.println("Initialize system");

  resetOledDisplay();
  initializeOledDisplay();
  showLogo();
  initializeLoRa();
  initializeWLAN();
}

String LoRaData;

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
      LoRaData = LoRa.readString();
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());

    String httpRequestData = "api_key=1234&data=" + LoRaData;
    httpClient.setConnectTimeout(2000);
    httpClient.begin("https://webhook.site/8c40886b-b322-4e11-be43-9107ae30b187");
    httpClient.setTimeout(5000); //5 seconds
    int httpCode = httpClient.POST(httpRequestData);
    if (httpCode == HTTPC_ERROR_SEND_HEADER_FAILED) {
      Serial.println(F("HTTPC_ERROR_SEND_HEADER_FAILED"));
    } else if (httpCode == HTTPC_ERROR_SEND_PAYLOAD_FAILED) {
      Serial.println(F("HTTPC_ERROR_SEND_PAYLOAD_FAILED"));
    } else if (httpCode == HTTPC_ERROR_NOT_CONNECTED) {
      Serial.println(F("HTTPC_ERROR_NOT_CONNECTED"));
    } else if (httpCode == HTTPC_ERROR_CONNECTION_LOST) {
      Serial.println(F("HTTPC_ERROR_CONNECTION_LOST"));
    } else if (httpCode == HTTPC_ERROR_NO_STREAM) {
      Serial.println(F("HTTPC_ERROR_NO_STREAM"));
    } else if (httpCode == HTTPC_ERROR_NO_HTTP_SERVER) {
      Serial.println(F("HTTPC_ERROR_NO_HTTP_SERVER"));
    } else if (httpCode == HTTPC_ERROR_TOO_LESS_RAM) {
      Serial.println(F("HTTPC_ERROR_TOO_LESS_RAM"));
    } else if (httpCode == HTTPC_ERROR_ENCODING) {
      Serial.println(F("HTTPC_ERROR_ENCODING"));
    } else if (httpCode == HTTPC_ERROR_STREAM_WRITE) {
      Serial.println(F("HTTPC_ERROR_STREAM_WRITE"));
    } else if (httpCode == HTTPC_ERROR_READ_TIMEOUT) {
      Serial.println(F("HTTPC_ERROR_READ_TIMEOUT"));
    }
    httpClient.end();
    Serial.println(httpCode);
  } else {
    displaySmallText(0, 0, "Received packages");
    displayLargeText(0, 20, String(receiveCounter));
  }

  display.display();
  delay(1000);
}
