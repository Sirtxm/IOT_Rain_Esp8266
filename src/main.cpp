#define BLYNK_TEMPLATE_ID "TMPL6qa0kR4oO"
#define BLYNK_TEMPLATE_NAME "IOTminiproject"
#define BLYNK_AUTH_TOKEN "ObOO0GIbZFrTGdf94tE9twO0YtAQusJ0"

#define LINE_TOKEN "EyJrzVqBHco201JeSLHs98NFCB0ZmoDLcEJ2sk5iVm6"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Servo.h>

int RAIN_SENSOR_PIN = A0;
int LDR_PIN = D0;
int SERVO_PIN = D1;
int RAIN_THRESHOLD = 20;

Servo myservo;

WiFiClientSecure client;
HTTPClient http;

void sendLineNotification(String message);

const int IDLE = 0;
const int RAINING = 1;
const int SUNSHINE = 2;
int state = IDLE;
int previousState = IDLE;

char ssid[] = "TP-Link_A287";
char pass[] = "66845355";



void setup() {
    pinMode(RAIN_SENSOR_PIN, INPUT);
    pinMode(LDR_PIN, INPUT);
    Serial.begin(115200);

    myservo.attach(SERVO_PIN);
    myservo.write(0);

    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi connected");
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop() {
    Blynk.run();

    int rainDetected = analogRead(RAIN_SENSOR_PIN);
    int lightValue = digitalRead(LDR_PIN);
    float rainLevel = map(rainDetected, 0, 1023, 0, 100);

    if (state == IDLE) {
        Serial.println("IDLE MODE");
        delay(1000);

        if (rainDetected > RAIN_THRESHOLD) {
            previousState = state;
            state = RAINING;
        } 
        else if (lightValue == 0 && rainDetected < RAIN_THRESHOLD) {
            previousState = state;
            state = SUNSHINE;
        }
    }
    else if (state == RAINING) {
        delay(1000);
        Blynk.virtualWrite(V1, 0);
        Blynk.virtualWrite(V2, 255);
        Serial.println("RAINING MODE");
        Blynk.virtualWrite(V4, rainLevel);
        Serial.println(rainLevel);
        if (previousState != RAINING) {
            sendLineNotification("Rain detected! Switching to RAINING mode.");
            myservo.write(0);
            Blynk.virtualWrite(V0, 0);
            previousState = RAINING;
        }

        if (lightValue == 0 && rainDetected < RAIN_THRESHOLD) {
            previousState = state;
            state = SUNSHINE;
        }
    }
    else if (state == SUNSHINE) {
        delay(1000);
        Blynk.virtualWrite(V1, 255);
        Blynk.virtualWrite(V2, 0);
        Serial.println("SUNSHINE MODE");

        if (previousState != SUNSHINE) {
            sendLineNotification("Sunshine detected! Switching to SUNSHINE mode.");
            myservo.write(180);
            Blynk.virtualWrite(V0, 255);
            previousState = SUNSHINE;
        }

        if (rainDetected > RAIN_THRESHOLD) {
            previousState = state;
            state = RAINING;
        }
    }
}

void sendLineNotification(String message)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        client.setInsecure();  
        http.begin(client, "https://notify-api.line.me/api/notify");

        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        http.addHeader("Authorization", "Bearer " + String(LINE_TOKEN));

        String payload = "message=" + message;
        int httpResponseCode = http.POST(payload);

        if (httpResponseCode > 0) {
            Serial.print("LINE Notify Response Code: ");
            Serial.println(httpResponseCode);
        } else {
            Serial.print("Error sending LINE Notify: ");
            Serial.println(httpResponseCode);
        }
        http.end();
    } else {
        Serial.println("WiFi not connected");
    }
}

BLYNK_WRITE(V0) {
    int pinValue = param.asInt(); // อ่านค่าจาก Switch
    if (pinValue == 1) { // ถ้า Switch ON
        myservo.write(180); // หมุนเซอร์โวไปที่มุม 90 องศา
        Serial.println("Servo moved to 90 degrees");
    } else { // ถ้า Switch OFF
        myservo.write(0); // หมุนเซอร์โวกลับไปที่มุม 0 องศา
        Serial.println("Servo moved to 0 degrees");
    }
}

