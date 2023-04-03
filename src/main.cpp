#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include "SPIFFS.h"
 
const char *ssid = "Nordnet_EF2D";
const char *password = "DA33M73D";
const char* serverName = "https://api.sunrise-sunset.org/json?lat=48.8566&lng=2.3522&formatted=0";  // Coordonnées GPS de Paris

unsigned long currentMillis = millis();

const int doorPin = 27;

unsigned long previousMillis = 0; // Temps depuis le dernier appel à l'API
const long interval = 24 * 60 * 60 * 1000; // Interval entre deux appels à l'API (1 jour)

AsyncWebServer server(80);

void setup()
{
  //----------------------------------------------------Serial
  Serial.begin(115200);
  Serial.println("\n");

  //----------------------------------------------------GPIO
  pinMode(doorPin, OUTPUT);

  //----------------------------------------------------SPIFFS
  if(!SPIFFS.begin())
  {
    Serial.println("Erreur SPIFFS...");
    return;
  }

  File root = SPIFFS.open("/");
  File file = root.openNextFile();

  while(file)
  {
    Serial.print("File: ");
    Serial.println(file.name());
    file.close();
    file = root.openNextFile();
  }

  //----------------------------------------------------WIFI
  WiFi.begin(ssid, password);
	Serial.print("Tentative de connexion...");
	
	while(WiFi.status() != WL_CONNECTED)
	{
		Serial.print(".");
		delay(200);
	}
	
	Serial.println("\n");
	Serial.println("Connexion etablie!");
	Serial.print("Adresse IP: ");
	Serial.println(WiFi.localIP());
  Serial.print("Force du signal :");
  Serial.println(WiFi.RSSI());
  //----------------------------------------------------SERVER
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/w3.css", "text/css");
  });

  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/script.js", "text/javascript");
  });

  server.on("/lireEtatPorte", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    int val = touchRead(T0);
    if(val > 50)
    {
      val = 0;
    } else {
      val = 1;
    };
    String porte = String(val);
    request->send(200, "text/plain", porte);
  });

  server.on("/open", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    Serial.println("Ouverture de la porte");
    pinMode(doorPin, LOW);
    // TODO : Test avec moteur
    request->send(200);
  });

  server.on("/close", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    Serial.println("Fermeture de la porte");
    pinMode(doorPin, HIGH);
    // TODO : Test avec moteur
    request->send(200);
  });

  server.begin();
  Serial.println("Serveur actif!");
}



void loop()
{
  // Appel à l'API tous les jours
  if (currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;

    // Connexion à l'API pour récupérer le lever et le coucher du soleil
    HTTPClient http;
    http.begin(serverName);
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
      String payload = http.getString();
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);

      const char* heureLever = doc["results"]["sunrise"]; // Heure de lever du soleil au format "hh:mm:ss AM/PM"
      const char* heureCoucher = doc["results"]["sunset"]; // Heure de coucher du soleil au format "hh:mm:ss AM/PM"

      int heureLeverHeures = atoi(strtok((char*)heureLever, ": "));
      int heureLeverMinutes = atoi(strtok(NULL, ": "));
      int heureLeverSecondes = atoi(strtok(NULL, ": "));
      if (strcmp(strtok(NULL, ": "), "PM") == 0 && heureLeverHeures < 12) {
        heureLeverHeures += 12;
      }

      int heureCoucherHeures = atoi(strtok((char*)heureCoucher, ": "));
      int heureCoucherMinutes = atoi(strtok(NULL, ": "));
      int heureCoucherSecondes = atoi(strtok(NULL, ": "));
      if (strcmp(strtok(NULL, ": "), "PM") == 0 && heureCoucherHeures < 12) {
        heureCoucherHeures += 12;
      }

      // Conversion de l'heure de Paris (UTC+1) en heures locales
      heureLeverHeures += 1;
      heureCoucherHeures += 1;

      Serial.print("Heure de lever du soleil : ");
      Serial.print(heureLeverHeures);
      Serial.print(":");
      Serial.print(heureLeverMinutes);
      Serial.print(":");
      Serial.println(heureLeverSecondes);

      Serial.print("Heure de coucher du soleil : ");
      Serial.print(heureCoucherHeures);
      Serial.print(":");
      Serial.print(heureCoucherMinutes);
      Serial.print(":");
      Serial.println(heureCoucherSecondes);
    }
  }
  // TODO : Ouvrir la porte à l'heure de lever du soleil et la fermer à l'heure de coucher du soleil
}