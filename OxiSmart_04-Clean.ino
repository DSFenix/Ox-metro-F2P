//Oxímetro de bajo costo basado en IOT controlado por NodeMCU y MAX30102 orientado a la población no especializada del Perú en 2022.
//Enrique Dobbertin Sánchez
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
//Datos Wifi
#include <WiFi.h>
#include <ThingSpeak.h>

MAX30105 particleSensor;

#define MAX_BRIGHTNESS 255

uint32_t irBuffer[100];
uint32_t redBuffer[100];
int32_t bufferLength;
int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

byte pulseLED = 11;
byte readLED = 13;

//WIFI

const char* ssid="WIFI NAME";
const char* password="WIFI PASS";
unsigned long channelID = 18xxx72;
const char* WriteAPIKey ="UIxxxxxxxxxxxZ06";
WiFiClient cliente;

void setup()
{
  Serial.begin(115200);
 //WIFI
  WiFi.begin(ssid,password);
  while(WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.print(".");
  }
  /*Una vez conextado, se imprimirá una frase y se iniciará la conexión a la Plataforma usando el cliente definido anteriormente*/
  Serial.println("Conectado al WiFi");
  ThingSpeak.begin(cliente);
  delay(2000);

  //Sensor
  pinMode(pulseLED, OUTPUT);
  pinMode(readLED, OUTPUT);

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST))//400kHz speed
  {
    Serial.println(F("MAX30102 was not found. Please check wiring/power."));
    while (1);
  }
  byte ledBrightness = 60; //Options: 0=Off to 255=50mA
  byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
}

void loop()
{
 bufferLength = 100;
 for (byte i = 0 ; i < bufferLength ; i++)
 maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  while (1)
  {
   for (byte i = 25; i < 100; i++)
   {
    redBuffer[i - 25] = redBuffer[i];
    irBuffer[i - 25] = irBuffer[i];
   }
    for (byte i = 25; i < 100; i++)
    {
      while (particleSensor.available() == false)
        particleSensor.check();
      digitalWrite(readLED, !digitalRead(readLED));
      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample();
      
      if(validSPO2==HIGH){
      Serial.print(F("SPO2=")); 
      Serial.println(spo2, DEC);
      ThingSpeak.writeFields(channelID,WriteAPIKey);
      /*Imprimimos una frase indicando el envío, y agregamos un retardo de 10 segundos*/
      //Serial.println("Datos enviados a ThingSpeak!");
      ThingSpeak.setField(1,spo2);}
     else{Serial.println("Calibrando");}
    }
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  }
}
