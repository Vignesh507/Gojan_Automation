#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Preferences.h>

Preferences pref;
TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(21);

#define BUZZER 16
#define NUM_SWITCHES 7
#define NUM_DEVICES 2

const char* switchNames[NUM_DEVICES][NUM_SWITCHES] = {
  { "Light", "Fan", "none", "none", "Plug 1", "Plug 2", "none" },   
  { "Light 1", "Light 2", "Fan", "Lamp", "Plug", "none", "none" } 
};

uint8_t macList[NUM_DEVICES][6] = {
  { 0xCC, 0xDB, 0xA7, 0x69, 0x12, 0xDC }, 
  { 0x24, 0x6F, 0x28, 0xAA, 0xBB, 0xCC }  
};

const char* deviceNames[NUM_DEVICES] = {
  "ROOM 1",
  "ROOM 2"
};

int currentDevice = 0;
bool sw[NUM_SWITCHES] = {0};

int bx = 20, by = 40, bw = 200, bh = 35, gap = 40;

#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3800

typedef struct {
  int id;
  bool state;
} SwitchMessage;

SwitchMessage msg;

void saveAllStates() {
  pref.begin("switchDB", false);
  for (int i = 0; i < NUM_SWITCHES; i++) {
    String key = "D" + String(currentDevice) + "-" + String(i);
    pref.putBool(key.c_str(), sw[i]);
  }
  pref.end();
}

void loadAllStates() {
  pref.begin("switchDB", true);
  for (int i = 0; i < NUM_SWITCHES; i++) {
    String key = "D" + String(currentDevice) + "-" + String(i);
    sw[i] = pref.getBool(key.c_str(), false);
  }
  pref.end();
}

void saveSelectedDevice() {
  pref.begin("switchDB", false);
  pref.putUChar("dev", currentDevice);
  pref.end();
}

void loadSelectedDevice() {
  pref.begin("switchDB", true);
  currentDevice = pref.getUChar("dev", 0);
  if (currentDevice >= NUM_DEVICES) currentDevice = 0;
  pref.end();
}

void buzzerBeep(bool on) {
  tone(BUZZER, on ? 2000 : 800, 120);
}

void sendSwitchState(int id, bool state) {
  msg.id = id + 1;
  msg.state = state;
  esp_now_send(macList[currentDevice], (uint8_t*)&msg, sizeof(msg));
}

void onDataSent(const wifi_tx_info_t*, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sent OK" : "Send Fail");
}

bool getTouch(int &x, int &y) {
  if (!ts.touched()) return false;

  long ax = 0, ay = 0;
  for (int i = 0; i < 5; i++) {
    TS_Point p = ts.getPoint();
    ax += p.x;
    ay += p.y;
    delay(3);
  }

  x = map(ay / 5, TS_MINY, TS_MAXY, 0, 240);
  y = map(ax / 5, TS_MINX, TS_MAXX, 320, 0);

  x = constrain(x, 0, 240);
  y = constrain(y, 0, 320);
  return true;
}

void drawDeviceBar() {
  tft.fillRect(0, 0, 240, 30, TFT_BLUE);
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.setTextSize(2);

  tft.setCursor(5, 6);
  tft.print("<");

  tft.setCursor(70, 6);
  tft.print(deviceNames[currentDevice]);

  tft.setCursor(210, 6);
  tft.print(">");
}

void drawSwitch(int id) {
  int y = by + id * gap;

  tft.fillRoundRect(bx, y, bw, bh, 8, TFT_NAVY);
  tft.drawRoundRect(bx, y, bw, bh, 8, TFT_WHITE);

  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(bx + 5, y + 8);
  tft.print(switchNames[currentDevice][id]);

  tft.fillCircle(
    bx + bw - 25,
    y + bh / 2,
    12,
    sw[id] ? TFT_RED : TFT_GREEN
  );
}

void changeDevice(int dir) {
  currentDevice += dir;
  if (currentDevice < 0) currentDevice = NUM_DEVICES - 1;
  if (currentDevice >= NUM_DEVICES) currentDevice = 0;

  saveSelectedDevice();
  loadAllStates();

  drawDeviceBar();
  for (int i = 0; i < NUM_SWITCHES; i++) {
    drawSwitch(i);
    sendSwitchState(i, sw[i]);
    delay(30);
  }

  tone(BUZZER, 1500, 80);
}

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER, OUTPUT);

  loadSelectedDevice();
  loadAllStates();

  tft.init();
  tft.setRotation(0);
  ts.begin();
  tft.fillScreen(TFT_BLACK);

  drawDeviceBar();
  for (int i = 0; i < NUM_SWITCHES; i++) {
    drawSwitch(i);
  }

  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_send_cb(onDataSent);

  esp_now_peer_info_t peer = {};
  peer.channel = 0;
  peer.encrypt = false;

  for (int i = 0; i < NUM_DEVICES; i++) {
    memcpy(peer.peer_addr, macList[i], 6);
    esp_now_add_peer(&peer);
  }

  Serial.println("Transmitter Ready ✔");
}

void loop() {
  int tx, ty;
  if (!getTouch(tx, ty)) return;

  if (ty < 30) {
    if (tx < 60) changeDevice(-1);
    else if (tx > 180) changeDevice(+1);
    while (ts.touched());
    delay(120);
    return;
  }

  for (int i = 0; i < NUM_SWITCHES; i++) {
    int sy = by + i * gap;
    if (tx > bx && tx < bx + bw && ty > sy && ty < sy + bh) {

      sw[i] = !sw[i];
      drawSwitch(i);
      buzzerBeep(sw[i]);
      sendSwitchState(i, sw[i]);
      saveAllStates();

      while (ts.touched());
      delay(120);
      return;
    }
  }
}
