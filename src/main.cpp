// #include <Arduino.h>
// #include <Wire.h>
// #include <Adafruit_Sensor.h>
// #include <Adafruit_BME680.h>

// #include <WiFi.h>
// #include <WiFiUdp.h>

// #include "config.h"

// #define SDA_PIN A4 // Broche SDA de l'ESP32 Nano
// #define SCL_PIN A5 // Broche SCL de l'ESP32 Nano

// Adafruit_BME680 bme; // I2C

// unsigned int local_port = 10000;
// unsigned int dest_port = 10000;
// WiFiUDP udp;

// IPAddress dest_ip; // Server address

// void displayIPaddress(const IPAddress address, unsigned int port);

// void setup()
// {
//     Serial.begin(115200);
//     Serial.println("⏳ Initialisation du BME680...");

//     Wire.begin(SDA_PIN, SCL_PIN); // Initialise I2C avec les bonnes broches

//     if (!bme.begin())
//     {
//         Serial.println("❌ Erreur : BME680 non détecté !");
//         while (1)
//             ;
//     }

//     Serial.println("✅ BME680 détecté !");

//     WiFi.mode(WIFI_STA); // Optional
//     WiFi.begin(ssid, password);
//     Serial.println("\nWIfi Connecting");

//     while (WiFi.status() != WL_CONNECTED)
//     {
//         Serial.print(".");
//         delay(100);
//     }

//     Serial.println("\nConnected to the WiFi network");
//     Serial.print("Local ESP32 IP: ");
//     Serial.println(WiFi.localIP());

//     // Begin udp port
//     Serial.println("Remote addresse: ");
//     displayIPaddress(dest_ip, dest_port);
// }

// void loop()
// {
//     if (!bme.performReading())
//     {
//         Serial.println("❌ Erreur de lecture des données !");
//         return;
//     }

//     // Serial.print("Température : ");
//     // Serial.print(bme.temperature);
//     // Serial.println(" °C");

//     // Serial.print("Pression : ");
//     // Serial.print(bme.pressure / 100.0);
//     // Serial.println(" hPa");

//     // Serial.print("Humidité : ");
//     // Serial.print(bme.humidity);
//     // Serial.println(" %");

//     // Serial.print("Gaz : ");
//     // Serial.print(bme.gas_resistance / 1000.0);
//     // Serial.println(" kΩ");

//     udp.begin(local_port);
//     char text[100] = {0};
//     sprintf(text, "{ \"Temp\" : %.2f, \"Press\" : %.2f, \"Hum\" : %.2f, \"Gaz\" : %.2f }", bme.temperature, bme.pressure / 100.0, bme.humidity, bme.gas_resistance / 1000.0);
//     Serial.print("Sending to: ");
//     Serial.println(dest_ip);
//     Serial.println(text);
//     udp.beginPacket(dest_ip, dest_port);
//     udp.print(text);
//     udp.endPacket();

//     delay(5000);
// }

// void displayIPaddress(const IPAddress address, unsigned int port)
// {
//     Serial.print(" IP ");
//     for (int i = 0; i < 4; i++)
//     {
//         Serial.print(address[i], DEC);
//         if (i < 3)
//             Serial.print(".");
//     }
//     Serial.print(" port ");
//     Serial.println(port);
// }

// test envoi direct en wifi avec wifi iphone

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

#include <WiFi.h>
#include <Firebase_ESP_Client.h> // Firebase Library
#include "addons/TokenHelper.h"  // Aide pour le Token Firebase
#include "addons/RTDBHelper.h"   // Aide pour Real-Time Database
#include "config.h"              // ⚠️ Ton fichier de configuration

// 🔥 Déclaration du capteur BME680
Adafruit_BME680 bme;

// 🔥 Déclaration Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// 🔥 WiFi et Firestore
void connectWiFi();
void connectFirebase();
void sendDataToFirestore();

// 🔥 Configuration de l'ESP32
void setup()
{
    Serial.begin(115200);
    Serial.println("⚙️ Démarrage de l'ESP32...");
    delay(2000); // Attente pour bien voir le message
    Serial.println("⏳ Initialisation du BME680...");

    Wire.begin(A4, A5); // Initialisation I2C

    if (!bme.begin())
    {
        Serial.println("❌ Erreur : BME680 non détecté !");
        while (1)
            ;
    }
    Serial.println("✅ BME680 détecté !");

    connectWiFi();
    connectFirebase();
}

// 🔥 Boucle principale (envoi toutes les 5 secondes)
void loop()
{
    if (!bme.performReading())
    {
        Serial.println("❌ Erreur de lecture des données !");
        return;
    }

    sendDataToFirestore();
    delay(10000);
}

