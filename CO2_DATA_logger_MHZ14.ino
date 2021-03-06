#include <SPI.h>

#define CHILD_ID 0
#define CO2_SENSOR_PWM_PIN 3

unsigned long SLEEP_TIME = 30*1000; // Sleep time between reads (in milliseconds)

float valAIQ =0.0;
float lastAIQ =0.0;
unsigned long duration;
long ppm;
/*MySensor gw;
MyMessage msg(CHILD_ID, V_LEVEL);
*/
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------MH-Z14
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"

// A simple data logger for the Arduino analog pins

// how many milliseconds between grabbing data and logging it. 1000 ms is once a second
#define LOG_INTERVAL  10000 // mills between entries (reduce to take more/faster data)

// how many milliseconds before writing the logged data permanently to disk
// set it to the LOG_INTERVAL to write each time (safest)
// set it to 10*LOG_INTERVAL to write all data every 10 datareads, you could lose up to 
// the last 10 reads if power is lost but it uses less power and is much faster!
#define SYNC_INTERVAL 10000 // mills between calls to flush() - to write data to the card
uint32_t syncTime = 0; // time of last sync()

#define ECHO_TO_SERIAL   1 // echo data to serial port
#define WAIT_TO_START    0 // Wait for serial input in setup()

/* the digital pins that connect to the LEDs
#define redLEDpin 2
#define greenLEDpin 3

// The analog pins that connect to the sensors
#define photocellPin 0           // analog 0
#define tempPin 1                // analog 1
#define BANDGAPREF 14            // special indicator that we want to measure the bandgap

#define aref_voltage 3.3         // we tie 3.3V to ARef and measure it with a multimeter!
#define bandgap_voltage 1.1      // this is not super guaranteed but its not -too- off
*/

RTC_DS1307 RTC; // define the Real Time Clock object

// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;

// the logging file
File logfile;


//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------Logger

#include <Adafruit_Sensor.h>

#include <Adafruit_BME280.h>


#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)


Adafruit_BME280 bme; // I2C
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------BME/BMP280

//int led = 9;

int carboni = 0;
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------LED/Variables

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
  
  /* red LED indicates error
  digitalWrite(redLEDpin, HIGH);
*/

  while(1);
}

void setup() {
   // Run serial to connect to a computer
  Serial.begin(9600);

  pinMode(CO2_SENSOR_PWM_PIN, INPUT);
  
  // initialize the SD card
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:

   Serial.println(F("BME280 test"));

    bool status;
    
    // default settings
    status = bme.begin();
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }
    
    Serial.println("-- Default Test --");

    Serial.println();

    delay(100); // let sensor boot up
  pinMode(10, OUTPUT);
 // pinMode(led, OUTPUT);
  
  // see if the card is present and can be initialized:

  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    error("Card failed, or not present");
  }
  Serial.println("card initialized.");
  
  // create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE); 
      break;  // leave the loop!
    }
  }
  
/*  if (! logfile) {
    error("couldnt create file");
  }
  */
  Serial.print("Logging to: ");
  Serial.println(filename);

  // connect to RTC
  Wire.begin();  
  if (!RTC.begin()) {
    logfile.println("RTC failed");
#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
#endif  //ECHO_TO_SERIAL
  }
  

  logfile.println("datetime,CO2_ppm,Corrected_CO2,temp,pressure");    
#if ECHO_TO_SERIAL
  Serial.println("datetime,CO2_ppm,Corrected_CO2,temp,pressure");
#endif //ECHO_TO_SERIAL
 
}

void loop(void)
{
  DateTime now;

  // delay for the amount of time we want between readings
  delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL));
    // log milliseconds since starting
  uint32_t m = millis();
//  logfile.print(m);           // milliseconds since start
  //logfile.print(", ");    
//#if ECHO_TO_SERIAL
  //Serial.print(m);         // milliseconds since start
  //Serial.print(", ");  
//#endif

  // fetch the time
  now = RTC.now();
  // log time
  logfile.print(now.year(), DEC);
  logfile.print("/");
  logfile.print(now.month(), DEC);
  logfile.print("/");
  logfile.print(now.day(), DEC);
  logfile.print(" ");
  logfile.print(now.hour(), DEC);
  logfile.print(":");
  logfile.print(now.minute(), DEC);
  logfile.print(":");
  logfile.print(now.second(), DEC);
  logfile.print('"');
#if ECHO_TO_SERIAL
  Serial.print(now.year(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
  Serial.print('"');
#endif //ECHO_TO_SERIAL

 while(digitalRead(CO2_SENSOR_PWM_PIN) == HIGH) {;}
  //wait for the pin to go HIGH and measure HIGH time
  duration = pulseIn(CO2_SENSOR_PWM_PIN, HIGH, 2000000);
  ppm = 5000 * ((duration/1000) - 2)/1000;
  //Serial.print(ppm);

  float carboni = ppm * 100.0F * ((bme.readTemperature() + 273) * (1013))/(bme.readPressure() * (298));

  logfile.print(", ");    
  logfile.print(ppm); //this is useful for writing the data onto the logger
   logfile.print(", ");    
  logfile.print(carboni); //this is useful for writing the data onto the logger
  logfile.print(", ");    
  logfile.print(bme.readTemperature());
  logfile.print(", ");    
  logfile.print(bme.readPressure() / 100.0F);
#if ECHO_TO_SERIAL
  Serial.print(", ");   
  Serial.print(ppm);
   Serial.print(", ");   
  Serial.print(carboni);
  Serial.print(", ");    
  Serial.print(bme.readTemperature());
  Serial.print(", ");    
  Serial.print(bme.readPressure()/ 100.0F);
  
#endif //ECHO_TO_SERIAL
  logfile.println();
#if ECHO_TO_SERIAL
  Serial.println();
#endif // ECHO_TO_SERIAL

  //digitalWrite(greenLEDpin, LOW);

  // Now we write data to disk! Don't sync too often - requires 2048 bytes of I/O to SD card
  // which uses a bunch of power and takes time
  if ((millis() - syncTime) < SYNC_INTERVAL) return;
  syncTime = millis();
   logfile.flush();
  /*
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  */
}
