#include <Servo.h>                 // Uvozimo kniznjice za servomotor, display, rfid ter temp/humidity sensor
#include <LiquidCrystal.h>
#include <dht.h>
#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN         9          // Reset pin za RFID scanner
#define SS_PIN          10         // RFID signal input
#define DHT11_PIN       6          // Pin za branje podatkov iz temp/humidity tipala
#define approve         A2         // Pin za aktivacijo brenčača
#define red_light_pin   A4         // Analog pin za dolocanje R barve na RGB led diodi
#define green_light_pin A3         // Analog pin za dolocanje G barve na RGB led diodi
#define servo           A5         // Pin za komunikacijo s servo motorjem
#define LEDOUTPUT       A1         // Pin output za led diodo
#define LIGHTINPUT      A0         // Pin input za fotosenzor
dht DHT;                           // Kreiranje dht objekta - temp/humi
Servo Servom;                      // Kreiranje Servo objekta

const int  rs = 8, en = 7, d4 = 5, d5 = 4, d6 = 3, d7 = 2;    // Za liquid display uporabim 4bit komunikacijo (2, 3, 4, 5), register select pin (8), enable pin (7),
String card = "87 66 71 62";                                  // ID kartice, katero RFID scanner spusti naprej
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);                    // Kreiramo nov objekt tipa LiquidCrystal
MFRC522 mfrc522(SS_PIN, RST_PIN);                             // Kreiramo nov objekt tipa MFRC522 za RFID scanner

void setup(){
  Serial.begin(9600);                               
  SPI.begin();
  pinMode(approve, OUTPUT);                       // Nastavimo brenčač kot output pin
  pinMode(red_light_pin, OUTPUT);                 // Nastavimo pin za R barvo kot output
  pinMode(green_light_pin, OUTPUT);               // Nastavimo pin za G barvo kot output
  pinMode(LIGHTINPUT, INPUT);                     // Nastavimo pin za foto tipalo kot input
  pinMode(LEDOUTPUT, OUTPUT);                     // Nastavimo pin za LED diodo kot output
  mfrc522.PCD_Init();                             // Inicializiramo RFID scanner
  Servom.attach(servo);                           // Inicializiramo servo motor
  lcd.begin(16, 2);                               // Nastavimo LCD display na 16 stolpcev (char), ter 2 vrstice
  RGB_color(255, 0);                              // RGB LED diodo nastavimo na rdečo barvo s pomocjo RGB_color funkcije
}

void loop(){
  if (mfrc522.PICC_IsNewCardPresent()) {          // Preverimo če RFID zazna kartice
      if (mfrc522.PICC_ReadCardSerial()) {        // Preverimo če RFID lahko prebere ID kartice
          //Poiscemo in zapisemo ID kartice v content
          String content= "";   
          byte letter;
          for (byte i = 0; i < mfrc522.uid.size; i++) 
          { 
             content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
             content.concat(String(mfrc522.uid.uidByte[i], HEX));
          }
          content.toUpperCase();                  // Postavimo ID v uppercase
          lcd.clear();                            // Pocistimo trenutna vsebina displaya
          if (content.substring(1) == card)       // Primerjamo, če se prebrani ID ujema z String card
          {
            int val = digitalRead(red_light_pin);           // V val postavimo 1, ce gori rdeca, 0 drugace
            if(val == 1){                                   // Preverimo ce gori rdeca
              int LightVal = analogRead(LIGHTINPUT);        // Preberemo vrednost foto tipala 
              int Light = map(LightVal, 10, 600, 0, 1400);  // In ga mapiramo
              Serial.println(Light);                        // Ce je vrednost dovolj nizka oz. je v sobi tema, na ledout postavimo HIGH
              if(Light < 10){
                digitalWrite(LEDOUTPUT, HIGH);
              }
              RGB_color(0, 255);                // Vklopimo zeleno barvo na RGB LED diodi
              lcd.print("WELCOME!");            // LCD izpis
              tone(approve, 1000);              // Na brenčač posljemo 1kHZ signal
              delay(200);                       // Pocakamo 200ms
              Servom.write(0);                  // Z servo motorjem odpremo
            }
            else{
              RGB_color(255, 0);                // Postavimo RGB LED diodo na rdeco barvo
              lcd.print("GOODBYE!");            /// LCD izpis
              tone(approve, 1000);              // Spet posljemo 1kHZ signal
              delay(100);                       // 100ms
              Servom.write(90);                 // Servo motor zapre
              int readlig = digitalRead(LEDOUTPUT);     // Ce je bila LED dioda za osvetlitev vklopljena, jo izklopimo
              if(readlig == 1){
                digitalWrite(LEDOUTPUT, LOW);
              }
            }
          }
         else   {
            RGB_color(255, 0);        // Postavimo RGB LED diodo na rdeco barvo
            lcd.print("WRONG KEY!");  // LCD izpis
            tone(approve, 1000);      // Posljemo 1kHz signal na brenčač
            delay(500);               // 500ms
          }
      noTone(approve);   // Izklopimo brenčač  
      delay(2000);      // in pocakamo 2s
      } 
  }
  int chk = DHT.read11(DHT11_PIN);  //Branje temp in vlage  
  lcd.setCursor(0, 0);              // Cursor LCD postavimo na 0 vrstico in 0 stolpec
  lcd.print("Temp = ");             // LCD izpis
  lcd.print(DHT.temperature);       // Izpis temp
  lcd.print((char)223);             // Izpis celsius znaka
  lcd.print("C");               
  lcd.setCursor(0, 1);              // Cursor LCD postavimo na 1 vrstico in 0 stolpec
  lcd.print("Humidity = ");         // LCD izpis
  lcd.print(DHT.humidity);          // Izpis vlaznosti
  delay(1200);                      
}

//Funkcija za kontroliranje RGB LED diode, v tem primeru sam R in G barve, ker modre ne potrebujem.
void RGB_color(int red_light_value, int green_light_value)
 {
  analogWrite(red_light_pin, red_light_value);       // Nastavimo vrednost R barve          
  analogWrite(green_light_pin, green_light_value);   // Nastavimo vrednost G barve
}
