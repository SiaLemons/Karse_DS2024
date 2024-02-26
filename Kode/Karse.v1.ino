#include <dht.h>
#include <Wire.h>
#include "SparkFun_VL53L1X.h"
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>

dht DHT;
SFEVL53L1X distanceSensor;
File myFile;

#define DHT11_PIN 7

int sensorValue = 0;
int digiPin = 2;
float val;

// lamper
int LedTempH = 9;
int LedTempL = 8;
int LedHuH = 6;
int LedHuL = 3;


// RTC setup
ThreeWire myWire(4, 5, 2);  // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

LiquidCrystal_I2C lcd(0x27, 16, 2);




void setup() {
  Wire.begin();

  pinMode(A0, INPUT);
  pinMode(digiPin, OUTPUT);

  Serial.begin(9600);

  //sd kort
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.print("Initializing SD card...");
  if (!SD.begin(10)) {   //Tallet i parentesen skal være det pinnummer, I har tildelt CS fra modulet til jeres SD-læser
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");


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

  // Turn on the blacklight
  lcd.setBacklight((uint8_t)1);
}




void loop() {
  lcd.clear();

  // ur
  // Get current date and time from RTC
  RtcDateTime now = Rtc.GetDateTime();
  Serial.print(now.Day());
  Serial.print('/');
  Serial.print(now.Month());
  Serial.print('/');
  Serial.print(now.Year());
  Serial.print("\t");
  Serial.print(now.Hour());
  Serial.print(':');
  Serial.print(now.Minute());
  Serial.print(':');
  Serial.print(now.Second());
  Serial.print("\t");
  Serial.print("\n");

  /// print på lcd
  lcd.setCursor(0, 0);
  lcd.print(now.Day());
  lcd.print('/');
  lcd.print(now.Month());
  lcd.print('/');
  lcd.print(now.Year());

  lcd.setCursor(0, 1);
  lcd.print(now.Hour());
  lcd.print(':');
  lcd.print(now.Minute());
  lcd.print(':');
  lcd.print(now.Second());

  delay(2000);
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



  // Gem på sd kort
  myFile = SD.open("Karse.v1.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    // dag og tid
    myFile.print(now.Day());
    myFile.print('/');
    myFile.print(now.Month());
    myFile.print('/');
    myFile.print(now.Year());
    myFile.print("\t");
    myFile.print(now.Hour());
    myFile.print(':');
    myFile.print(now.Minute());
    myFile.print(':');
    myFile.print(now.Second());
    myFile.print("\t");
    myFile.print("\n");

    // temp og fugt
    myFile.print("Temperature: ");
    myFile.print(temperature);
    myFile.print("C ");
    myFile.print("Humidity: ");
    myFile.print(humidity);
    myFile.print("%");
    myFile.print("\n");


    // dist
    myFile.print("Distance: ");
    myFile.print(distanceMm);
    myFile.print("mm ");
    myFile.print("\n");


    // lys
    myFile.print("Lys: ");
    myFile.print(light);
    myFile.print("\n");
    myFile.println("");


    myFile.close();
    delay(1000);

  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }


// Led lys
/// temp mellem 15 og 23
  if (DHT.temperature > 15 && DHT.temperature < 23) {
    digitalWrite(LedTempH, HIGH);
  } else {
    digitalWrite(LedTempL, HIGH);
  }


/// fugt mellem 40 og 70
  if (DHT.humidity > 15 && DHT.humidity < 23) {
    digitalWrite(LedHuH, HIGH);
  } else {
    digitalWrite(LedHuL, HIGH);
  }



  // venter inden loopet starter igen
  delay(2000);
}
