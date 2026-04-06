#include <SimpleDHT.h>
#include <Wire.h>
#include <SFE_BMP180.h>

// ----- Pin definitions -----
const int pinDHT11 = 2;         // DHT11 data pin
const int pinWaterSensor = 3;   // Water sensor digital pin (Test/D0)

// ----- Sensor objects -----
SimpleDHT11 dht11(pinDHT11);
SFE_BMP180 bmp;

// ----- Calibration offsets -----
const int tempOffset = 2;       // °C correction for DHT11
const int humOffset = 5;        // %RH correction for DHT11
const double pressureOffset = -500; // Pa correction for BMP180

// Function prototype
void readBmpSensor(double &pressure);

void setup() {
  Serial.begin(9600);  
  Serial.println("CLEARDATA");
  Serial.println("RESETTIMER");
  Serial.println("LABEL:Temperature,Humidity,Pressure,Water");

  // BMP180 initialization
  if (!bmp.begin()) {
    Serial.println("BMP180 init fail!");
    while (1);
  }

  // Water sensor pin setup
  pinMode(pinWaterSensor, INPUT);
}

void loop() {
  // --- Read DHT11 ---
  byte temperature = 0;
  byte humidity = 0;
  int err = dht11.read(&temperature, &humidity, NULL);

  double pressure = 0;

  if (err == SimpleDHTErrSuccess) {
    // Apply DHT11 calibration
    int calibratedTemp = (int)temperature + tempOffset;
    int calibratedHum  = (int)humidity + humOffset;

    // --- Read BMP180 ---
    readBmpSensor(pressure);
    pressure += pressureOffset;

    // --- Read Water Sensor ---
    int waterState = digitalRead(pinWaterSensor); // LOW = water detected

    // --- Print readings to virtual terminal ---
    Serial.print("DATA:");
    Serial.print(calibratedTemp); Serial.print(",");
    Serial.print(calibratedHum);  Serial.print(",");
    Serial.print(pressure);       Serial.print(",");
    Serial.println(waterState == LOW ? "Water Detected" : "No Water");

  } else {
    Serial.print("DHT11 read error: ");
    Serial.println(err);
  }

  delay(1500); // DHT11 max rate ~1 Hz
}

// ----- Read BMP180 -----
void readBmpSensor(double &P) {
  char status;
  double T;

  status = bmp.startTemperature();
  if (status != 0) {
    delay(status);
    status = bmp.getTemperature(T);
    if (status != 0) {
      status = bmp.startPressure(3); // oversampling=3
      if (status != 0) {
        delay(status);
        status = bmp.getPressure(P, T);
        if (status == 0) {
          Serial.println("BMP180 read error");
        }
      } else Serial.println("BMP180 startPressure error");
    } else Serial.println("BMP180 getTemperature error");
  } else Serial.println("BMP180 startTemperature error");
}
