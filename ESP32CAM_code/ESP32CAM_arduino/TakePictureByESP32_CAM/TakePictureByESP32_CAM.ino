#include "WifiCam.hpp"
#include <WiFi.h>
#include <WebServer.h>
#include <memory>

// Mảng chứa các cặp SSID và mật khẩu
const char* wifiCredentials[][2] = {
  {"banhmitamda", "luuvohoang"},
  {"huybaka", "12345678"},
  {"Melody4_5G", ""},
  {"Melody4", ""} 
};

const int numNetworks = sizeof(wifiCredentials) / sizeof(wifiCredentials[0]);

esp32cam::Resolution initialResolution;
WebServer server(80);
unsigned long lastCaptureTime = 0;
const unsigned long captureInterval = 5000; // 5 seconds
std::unique_ptr<esp32cam::Frame> lastFrame;

void setup() {
  Serial.begin(115200);
  Serial.println();
  delay(1000);

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);

  // Thử kết nối với từng mạng WiFi
  bool connected = false;
  for (int i = 0; i < numNetworks; i++) {
    Serial.printf("Trying to connect to SSID: %s\n", wifiCredentials[i][0]);
    WiFi.disconnect(true); // Ensure proper disconnection
    delay(1000); // Increase delay between connections
    WiFi.begin(wifiCredentials[i][0], wifiCredentials[i][1]);
    if (WiFi.waitForConnectResult() == WL_CONNECTED) {
      Serial.printf("Connected to SSID: %s\n", wifiCredentials[i][0]);
      connected = true;
      break;
    } else {
      Serial.printf("Failed to connect to SSID: %s, Status: %d\n", wifiCredentials[i][0], WiFi.status());
    }
  }

  if (!connected) {
    Serial.println("Failed to connect to any WiFi network");
    delay(5000);
    ESP.restart();
  }

  Serial.println("WiFi connected");
  delay(1000);

  {
    using namespace esp32cam;

    initialResolution = Resolution::find(1024, 768);

    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(initialResolution);
    cfg.setJpeg(80);

    bool ok = Camera.begin(cfg);
    if (!ok) {
      Serial.println("camera initialize failure");
      delay(5000);
      ESP.restart();
    }
    Serial.println("camera initialize success");
  }

  Serial.println("camera starting");
  Serial.print("http://");
  Serial.println(WiFi.localIP());

  server.on("/capture", HTTP_GET, handleCapture);
  server.begin();
}

void loop() {
  server.handleClient();

  unsigned long currentTime = millis();
  if (currentTime - lastCaptureTime >= captureInterval) {
    captureImage();
    lastCaptureTime = currentTime;
  }
}

void captureImage() {
  using namespace esp32cam;
  lastFrame = Camera.capture();
  if (!lastFrame) {
    Serial.println("Capture failed");
    return;
  }
  Serial.printf("Captured %u bytes\n", lastFrame->size());
}

void handleCapture() {
  if (!lastFrame) {
    server.send(500, "text/plain", "No image captured");
    return;
  }
  server.sendHeader("Content-Type", "image/jpeg");
  server.sendHeader("Content-Length", String(lastFrame->size()));
  server.send(200);
  server.sendContent((const char*)lastFrame->data(), lastFrame->size());
}