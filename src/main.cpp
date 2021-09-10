#include "secret.h"
#include "..\.pio\libdeps\nano_33_iot\WiFiNINA_Generic\examples\ConnectWithWPA\defines.h"
#include <Arduino.h>
#include <SPI.h>
#define WIFI_FIRMWARE_LATEST_VERSION "1.4.5"
#include <WiFiNINA_Generic.h>
#include <MFRC522.h>
#include "DFRobotDFPlayerMini.h"
//RFID CONFIGURATIONS
#define SS_PIN 10
#define RST_PIN 9
MFRC522 rfid(SS_PIN, RST_PIN); //instance of the class
MFRC522::MIFARE_Key key;
//red keycard
int code[] = {50, 72, 157, 178};
int codeRead = 0;
String uidString;

DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

void printDec(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}

void readRFID()
{
  Serial.println("i found something...");
  rfid.PICC_ReadCardSerial();
  Serial.print(F("\nPICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));
  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K)
  {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }
  Serial.println("Scanned PICC's UID:");
  printDec(rfid.uid.uidByte, rfid.uid.size);
  uidString = String(rfid.uid.uidByte[0]) + " " + String(rfid.uid.uidByte[1]) + " " + String(rfid.uid.uidByte[2]) + " " + String(rfid.uid.uidByte[3]);
  int i = 0;
  boolean match = true;
  while (i < rfid.uid.size)
  {
    if (!(int(rfid.uid.uidByte[i]) == int(code[i])))
    {
      match = false;
    }
    i++;
  }
  if (match)
  {
    Serial.println("\n*** Unlock ***");
    // myDFPlayer.play(1);
  }
  else
  {
    Serial.println("\nUnknown Card");
  }

  Serial.println("============================");

  // Halt PICC

  rfid.PICC_HaltA();

  // Stop encryption on PCD

  rfid.PCD_StopCrypto1();
}

//WIFI CONFIGURATIONS
char ssid[] = SECRET_SSID; // your network SSID (name)
char pass[] = SECRET_PASS; // your network password (use for WPA, or use as key for WEP), length must be 8+
int status = WL_IDLE_STATUS;
int led = LED_BUILTIN;
WiFiServer server(80);

void print_WiFi_status()
{
  // print the SSID of the network you're attached to:
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print(F("IP Address: "));
  Serial.println(ip);

  // print where to go in a browser:
  Serial.print(F("To see this page in action, open a browser to http://"));
  Serial.println(ip);
}

void setup_WiFi_Status()
{
  Serial.print(F("\nStart AP_SimpleWebServer on "));
  Serial.println(BOARD_NAME);
  Serial.println(WIFININA_GENERIC_VERSION);

  pinMode(led, OUTPUT); // set the LED pin mode

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println(F("Communication with WiFi module failed!"));
    // don't continue
    while (true)
      ;
  }

  String fv = WiFi.firmwareVersion();

  if (fv < WIFI_FIRMWARE_LATEST_VERSION)
  {
    Serial.print(F("Your current firmware NINA FW v"));
    Serial.println(fv);
    Serial.print(F("Please upgrade the firmware to NINA FW v"));
    Serial.println(WIFI_FIRMWARE_LATEST_VERSION);
  }

  // by default the local IP address will be 192.168.4.1
  // you can override it with the following:
  // WiFi.config(IPAddress(10, 0, 0, 1));

  // print the network name (SSID);
  Serial.print(F("Creating access point named: "));
  Serial.println(ssid);

  // Create open network. Change this line if you want to create an WEP network:
  status = WiFi.beginAP(ssid, pass);
  if (status != WL_AP_LISTENING)
  {
    Serial.println(F("Creating access point failed"));
    // don't continue
    while (true)
      ;
  }

  // start the web server on port 80
  server.begin();

  // you're connected now, so print out the status
  print_WiFi_status();
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial)
    ;
  SPI.begin();
  Serial.println("starting setup");
  rfid.PCD_Init();
  // myDFPlayer.volume(4);
}

void loop()
{
  if (rfid.PICC_IsNewCardPresent())
  {
    readRFID();
  }
  // if (myDFPlayer.available())
  // {
  //   printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  // }

  delay(100);
}

void printDetail(uint8_t type, int value)
{
  switch (type)
  {
  case TimeOut:
    Serial.println(F("Time Out!"));
    break;
  case WrongStack:
    Serial.println(F("Stack Wrong!"));
    break;
  case DFPlayerCardInserted:
    Serial.println(F("Card Inserted!"));
    break;
  case DFPlayerCardRemoved:
    Serial.println(F("Card Removed!"));
    break;
  case DFPlayerCardOnline:
    Serial.println(F("Card Online!"));
    break;
  case DFPlayerUSBInserted:
    Serial.println("USB Inserted!");
    break;
  case DFPlayerUSBRemoved:
    Serial.println("USB Removed!");
    break;
  case DFPlayerPlayFinished:
    Serial.print(F("Number:"));
    Serial.print(value);
    Serial.println(F(" Play Finished!"));
    break;
  case DFPlayerError:
    Serial.print(F("DFPlayerError:"));
    switch (value)
    {
    case Busy:
      Serial.println(F("Card not found"));
      break;
    case Sleeping:
      Serial.println(F("Sleeping"));
      break;
    case SerialWrongStack:
      Serial.println(F("Get Wrong Stack"));
      break;
    case CheckSumNotMatch:
      Serial.println(F("Check Sum Not Match"));
      break;
    case FileIndexOut:
      Serial.println(F("File Index Out of Bound"));
      break;
    case FileMismatch:
      Serial.println(F("Cannot Find File"));
      break;
    case Advertise:
      Serial.println(F("In Advertise"));
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
}
