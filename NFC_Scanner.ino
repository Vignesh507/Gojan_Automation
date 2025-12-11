#include <SPI.h>
#include <Adafruit_PN532.h>

#define PN532_SCK  18
#define PN532_MISO 19
#define PN532_MOSI 23
#define PN532_SS    5

Adafruit_PN532 nfc(PN532_SS);

void setup()
{
  Serial.begin(115200);
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if(!versiondata)
  {
    Serial.print("NFC not Connected........");
  }

  Serial.print("Found PN532 with Firmware Version : ");
  Serial.println((versiondata >> 16) &0xFF ,HEX);

  nfc.SAMConfig();
  Serial.println("Waiting for an NFC/RFID Card");
}

void loop()
{
  uint8_t success;
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
  uint8_t uidLength;

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if(success)
  {
    Serial.println("Found a Card ");
    Serial.print("UID Length : ");
    Serial.print(uidLength, DEC);
    Serial.println(" bytes");

    Serial.print("UID Value : ");
    for(uint8_t i = 0; i < uidLength; i++)
    {
      Serial.print(" 0x");
      Serial.print(uid[i], HEX);
    }
    Serial.println(" ");
    delay(1000);
  }
}
