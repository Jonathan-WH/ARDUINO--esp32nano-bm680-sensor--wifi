#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

#include <WiFi.h>
#include <WiFiUdp.h>

#include "config.h"

#define SDA_PIN A4 // Broche SDA de l'ESP32 Nano
#define SCL_PIN A5 // Broche SCL de l'ESP32 Nano

Adafruit_BME680 bme; // I2C

unsigned int local_port = 10000;
unsigned int dest_port = 10000;
WiFiUDP udp;

IPAddress dest_ip; // Server address

void displayIPaddress(const IPAddress address, unsigned int port);

void setup()
{
    Serial.begin(115200);
    Serial.println("⏳ Initialisation du BME680...");

    Wire.begin(SDA_PIN, SCL_PIN); // Initialise I2C avec les bonnes broches

    if (!bme.begin())
    {
        Serial.println("❌ Erreur : BME680 non détecté !");
        while (1)
            ;
    }

    Serial.println("✅ BME680 détecté !");

    WiFi.mode(WIFI_STA); // Optional
    WiFi.begin(ssid, password);
    Serial.println("\nWIfi Connecting");

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(100);
    }

    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());

    // Begin udp port
    Serial.println("Remote addresse: ");
    displayIPaddress(dest_ip, dest_port);
}

void loop()
{
    if (!bme.performReading())
    {
        Serial.println("❌ Erreur de lecture des données !");
        return;
    }

    // Serial.print("Température : ");
    // Serial.print(bme.temperature);
    // Serial.println(" °C");

    // Serial.print("Pression : ");
    // Serial.print(bme.pressure / 100.0);
    // Serial.println(" hPa");

    // Serial.print("Humidité : ");
    // Serial.print(bme.humidity);
    // Serial.println(" %");

    // Serial.print("Gaz : ");
    // Serial.print(bme.gas_resistance / 1000.0);
    // Serial.println(" kΩ");

    udp.begin(local_port);
    char text[100] = {0};
    sprintf(text, "{ \"Temp\" : %.2f, \"Press\" : %.2f, \"Hum\" : %.2f, \"Gaz\" : %.2f }", bme.temperature, bme.pressure / 100.0, bme.humidity, bme.gas_resistance / 1000.0);
    Serial.print("Sending to: ");
    Serial.println(dest_ip);
    Serial.println(text);
    udp.beginPacket(dest_ip, dest_port);
    udp.print(text);
    udp.endPacket();

    delay(5000);
}

void displayIPaddress(const IPAddress address, unsigned int port)
{
    Serial.print(" IP ");
    for (int i = 0; i < 4; i++)
    {
        Serial.print(address[i], DEC);
        if (i < 3)
            Serial.print(".");
    }
    Serial.print(" port ");
    Serial.println(port);
}