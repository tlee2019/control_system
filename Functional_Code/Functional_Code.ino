#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>

SoftwareSerial SIM900(7, 8); // RX, TX

// All data wire are plugged into port 2 on the Arduino
#define temp_sensors 2
#define relay_switch 10
float temp_in;
float temp_out;

String t_in;
String t_out;
String temp_sense;

String pumpStatus;

String inData = "";
int check = 0;
String Address = "";
String IP = "";


// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
// call the library oneWire, provide a name to your Onewire devices (we called them "Sajjie")
// and provide the pin number of these "Sajje" devices (our pin number is temp_sensor = 2)
OneWire Sajje(temp_sensors);

// Pass our oneWire reference to Dallas Temperature.
// call the library oneWire, name your Dallas library (we called them "Dallas")
// and provide the "Sajje" devices reference to Dallas
// The & symbol in a C++ variable declaration means it's a reference. https://en.wikipedia.org/wiki/Reference_%28C%2B%2B%29
DallasTemperature Dallas(&Sajje);

void dataWrite(String toSend, int tDelay = 500) {
  SIM900.println(toSend);
  delay(tDelay);

  while (SIM900.available()){
     inData = SIM900.readStringUntil('\n');
     if(inData == "10.59.8.243\r"){
        Serial.println("SetupTCP Complete");
        check = 1; 
        Serial.print("Check is: ");
        Serial.println(check);
    }
     Serial.println(inData);
  }
}

void setupTCP() {

  IP = String("10.59.8.243");
  
  while(check == 0){
    
    dataWrite("AT+CREG?");
    dataWrite("AT+CGREG?");
    dataWrite("AT+CMEE=1");
    dataWrite("AT+CGACT?");
    dataWrite("AT+CIPSHUT");
    dataWrite("AT+CSTT=\"hologram\"");      //Set the APN to hologram
    dataWrite("AT+CIICR", 1000);
    dataWrite("AT+CIFSR", 1000);            //Get confirmation of the IP address
    delay(1000);
  }
}


void sendData(String data) {
  String message;
  message = data;
  dataWrite("AT+CIPSTART=\"TCP\",\"cloudsocket.hologram.io\",\"9999\"", 5000);
  dataWrite("AT+CIPSEND", 100);
  dataWrite("{\"k\":\"nEPN%q2_\",\"d\":\"" + message + "\",\"t\":\"data\"}", 100);
  SIM900.write(0x1a);
  delay(1000);
  while (SIM900.available()){
     inData = SIM900.readStringUntil('\n');
     delay(30);
     
     if(inData == "Error\r"){
      check = 0;
      while(check == 0){
      
        dataWrite("AT+CREG?");
        dataWrite("AT+CGREG?");
        dataWrite("AT+CMEE=1");
        dataWrite("AT+CGACT?");
        dataWrite("AT+CIPSHUT");
        dataWrite("AT+CSTT=\"hologram\"");      //Set the APN to hologram
        dataWrite("AT+CIICR", 1000);
        dataWrite("AT+CIFSR", 1000);            //Get confirmation of the IP address
        delay(1000);
        
      }
      }
  }
}

//void getTime(){ 
//  dataWrite("AT+HTTPINIT");
//  dataWrite("AT+HTTPPARA="CID",1");
//  dataWrite("AT+HTTPPARA="URL","http://worldtimeapi.org/api/timezone/America/Los_Angeles");
//  dataWrite("AT+HTTPACTION=0");
//  dataWrite("AT+HTTPREAD");
//  
//}

void setup() {
  SIM900.begin(19200);
  delay(2000);
  Serial.begin(19200);
  delay(2000);
  setupTCP();
  delay(1000);
  //  sendData(105);
  //  Serial.write("LETS GO");

  // Start up the Dallas library
  Dallas.begin();
  Serial.println("Dallas Temperature Begin");

  // setup the pin for pump
  pinMode(relay_switch, OUTPUT);
}

void loop() {
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");

  // Send the command to get temperatures using Dallas Library (same as printing "Done" using the "Serial" library)
  Dallas.requestTemperatures();
  Serial.println("DONE");
  // We use the function ByIndex
  temp_in = Dallas.getTempCByIndex(0);
  temp_out = Dallas.getTempCByIndex(1);
  Serial.print("temp_in: ");
  Serial.print(temp_in);
  Serial.print("/ temp_out: ");
  Serial.println(temp_out);

  temp_in = (temp_in * (1.8)) + 32; //converts Celsius to Fahrenheit
  temp_out = (temp_out * (1.8)) + 32;

  // After we got the temperatures, we can print them here.
  Serial.print("Temperature for water inlet is: ");
  Serial.println(temp_in);
  Serial.print("Temperature for water outlet is: ");
  Serial.println(temp_out);

  if (temp_out - temp_in > 2) {
    Serial.println("turn on the pump.");
    Serial.println(" ");
    digitalWrite(relay_switch, HIGH);
    pumpStatus = "ON";
  }
  else {
    Serial.println("turn off the pump.");
    digitalWrite(relay_switch, LOW);
    Serial.println(" ");
    pumpStatus = "OFF";
  }
  t_in = String(temp_in);
  t_out = String(temp_out);
  temp_sense = "Inlet Temp: " + t_in + " / " + "Outlet Temp: " + t_out + " / " + "Pump Status: " + pumpStatus;
  Serial.println(temp_sense);
  sendData(temp_sense);
  delay(5000);

  //  while(SIM900.available())
  //    Serial.write(SIM900.read());
  //  while (Serial.available()) {
  //    byte b = Serial.read();
  //    if (b == '*')
  //      SIM900.write(0x1a);
  //    else
  //      SIM900.write(b);
  //      SIM900.write(Serial.read());
  //  }

}
