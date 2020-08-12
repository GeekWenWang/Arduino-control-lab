#include <SoftwareSerial.h>
#include <Stepper.h>

const byte numChars = 32;
char receivedChars[numChars];

boolean newData = false;

const int stepsPerRevolution = 2048;  
// change this to fit the number of steps per revolution
// for your motor

// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, 8, 10, 9, 11);

SoftwareSerial wifiSerial(2, 3);      // RX, TX for ESP8266

bool DEBUG = true;   //show more logs
int responseTime = 10; //communication timeout

void setup()
{
  // set the speed at 60 rpm:
  myStepper.setSpeed(10);
  pinMode(13,OUTPUT);  //set build in led as output
  // Open serial communications and wait for port to open esp8266:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  wifiSerial.begin(115200);
  while (!wifiSerial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  sendToWifi("AT+CWMODE=2",responseTime,DEBUG); // configure as access point
  sendToWifi("AT+CIFSR",responseTime,DEBUG); // get ip address
  sendToWifi("AT+CIPMUX=1",responseTime,DEBUG); // configure for multiple connections
  sendToWifi("AT+CIPSERVER=1,80",responseTime,DEBUG); // turn on server on port 80
  sendToWifi("AT+CIPSTO=180",responseTime,DEBUG); // sever connection time
  //Serial.println((char)wifiSerial.read());
 
  sendToUno("Wifi connection is running!",responseTime,DEBUG);
  

}


void loop()
{
  if(Serial.available()>0){
     String message = readSerialMessage();
    if(find(message,"debugEsp8266:")){
      String result = sendToWifi(message.substring(13,message.length()),responseTime,DEBUG);
      if(find(result,"OK"))
        sendData("\nOK");
      else
        sendData("\nEr");
    }
  }
  if(wifiSerial.available()>0){
    String message = "";
    Serial.println(message);
    while(message == "" || message == "0,CLOSED" || message == "SEND OK" || message == "0,CONNECT"){
      delay(500);
      message = readWifiSerialMessage();
    }
    //Serial.println("start");
    Serial.println(message);
    //Serial.println("end");
    
    if(find(message,"ESP8266")){
       String result = sendToWifi("AT+GMR",responseTime,DEBUG);
      if(find(result,"OK"))
        sendData("\n"+result);
      else
        sendData("\nErrRead");               //At command ERROR CODE for Failed Executing statement
    }else
    if(find(message,"HELLO")){  //receives HELLO from wifi
        sendData("\nHI!");    //arduino says HI
        message = "";
    }else if(find(message,"LEDON")){
      //turn on built in LED:
      digitalWrite(13,HIGH);
      message = "";
    }else if(find(message,"LEDOFF")){
      //turn off built in LED:
      digitalWrite(13,LOW);
      message = "";;
    }else if(find(message,"MOTORON")){
      Serial.println("MOTORON");
      myStepper.step(stepsPerRevolution);
      message = "";    
    }else if(find(message,"MOTOROFF")){
      myStepper.step(-stepsPerRevolution); 
      message = "";
    }
    else{
      sendData("\nErrRead");                 //Command ERROR CODE for UNABLE TO READ
      //message = "";
    }
  }
  //delay(3000);
}


/*
* Name: sendData
* Description: Function used to send string to tcp client using cipsend
* Params: 
* Returns: void
*/
void sendData(String str){
  String len="";
  len+=str.length();
  sendToWifi("AT+CIPSEND=0,"+len,responseTime,DEBUG);
  delay(100);
  sendToWifi(str,responseTime,DEBUG);
  delay(100);
  //sendToWifi("AT+CIPCLOSE=5",responseTime,DEBUG);
}


/*
* Name: find
* Description: Function used to match two string
* Params: 
* Returns: true if match else false
*/
boolean find(String string, String value){
  return string.indexOf(value)>=0;
}


/*
* Name: readSerialMessage
* Description: Function used to read data from Arduino Serial.
* Params: 
* Returns: The response from the Arduino (if there is a reponse)
*/
String  readSerialMessage(){
  char value[100]; 
  int index_count =0;
  while(Serial.available()>0){
    value[index_count]=Serial.read();
    if(value[index_count]=='<' || value[index_count]=='>'){
      value[index_count] = '\0';
    }
    else{
      index_count++;
      value[index_count] = '\0'; // Null terminate the string
    }
    
  }
  String str(value);
  str.trim();
  return str;
}



/*
* Name: readWifiSerialMessage
* Description: Function used to read data from ESP8266 Serial.
* Params: 
* Returns: The response from the esp8266 (if there is a reponse)
*/
String  readWifiSerialMessage(){
  char value[100]; 
  int index_count =0;
  while(wifiSerial.available()>0){
    value[index_count]=wifiSerial.read();
    if(value[index_count]=='<' || value[index_count]=='>'){
      value[index_count] = '\0';
    }
    else{
      index_count++;
      value[index_count] = '\0'; // Null terminate the string
    }
  }
  //Serial.println(value);
  String str(value);
  str.trim();
  return str;
}



/*
* Name: sendToWifi
* Description: Function used to send data to ESP8266.
* Params: command - the data/command to send; timeout - the time to wait for a response; debug - print to Serial window?(true = yes, false = no)
* Returns: The response from the esp8266 (if there is a reponse)
*/
String sendToWifi(String command, const int timeout, boolean debug){
  String response = "";
  wifiSerial.println(command); // send the read character to the esp8266
  long int time = millis();
  while( (time+timeout) > millis())
  {
    while(wifiSerial.available())
    {
    // The esp has data so display its output to the serial window 
    char c = wifiSerial.read(); // read the next character.
    response+=c;
    }  
  }
  if(debug)
  {
    Serial.println(response);
  }
  return response;
}

/*
* Name: sendToUno
* Description: Function used to send data to Arduino.
* Params: command - the data/command to send; timeout - the time to wait for a response; debug - print to Serial window?(true = yes, false = no)
* Returns: The response from the esp8266 (if there is a reponse)
*/
String sendToUno(String command, const int timeout, boolean debug){
  String response = "";
  Serial.println(command); // send the read character to the esp8266
  long int time = millis();
  while( (time+timeout) > millis())
  {
    while(Serial.available())
    {
      // The esp has data so display its output to the serial window 
      char c = Serial.read(); // read the next character.
      response+=c;
    }  
  }
  if(debug)
  {
    Serial.println(response);
  }
  return response;
}

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
 
    while (wifiSerial.available() > 0 && newData == false) {
        rc = wifiSerial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}
