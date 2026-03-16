#pragma once

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT11.h>

#include <include/bedridden.hpp>

class Sensors {
    private:
        int temperature;
        int humidity;
        bool motionDetected;
        bool motorRunning;
        uint16_t lightLevel;
        
    public:
        Sensors() : temperature(0), humidity(0), motionDetected(false), motorRunning(false), lightLevel(0) {};

        bool makeApiCall(String api, JsonDocument& doc) 
        {
            if (WiFi.status() != WL_CONNECTED) {
                Serial.println("WiFi Disconnected");
                return false;
            }
            http.useHTTP10(true);
            http.begin(api);
            int httpResponseCode = http.GET();
            
            if (httpResponseCode == 200) {
                doc.clear();
                DeserializationError error = deserializeJson(doc, http.getStream());

                if (!error) {
                    http.end();
                    return true; 
                } else {
                    Serial.print("JSON Parse-error: ");
                    Serial.println(error.f_str());
                }
            } else {
                Serial.print("HTTP Error code: ");
                Serial.println(httpResponseCode);
            }

            http.end();
            return false;
        }

        bool fetchWeather(JsonDocument& doc, const char* weatherApi)
        {
            if (!makeApiCall(weatherApi, doc)) {
                Serial.println("Failed to fetch weather data");
                return false;
            }
            return true;
        }
        
        void fetchAirSensor()
        {
            dht11.readTemperatureHumidity(temperature, humidity);
        }

        void fetchLight()
        {
            int val = analogRead(PIN_PHOTORES);
            lightLevel = (uint16_t)map(val, 0, 4095, 0, 100);
        }
        
        void fetchProximity() 
        {
            motionDetected = (bool)digitalRead(PIN_PROXSENSOR);
        }

        void getAirSensor(int* output) 
        { 
            output[0] = temperature;
            output[1] = humidity; 
        }

};