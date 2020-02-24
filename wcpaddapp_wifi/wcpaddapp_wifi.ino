#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>


TinyGPS gps;
SoftwareSerial ss(5, 4);

int ledRed = 14;
int boton = 12;
int valBoton = 1;

unsigned long time1 = 0;
unsigned long time2 = 0;
unsigned long time3 = 0;
float flat, flon;
unsigned long age;
void setup() {

  Serial.begin(9600);
  ss.begin(9600);

  pinMode(ledRed, OUTPUT);
  pinMode(boton, INPUT_PULLUP);

  
  resetWifi();
  time1 = millis();
  time2 = millis();

}

void loop() {

  valBoton = digitalRead(boton);  // read input value
  if (valBoton == LOW) {         // check if the input is HIGH (button released)
    delay(50);
    valBoton = digitalRead(boton);  // read input value
    if (valBoton == LOW) {         // check if the input is HIGH (button released)
      resetWifi();


    }

  }


  if ( millis() > (time1 + 1000)) {
    getGps();
    time1 = millis();
  }

  if ( millis() > (time2 + 15000)) {

    if (WiFi.status() == WL_CONNECTED) {

      http_post();


    } else {

      Serial.println("Error in WiFi connection");

    }

    time2 = millis();
  }




}

void http_post() {
  HTTPClient http;

  http.begin("http://www.wcpaddapp.com/arduino/report2.php");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpCode = http.POST("lat=" + String(flat) + "&lng=" + String(flon) + "&device_id=0002");
  String payload = http.getString();

  Serial.println(httpCode);   //Print HTTP return code
  Serial.println(payload);    //Print request response payload

  http.end();
  int e=0;
  String req = getValue(payload, ',', 0);
  if (req != "ok") {
    e++;
    if (e == 3) {
      e = 0;
      resetWifi();

    }
  }

}


void getGps() {

  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;

  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (ss.available())
    {
      char c = ss.read();
      if (gps.encode(c))
        newData = true;
    }
  }

  if (newData)
  {

    gps.f_get_position(&flat, &flon, &age);

    flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6;
    flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6;
    Serial.println(flat);
    Serial.println(flon);

  }
}

void resetWifi() {
  digitalWrite(ledRed, HIGH);
  WiFi.mode(WIFI_STA);
  WiFi.begin("TP-LINK_7F84", "95056258");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  digitalWrite(ledRed, LOW);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
