#include <SoftwareSerial.h>
SoftwareSerial SIM900(8, 7);

int ledGreen = 3;
int ledRed = 2;
int rele1= 6;
int rele2= 7;

int respuesta;
char aux_str[50];
long hour = 0;
String readData = "";


char uri[] = "PUT /arduino/report2.php?";
char variablesHour[100];
char variablesRest[] = "device_id=0001&status=ok&msg=unit-restart ";
char host[] = "HTTP/1.1\r\nHost: www.wcpaddapp.com\r\n\r\n ";


unsigned long time1 = 0;
unsigned long time2 = 0;
unsigned long time3 = 0;

void setup()
{
  SIM900.begin(19200);
  Serial.begin(9600);

  pinMode(ledGreen, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(rele1, OUTPUT);
  pinMode(rele2, OUTPUT);

  digitalWrite(ledGreen, LOW);
  digitalWrite(ledRed, LOW);
   digitalWrite(rele1, HIGH);
  digitalWrite(rele2, HIGH);

  delay(1000);
  Serial.println("Iniciando...");
  power_on();
  iniciar();
  delay(1000);
  PeticionHttpRestart();
  time1 = millis();
  time2 = millis();
  
}

void loop()
{

 
  

  if ( millis() > (time1 + 60000)) {

    hour ++;
    time1 = millis();
  }
  
  if (hour >= 60) {
    hour = 0;
    PeticionHttpHour();
  }



  if ( millis() > (time2 + 2000)) {

    digitalWrite(ledGreen, HIGH);
    delay(300);
    digitalWrite(ledGreen, LOW);
    smsRead();
    time2 = millis();
  }

   if ( millis() > (time3 + 5000)) {
    gps();
    time3 = millis();
  }

  response();
 
}


int enviarAT(String ATcommand, char* resp_correcta, unsigned int tiempo)
{

  int x = 0;
  bool correcto = 0;
  char respuesta[100];
  unsigned long anterior;

  memset(respuesta, '\0', 100);
  delay(100);
  while ( SIM900.available() > 0) SIM900.read();
  SIM900.print(ATcommand);
  x = 0;
  anterior = millis();
  do {

    if (SIM900.available() != 0)
    {
      respuesta[x] = SIM900.read();
      x++;
      if (strstr(respuesta, resp_correcta) != NULL)
      {
        correcto = 1;
      }
    }
  }

  while ((correcto == 0) && ((millis() - anterior) < tiempo));
  Serial.println(respuesta);
  
 


  return correcto;
}

void power_on()
{
  int respuesta = 0;


  if (enviarAT("AT\r\n", "OK", 2000) == 0)
  {
    Serial.println("Encendiendo el GPRS...");

    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);
    delay(1000);
    digitalWrite(2, LOW);
    delay(1000);


    while (respuesta == 0) {
      respuesta = enviarAT("AT\r\n", "OK", 2000);
      SIM900.println(respuesta);
    }
  }
}

void power_off()
{
  digitalWrite(2, HIGH);
  delay(1000);
  digitalWrite(2, LOW);
  delay(1000);
}

void reiniciar()
{
  Serial.println("Reiniciando...");
  power_off();
  delay (5000);
  power_on();
}

void iniciar()
{

  Serial.println("Conectando a la red...");
  delay (5000);


  while ( enviarAT("AT+CREG?\r\n", "+CREG: 0,1", 1000) == 0 )
  {
  }

  Serial.println("Conectado a la red.");


  enviarAT("AT+CGATT=1\r", "OK", 1000);
  enviarAT("AT+CSTT=\"gprs\",\"gprs\",\"gprs\"\r\n", "OK", 3000);
  enviarAT("AT+CIICR\r\n", "OK", 3000);
  enviarAT("AT+CIFSR\r\n", "", 3000);
}

