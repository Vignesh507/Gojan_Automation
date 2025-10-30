#include <WiFi.h>
#include <FirebaseESP32.h>
#include <ArduinoOTA.h>

#define SSID          "MEETING HALL"
#define PASSWORD      "Gojan@2025"
#define FIREBASE_HOST "pricipal-office-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_AUTH "qPhf5Crqc3YP05WTJh7qwGlhCHAqXLf1D6wwU9zA"
#define statusLED     2

FirebaseData   fbdo;
FirebaseAuth   auth;
FirebaseConfig config;

uint8_t relay[7]    = {18, 19, 22, 23, 21, 4, 15};
bool UpdateState[7] = {0, 0, 0, 0, 0, 0, 0};
const char* keys[7] = {"fan1", "fan2", "fan3", "fan4", "light1", "light2", "light3"};

void indicateLED(int ms_delay)
{
  digitalWrite(statusLED, HIGH); delay(ms_delay);
  digitalWrite(statusLED, LOW);  delay(ms_delay);
}

void wifiStatus()
{
  WiFi.begin(SSID, PASSWORD);
  Serial.println("WiFi Connecting");
  while(WiFi.status() != WL_CONNECTED)
  {
    indicateLED(200);
  }
  Serial.println("WiFi Connected"); 
}

void setup()
{
  Serial.begin(115200);
  pinMode(statusLED, OUTPUT);
  for(int i = 0; i < 7; i++) pinMode(relay[i], OUTPUT);

  wifiStatus();

  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  ArduinoOTA.setHostname("principal-office");
  ArduinoOTA.begin();
}

void loop()
{
  ArduinoOTA.handle();

  if(Firebase.getJSON(fbdo, "/switches")) 
  {
    FirebaseJson &json = fbdo.jsonObject();
    FirebaseJsonData result;

    for(int i = 0; i < 7; i++)
    {
      if(json.get(result, keys[i]))
      {
        UpdateState[i] = result.boolValue;
        digitalWrite(relay[i], UpdateState[i] ? HIGH : LOW);
      }
    }
  }
}

