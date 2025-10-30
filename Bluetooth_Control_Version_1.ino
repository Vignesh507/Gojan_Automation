#include <BluetoothSerial.h>
#include <Preferences.h>   

BluetoothSerial Bluetooth;
Preferences prefs;         

uint8_t relay[7] = {18, 19, 22, 23, 21, 4, 15};
char BluetoothData;

void setup()
{
  Serial.begin(115200);
  Bluetooth.begin("Principal Office");
  prefs.begin("relay_state", false);  

  for(int i = 0; i < 7; i++)
  {
    pinMode(relay[i], OUTPUT);
    int lastState = prefs.getInt(String(i).c_str(), LOW);
    digitalWrite(relay[i], lastState);
  }
  Serial.println("Last relay states restored after power-up");
}

void loop()
{
  if(Bluetooth.available() > 0)
  {
    BluetoothData = Bluetooth.read();
    Serial.print("Bluetooth Value : ");
    Serial.println(BluetoothData);
  }

  switch(BluetoothData)
  {
    case 'A': digitalWrite(relay[0], HIGH); prefs.putInt("0", HIGH); break;
    case 'B': digitalWrite(relay[1], HIGH); prefs.putInt("1", HIGH); break;
    case 'C': digitalWrite(relay[2], HIGH); prefs.putInt("2", HIGH); break;
    case 'D': digitalWrite(relay[3], HIGH); prefs.putInt("3", HIGH); break;
    case 'E': digitalWrite(relay[4], HIGH); prefs.putInt("4", HIGH); break;
    case 'F': digitalWrite(relay[5], HIGH); prefs.putInt("5", HIGH); break;
    case 'G': digitalWrite(relay[6], HIGH); prefs.putInt("6", HIGH); break;

    case '1': digitalWrite(relay[0], LOW); prefs.putInt("0", LOW); break;
    case '2': digitalWrite(relay[1], LOW); prefs.putInt("1", LOW); break;
    case '3': digitalWrite(relay[2], LOW); prefs.putInt("2", LOW); break;
    case '4': digitalWrite(relay[3], LOW); prefs.putInt("3", LOW); break;
    case '5': digitalWrite(relay[4], LOW); prefs.putInt("4", LOW); break;
    case '6': digitalWrite(relay[5], LOW); prefs.putInt("5", LOW); break;
    case '7': digitalWrite(relay[6], LOW); prefs.putInt("6", LOW); break;    

    case 'a':
      for(int i = 0; i < 7; i++)
      {
        digitalWrite(relay[i], HIGH);
        prefs.putInt(String(i).c_str(), HIGH);
      }
      break;  
      
    case 'b':
      for(int i = 0; i < 7; i++)
      {
        digitalWrite(relay[i], LOW);
        prefs.putInt(String(i).c_str(), LOW);
      }
      break;   
  }
}