void PeticionHttpRestart()
{
  if (enviarAT("AT+CREG?\r\n", "+CREG: 0,1", 1000) == 1)
  {
    enviarAT("AT+CIPSTART=\"TCP\",\"www.wcpaddapp.com\",\"80\"\r\n\r\n", "CONNECT OK", 5000);
    int len = strlen(uri) + strlen(variablesRest) + strlen(host);
    sprintf(aux_str, "AT+CIPSEND=%d\r\n", len);
    if (enviarAT(aux_str, ">", 10000) == 1)
    {
      enviarAT(uri, "OK", 1000);
      enviarAT(variablesRest, "OK", 1000);
      enviarAT(host, "OK", 1000);


    }
  }
  else
  {
    reiniciar();
    iniciar();
  }
}

void PeticionHttpHour()
{
  if (enviarAT("AT+CREG?\r\n", "+CREG: 0,1", 1000) == 1)
  {
    enviarAT("AT+CIPSTART=\"TCP\",\"www.wcpaddapp.com\",\"80\"\r\n\r\n", "CONNECT OK", 5000);
    int len = strlen(uri) + strlen(variablesHour) + strlen(host);
    sprintf(aux_str, "AT+CIPSEND=%d\r\n", len);
    if (enviarAT(aux_str, ">", 10000) == 1)
    { 
      enviarAT(uri, "OK", 1000);
      enviarAT(variablesHour, "OK", 1000);
      enviarAT(host, "OK", 1000);

    }
  }
  else
  {
    reiniciar();
    iniciar();
  }
}

void gps() {

  enviarAT("AT+CGNSPWR?\r\n","OK",1000);
  enviarAT("AT+CGNSPWR=1\r\n","OK",1000);
  

  int x = 0;
  bool correcto = 0;
  char respuesta[100];
  unsigned long anterior;

  memset(respuesta, '\0', 100);
  delay(100);
  while ( SIM900.available() > 0) SIM900.read();
  SIM900.print("AT+CGNSINF\r\n");
  x = 0;
  anterior = millis();
  do {

    if (SIM900.available() != 0)
    {
      respuesta[x] = SIM900.read();
      x++;
    }
  }

  while ((correcto == 0) && ((millis() - anterior) < 1000));

  String resp=String(respuesta);
  String LAT=getValue(resp,',',3);
  String LNG=getValue(resp,',',4);
  Serial.println(LAT);
  Serial.println(LNG);
  String vars="device_id=0001&status=ok&msg=hourly-report&lat="+LAT+"&lng="+LNG+" ";
  vars.toCharArray(variablesHour, vars.length());
  Serial.println(variablesHour);
  
  
}

void smsRead() {

  enviarAT("AT+CMGF=1\r\n", "OK", 1000);
  enviarAT("AT+CNMI=1,2,0,0,0\r\n", "OK", 1000);
}

void response() {


  if (SIM900.available()) {
    readData = SIM900.readStringUntil('\n');
    if (readData.startsWith("HTTP/1.1", 0)) {
      Serial.println("Got an OK from the server");
      for (int i = 0; i < 6; i++) {
        readData = SIM900.readStringUntil('\n');
      }

      if (readData.startsWith("ok", 0)) {
        Serial.println(readData);
        digitalWrite(ledGreen, HIGH);
        digitalWrite(ledRed, LOW);
      } else {
        digitalWrite(ledGreen, LOW);
        digitalWrite(ledRed, HIGH);
      }

    }
    if (readData.startsWith("+CMT:", 0)) {
      readData = SIM900.readStringUntil('\n');
      Serial.println(readData);
      String r1=getValue(readData,',',1);
      String r2=getValue(readData,',',2);

      if(r1=="1"){
        digitalWrite(rele1, LOW);
        }
      if(r1=="0"){
        digitalWrite(rele1, HIGH);
        }

       if(r2=="1"){
        digitalWrite(rele2, LOW);
        }
      if(r2=="0"){
        digitalWrite(rele2, HIGH);
        }   
    }

    
  }


}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}