// 🔥 Connexion WiFi
void connectWiFi()
{
    Serial.println("⏳ Connexion au WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) // 🔥 Limite d'attente de 10 secondes
    {
        Serial.print(".");
        delay(500);
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\n✅ Connecté au WiFi !");
        Serial.print("📡 Adresse IP : ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println("❌ Échec de connexion WiFi !");
    }
}

// 🔥 Connexion à Firebase Firestore
void connectFirebase()
{
    Serial.println("⏳ Connexion à Firebase...");

    config.database_url = FIREBASE_HOST;

    Serial.println("⚡ Configuration Firebase avec Service Account...");

    // 🔥 Assigner la clé privée correctement (remplacer les `\n` par des sauts de ligne valides)
    String privateKey = FIREBASE_PRIVATE_KEY;
    privateKey.replace("\\n", "\n"); // Firebase requiert des sauts de ligne valides

    config.signer.tokens.legacy_token = FIREBASE_AUTH; // Si tu utilises l'auth par clé API
    config.service_account.data.project_id = FIREBASE_PROJECT_ID;
    config.service_account.data.client_email = FIREBASE_CLIENT_EMAIL;
    config.service_account.data.private_key = privateKey.c_str(); // Conversion String -> char*

    Serial.println("🔗 Connexion Firebase...");

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    Serial.println("⏳ Vérification de la connexion Firebase...");
    if (Firebase.ready())
    {
        Serial.println("✅ Firebase connecté !");
    }
    else
    {
        Serial.println("❌ Échec de connexion Firebase !");
    }
}


// 🔥 Envoi des données capteur vers Firestore
void sendDataToFirestore()
{
    Serial.println("⏳ Préparation des données Firestore...");

    // ⏳ Obtenir l'heure actuelle du serveur NTP (UTC)
    time_t now;
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("❌ Impossible d'obtenir l'heure");
        return;
    }

    // 🔥 Obtenir un vrai timestamp UNIX en secondes depuis 1970
    time(&now);
    now += 3600;

    char dateStr[11]; // "YYYY-MM-DD"
    char hourStr[6];  // "HH:00"
    strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeinfo);
    strftime(hourStr, sizeof(hourStr), "%H:00", &timeinfo);

    // 📆 Affichage de la date et de l'heure dans la console
    Serial.print("📆 Date : ");
    Serial.println(dateStr);
    Serial.print("⏰ Heure : ");
    Serial.println(hourStr);

    // 📌 🔥 Génération d'un identifiant unique basé sur le timestamp UNIX
    String uniqueId = String(now);

    // 🔴 📂 **Mise à jour des données en temps réel**
    String livePath = "test/Ua4bgGFb4ibdrZohzulN/live/snapshot";

    FirebaseJson liveData;
    liveData.set("fields/timestamp/integerValue", now);  // ✅ Utilisation du vrai timestamp UNIX
    liveData.set("fields/temperature/doubleValue", bme.temperature);
    liveData.set("fields/humidity/doubleValue", bme.humidity);
    liveData.set("fields/pressure/doubleValue", bme.pressure / 100.0);
    liveData.set("fields/gas_resistance/doubleValue", bme.gas_resistance / 1000.0);

    Serial.println("📤 Mise à jour des données en temps réel...");
    if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", livePath.c_str(), liveData.raw(), "timestamp,temperature,humidity,pressure,gas_resistance"))
    {
        Serial.println("✅ Temps réel mis à jour !");
    }
    else
    {
        Serial.print("❌ Erreur mise à jour temps réel : ");
        Serial.println(fbdo.errorReason());
    }

    // 🔵 📂 **Ajout des données historiques**
    String historyPath = "test/";             
    historyPath += "Ua4bgGFb4ibdrZohzulN";    
    historyPath += "/dates/";                 
    historyPath += dateStr;                    
    historyPath += "/hours/";                  
    historyPath += hourStr;                    
    historyPath += "/";                        
    historyPath += uniqueId;                    

    FirebaseJson historyData;
    historyData.set("fields/timestamp/integerValue", now);  // ✅ Utilisation du vrai timestamp UNIX
    historyData.set("fields/temperature/doubleValue", bme.temperature);
    historyData.set("fields/humidity/doubleValue", bme.humidity);
    historyData.set("fields/pressure/doubleValue", bme.pressure / 100.0);
    historyData.set("fields/gas_resistance/doubleValue", bme.gas_resistance / 1000.0);

    Serial.println("📤 Ajout des données historiques...");
    if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "", historyPath.c_str(), historyData.raw()))
    {
        Serial.println("✅ Données enregistrées avec succès !");
    }
    else
    {
        Serial.print("❌ Échec ajout historique : ");
        Serial.println(fbdo.errorReason());
    }
}