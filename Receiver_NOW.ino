#include <WiFi.h>
#include <esp_now.h>
#include <Preferences.h>

Preferences pref;

const int NUM_RELAYS = 7;
int relayPins[NUM_RELAYS] = { 2, 4, 5, 18, 19, 23, 25 };

bool relayState[NUM_RELAYS] = {0};

typedef struct 
{
  int id;
  bool state;
} SwitchMessage;

SwitchMessage msg;

void loadRelayStates() 
{
  pref.begin("relays", true);
  for (int i = 0; i < NUM_RELAYS; i++) 
  {
    String key = "R" + String(i);
    relayState[i] = pref.getBool(key.c_str(), false); 
    digitalWrite(relayPins[i], relayState[i] ? LOW : HIGH); 
  }
  pref.end();
}

void saveRelayState(int index) 
{
  pref.begin("relays", false);
  String key = "R" + String(index);
  pref.putBool(key.c_str(), relayState[index]);
  pref.end();
}

void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len)
{
  if (len != sizeof(SwitchMessage)) 
  {
    Serial.println("Invalid data size");
    return;
  }

  memcpy(&msg, incomingData, sizeof(msg));
  int idx = msg.id - 1;

  if (idx < 0 || idx >= NUM_RELAYS) 
  {
    Serial.println("Invalid relay ID");
    return;
  }

  relayState[idx] = msg.state;
  digitalWrite(relayPins[idx], relayState[idx] ? LOW : HIGH); 
  saveRelayState(idx);

  Serial.print("Relay ");
  Serial.print(msg.id);
  Serial.print(" -> ");
  Serial.println(msg.state ? "ON" : "OFF");
}

void setup() 
{
  Serial.begin(115200);

  for (int i = 0; i < NUM_RELAYS; i++) 
  {
    pinMode(relayPins[i], OUTPUT);
  }

  loadRelayStates(); 

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) 
  {
    Serial.println("ESP-NOW Init Failed");
    return;
  }

  esp_now_register_recv_cb(onDataRecv);

  Serial.println("Receiver Ready ✔");
}

void loop() 
{

}
