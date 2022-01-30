#include <SPI.h>
#include <Wire.h>
#include <Keypad.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>


#define SS_PIN 10
#define RST_PIN 9

const byte rows = 4;
const byte cols = 4;

byte rowPins[rows] = {A0, A1, A2, A3};
byte colPins[cols] = {5, 4, 3, 2};

String initial_password = "1234";
String passWord = "";
String message = "";
String tag = "6E 38 4F D3";

boolean RFIDMode = true;

int i = 0;

char hexaKeys[rows][cols] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

SoftwareSerial mySerial(8, 7);//sim800l TX and RX connected to 8 and 7 respectively.
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
LiquidCrystal_I2C lcd(0x27, 16, 2);
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, rows, cols);

void setup()
{
  Serial.begin(9600); // Initiate a serial communication
  mySerial.begin(9600);
  lcd.init();
  lcd.begin(16, 2);
  lcd.backlight();
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println("Approximate your card to the reader...");
  lcd.print("Scan your Tag");
  Serial.println();

}
void loop() {
  if (RFIDMode == true) {
    lcd.setCursor(0, 0);
    lcd.print("   Door Lock");
    lcd.setCursor(0, 1);
    lcd.print(" Scan Your Tag ");
    // Look for new cards
    if ( ! mfrc522.PICC_IsNewCardPresent())
    {
      return;
    }
    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
    {
      return;
    }
    //Show UID on serial monitor
    Serial.print("UID tag :");
    String content = "";
    byte letter;
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    Serial.println();
    Serial.print("Message : ");
    content.toUpperCase();
    if (content.substring(1) == tag) {
      Serial.println("Authorized access");
      lcd.clear();
      lcd.print("Authorized tag");
      Serial.println();
      delay(3000);
      lcd.clear();
      lcd.print("Enter Password");
      lcd.setCursor(0, 1);
      RFIDMode = false;
    }
    else {
      lcd.clear();
      lcd.setCursor(0, 0);
      Serial.print("Wrong tag");
      delay(1000);
      lcd.setCursor(0, 1);
      Serial.println(" Access denied");
      lcd.clear();
      lcd.print("Access denied");
      delay(3000);
    }
  }
  if (RFIDMode == false) {
    //Enter Password if card read is successful
    thisLabel:
    char customKey = customKeypad.getKey();
    if (customKey) {
      passWord += customKey;
      lcd.print("*");
      Serial.println(passWord);
      if (passWord.length() == 4 && passWord == initial_password) {
        Serial.println("Correct passWord");
        lcd.clear();
        lcd.print("Access granted");
        passWord = "";
        message = "Someone has gained access with your tag";
        sendMessage(message);
        message = "";
        RFIDMode = true;
        //DO what you wanted to do;
        return;
      } else if (passWord.length() < 4) {
        goto thisLabel;
      } else {
        Serial.println("InCorrect passWord");
        passWord = "";
        message = "Someone hass tried to gain access with the wrong passWord";
        sendMessage(message);
        message = "";
        RFIDMode = true;
      }
    }

  }
}

void sendMessage(String message) {
  mySerial.println("AT");
  updateSerial();
  delay(3000);
  mySerial.println("AT+CMGF=1");
  updateSerial();
  delay(3000);
  mySerial.println("AT+CMGS=\"+254728127853\"");
  updateSerial();
  delay(3000);
  mySerial.println(message);
  updateSerial();
  delay(3000);
  mySerial.println((char)26);  // ASCII code of CTRL+Z
  delay(1000);
  mySerial.println();
  delay(1000);
}
void updateSerial() {
  delay(500);
  while (Serial.available())
  {
    mySerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while (mySerial.available())
  {
    Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
  }
}
