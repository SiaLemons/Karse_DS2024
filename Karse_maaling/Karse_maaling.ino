#include <dht.h>
#include <Wire.h>
#include "SparkFun_VL53L1X.h"
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>

// DHT setup
dht DHT;             // Laver en variabel, DHT, som vi kan referere til senere
#define DHT11_PIN 7  //  Fortæller at outputtet fra måleren skal sendes til pin 7

// VL53L1x (distance måler) setup
SFEVL53L1X distanceSensor;  // Laver en variabel, distanceSensor

// SD kort setup
File myFile;  // Laver en variabel, myFile

// RTC setup
ThreeWire myWire(4, 5, 2);         // Forbindelser der går til klokmodulet definieres
RtcDS1302<ThreeWire> Rtc(myWire);  // Laver en variabel, Rtc

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Specificere typen af LCD, 16 i bredde og 2 i højde

// Definer pins
int sensorValue = 0;
int digiPin = 2;
float val;

/* Nåede ikke
//lamper
int LedTempH = 9;
int LedTempL = 8;
int LedHuH = 6;
int LedHuL = 3;
*/


void setup() {
  Serial.begin(9600);

  Wire.begin();

  pinMode(A0, INPUT);  // Definerer input porten til lyssensoren
  pinMode(digiPin, OUTPUT);

  // VL53L1X distance sensor
  // Hvis sensoren finder en fejl under opstart, vil den fryse og udskrive en fejlmeddelelse.
  Serial.println("VL53L1X Qwiic Test");
  if (distanceSensor.begin() != 0)  //Begin returns 0 on a good init
  {
    Serial.println("Sensor failed to begin. Please check wiring. Freezing...");
    while (1)
      ;
  }
  Serial.println("Sensor online!");

  // RTC
  Rtc.Begin();
  // Genstarter uret. Skal kun kørers første gang og udkommenteres der efter
  //RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  //Rtc.SetDateTime(compiled);

  // initialize the LCD
  lcd.init();
  lcd.setBacklight((uint8_t)1);  // Tænder for baggrundslys

  // SD kort
  while (!Serial) {
    ;  // Vent på at seriel port er klar
  }
  Serial.print("Initializing SD card...");
  if (!SD.begin(10)) {  //Tallet i parentesen skal være det pinnummer, I har tildelt CS fra modulet til jeres SD-læser
    Serial.println("initialization failed!");
    while (1)
      ;
  }
  Serial.println("initialization done.");

  // Laver kolonner og \t afstande mellem så filen er lettere at uploade til R
  myFile = SD.open("KarseLog.txt", FILE_WRITE);
  if (myFile) {
    myFile.print("DatoTid");
    myFile.print("\t");
    myFile.print("Temperatur");
    myFile.print("\t");
    myFile.print("Luftfugtighed");
    myFile.print("\t");
    myFile.print("Afstand");
    myFile.print("\t");
    myFile.print("Lys");
    myFile.print("\n");
    myFile.close();
  } else {
    Serial.println("Kan ikke åbne log fil");
  }
}




void loop() {
  lcd.clear();

  // temp og fugt
  int chk = DHT.read11(DHT11_PIN);
  Serial.print("Temperature = ");
  Serial.println(DHT.temperature);
  Serial.print("Humidity = ");
  Serial.println(DHT.humidity);
  float temperature = DHT.temperature;
  float humidity = DHT.humidity;

  /// print på lcd
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print("C ");
  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print("%");

  delay(2000);
  lcd.clear();



  // afstand
  distanceSensor.startRanging();  //Write configuration bytes to initiate measurement
  while (!distanceSensor.checkForDataReady()) {
    delay(1);
  }
  int distance = distanceSensor.getDistance();  //Get the result of the measurement from the sensor
  distanceSensor.clearInterrupt();
  distanceSensor.stopRanging();
  float distanceMm = distance;

  Serial.print("Distance(mm): ");
  Serial.print(distance);

  float distanceInches = distance * 0.0393701;
  float distanceFeet = distanceInches / 12.0;

  Serial.print("\tDistance(ft): ");
  Serial.print(distanceFeet, 2);

  Serial.println();

  /// print på lcd
  lcd.setCursor(0, 0);
  lcd.print("Dist: ");
  lcd.print(distanceMm);
  lcd.print("mm ");

  delay(2000);
  lcd.clear();


  // Lysintensitet
  int lightRaw = analogRead(A0);
  int light = map(lightRaw, 0, 1023, 0, 100);
  Serial.print("Lys: ");
  Serial.println(light);

  /// print på lcd
  lcd.setCursor(0, 0);
  lcd.print("Lys: ");
  lcd.print(light);

  delay(2000);
  lcd.clear();


  // Læser dato og tid i ISO Get current date and time from RTC in standard IUPAC time format
  String iupacTime = getIUPACTime();


  // Gem på sd kort
  myFile = SD.open("KarseLog.txt", FILE_WRITE);
  if (myFile) {
    myFile.print(iupacTime);
    myFile.print("\t");
    myFile.print(temperature);
    myFile.print("\t");
    myFile.print(humidity);
    myFile.print("\t");
    myFile.print(distance);
    myFile.print("\t");
    myFile.println(light);
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("Error opening KarseLog.txt");
    // You might also want to display an error on the LCD here
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error opening");
    lcd.setCursor(0, 1);
    lcd.print("KarseLog.txt");
  }


  // Led lys
  /// temp mellem 15 og 23
  //if (DHT.temperature > 15 && DHT.temperature < 23) {
  //  digitalWrite(LedTempH, HIGH);
  //} else {
  //  digitalWrite(LedTempL, HIGH);
  //}


  /// fugt mellem 40 og 70
  //if (DHT.humidity > 15 && DHT.humidity < 23) {
  //  digitalWrite(LedHuH, HIGH);
  //} else {
  //  digitalWrite(LedHuL, HIGH);
  //}

  // venter inden loopet starter igen
  delay(2000);
}


// Laver en funktion som gør at tid og dato er mere lækker
// Function til at omformattere tidsformattet til ISO 8601
String getIUPACTime() {
  // Get current date and time from RTC
  RtcDateTime now = Rtc.GetDateTime();
  String dateTime;
  // Year
  dateTime += String(now.Year());
  dateTime += '-';
  // Month
  if (now.Month() < 10) dateTime += '0';
  dateTime += String(now.Month());
  dateTime += '-';
  // Day
  if (now.Day() < 10) dateTime += '0';
  dateTime += String(now.Day());
  dateTime += 'T';  // T is used to separate the date and time in ISO 8601 format
  // Hour
  if (now.Hour() < 10) dateTime += '0';
  dateTime += String(now.Hour());
  dateTime += ':';
  // Minute
  if (now.Minute() < 10) dateTime += '0';
  dateTime += String(now.Minute());
  dateTime += ':';
  // Second
  if (now.Second() < 10) dateTime += '0';
  dateTime += String(now.Second());
  return dateTime;
}
