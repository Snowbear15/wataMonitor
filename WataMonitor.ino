#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Pin definitions
const int trigPin = 3;
const int echoPin = 2;
const int pump1 = 7;
const int pump2 = 10;
const int sensorPin = A0;

// Container dimension (in cm)
const float L = 32.0;
const float W = 22.0;
const float H = 12.0;

// Ultrasonic sensor variables
long duration;
float h_water; // Water level height in cm (used to replace distance)
float volume;  // Volume in cubic centimeters
float gallons; // Volume in gallons

// Temperature sensor setup
#define ONE_WIRE_BUS 8
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float Celsius = 0;

// Turbidity sensor variables
float volt;
float ntu;

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address, 16 columns, 2 rows

void setup() {
  // Initialize LCD
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  delay(2000);

  // Initialize sensors
  sensors.begin();
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Initialize pumps
  pinMode(pump1, OUTPUT);
  pinMode(pump2, OUTPUT);
  digitalWrite(pump1, LOW);
  digitalWrite(pump2, LOW);

  lcd.clear();
  Serial.begin(9600); // Start serial communication
}

void loop() {
  // Measure water level
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  h_water = duration * 0.0344 / 2;  // Water level (in cm)

  // Reverse the water level measurement (12cm = 0, 11cm = 1, ...)
  h_water = H - h_water;  // Reverse calculation
  h_water = constrain(h_water, 0, H); // Clamp water height between 0 and H

  volume = L * W * h_water;          // Volume in cm³
  gallons = volume / 3785.0;         // Convert to gallons

  // Display water level in gallons and height in cm
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Volume:");
  lcd.print(gallons, 2);
  lcd.print(" gal");

  lcd.setCursor(0, 1);
  lcd.print("Level:");
  lcd.print(h_water, 1);
  lcd.print(" cm");

  delay(2000);

  // Read and display temperature
  sensors.requestTemperatures();
  Celsius = sensors.getTempCByIndex(0);

  lcd.clear();
  if (Celsius != -127.00) {
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(Celsius, 1);
    lcd.print(" C");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Temp Error!");
  }

  delay(2000);

  // Read and display turbidity
  volt = 0;
  for (int i = 0; i < 800; i++) {
    volt += ((float)analogRead(sensorPin) / 1023) * 5;
  }
  volt = volt / 800;

  // Debugging: Log the sensor voltage
  Serial.print("Voltage: ");
  Serial.println(volt);

  // Simplified NTU calculation
  if (volt > 4.5) {
    ntu = 0; // Clear water
  } else if (volt > 3.0) {
    ntu = map(volt * 100, 300, 450, 1000, 0); // Adjust for clearer water
  } else {
    ntu = map(volt * 100, 0, 300, 3000, 1000); // Muddy water
  }

  // Classify turbidity
  String turbidityStatus;
  if (ntu < 500) {
    turbidityStatus = "Clear";
  } else if (ntu > 500 && ntu < 1500) {
    turbidityStatus = "Slightly Muddy";
  } else if (ntu > 1500 && ntu < 1900) {
    turbidityStatus = "Muddy";
  } else {
    turbidityStatus = "Very Muddy";
  }

  // Display turbidity and status
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Turbidity:");
  lcd.setCursor(0, 1);
  lcd.print(turbidityStatus);
  Serial.print("NTU: ");
  Serial.println(ntu);

  delay(2000);
}

// Function to round to a specified number of decimal places
float round_to_dp(float in_value, int decimal_place) {
  float multiplier = pow(10.0, decimal_place);
  return round(in_value * multiplier) / multiplier;
}
