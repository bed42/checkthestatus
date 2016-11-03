#include <SoftwareSerial.h>
#include <String.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

#define BUFFSIZ 90
#define I2C_ADDR 0x27 // <<- Add your address here.
#define Rs_pin 0
#define Rw_pin 1
#define En_pin 2
#define BACKLIGHT_PIN 3
#define D4_pin 4
#define D5_pin 5
#define D6_pin 6
#define D7_pin 7
LiquidCrystal_I2C lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

char buffer[BUFFSIZ];
char buffidx;
int retry = 5;
int value;
int dLine[30];
int sendCommand(char* command, char* resp, int delayTime=500, int reps=5, int numData=2);
boolean config = false;
boolean power = false;
SoftwareSerial mySerial(10, 3);
//Constants
#define DHTPIN 33     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino



const byte interruptPin=19;
volatile byte panicState = 0;
 String deviceLightStatus="";
 String deviceFanStatus="";

void setup()
{

  
  pinMode(23, OUTPUT);//light
  pinMode(25, OUTPUT);//fan
  
  pinMode(31, INPUT);//Door
  pinMode(33, INPUT);//Temp
  
  pinMode(35, INPUT);//Smoke
  pinMode(interruptPin,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin),storePanicState,LOW );
  
 mySerial.begin(19200); // the GPRS baud rate 
 Serial.begin(9600); // the GPRS baud rate 
 delay(500);
 digitalWrite(23, HIGH);
 digitalWrite(25, HIGH);
 dht.begin();

lcd.begin (20,4); // <<-- our LCD is a 20x4, change for your LCD if needed
// LCD Backlight ON
lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
lcd.setBacklight(HIGH);
lcd.home (); // go home on LCD
 
 initializeHttp();
}

void storePanicState(){
  panicState=1;
}
void initializeHttp(){
   powerOn();
 mySerial.println("AT+CSQ");
 delay(100);
 showSerialData();// this code is to show the data from gprs shield, 
 //in order to easily see the process of
 //how the gprs shield submit a http request, and the following is for this purpose too.
 mySerial.println("AT+CGATT?");
 delay(100);
 showSerialData();
 mySerial.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");//setting the SAPBR, the connection 
 //type is using gprs
 delay(1000);
 showSerialData();

 mySerial.println("AT+SAPBR=3,1,\"APN\",\"live.vodafone.com\"");//setting the APN, the second need you fill in your local apn server
 //the second need you fill in your local apn server
 delay(4000);
 showSerialData();
 mySerial.println("AT+SAPBR=1,1");//setting the SAPBR, for detail you can refer to 
 //the AT command mamual
 delay(2000);
 showSerialData();
 mySerial.println("AT+HTTPINIT"); //init the HTTP request
 delay(4000); 
 showSerialData();

}

void loop()
{
    submitHttpRequest();
}

void parseGetRequests(){
String responseStr = "";
while (mySerial.available() !=0) {
    char in = mySerial.read();
     responseStr=responseStr+String(in);
}

  if(responseStr.indexOf("light")>0){
     int startIndexOfLight=responseStr.indexOf("light")+6;
     int endIndexOfLight=startIndexOfLight+1;
     deviceLightStatus = responseStr.substring(startIndexOfLight, endIndexOfLight);
     int lightSensor=deviceLightStatus.toInt();
     
     if(lightSensor==1){
      digitalWrite(23, LOW);
     }else{
      digitalWrite(23, HIGH);
     }
     Serial.println("Light status is "+deviceLightStatus);
  }
  if(responseStr.indexOf("fan")>0){
    int startIndexOfFan=responseStr.indexOf("fan")+4;
    int endIndexOfFan=startIndexOfFan+1;
    deviceFanStatus = responseStr.substring(startIndexOfFan, endIndexOfFan);
    int fanSensor=deviceFanStatus.toInt();
    if(fanSensor==1){
      digitalWrite(25, LOW);
     }else{
      digitalWrite(25, HIGH);
     }
    Serial.println("Fan status is "+deviceFanStatus);
  }
  
}

void submitHttpRequest()
{
              float tempreatureSensorData = dht.readTemperature();
              String stringTemp=String(tempreatureSensorData,2);
            
              float humiditySensorData = dht.readHumidity();;
              String stringHum=String(humiditySensorData,2);

              int doorSensorData = digitalRead(31);
              String stringDoor=String((doorSensorData)==1?0:1);// we are inverting bcozdoor sensor is programmed opposite //shrug
            
              int smokeSensorData = digitalRead(35);
              String stringSmoke=String((smokeSensorData)==1?0:1);// we are inverting bcoz smoke sensor is programmed opposite //shrug
              
              byte panicSensorData=panicState;
              String devicePanicStatus=String(panicSensorData);


              
              
            //this is for actuating the command
            
             mySerial.println("AT+HTTPPARA=\"URL\",\"http://139.59.249.126:4242/getrequests\"");// setting the httppara, 
             delay(4000);
             showSerialData();
             mySerial.println("AT+HTTPACTION=0");//submit the request 
             delay(5000);//the delay is very important, the delay time is base on the return from the website, 
             //if the return datas are very large, the time required longer.
//             showSerialData();
            
             
             String getResponse="";
             while(mySerial.available()!=0){
              char in = mySerial.read();
                 getResponse=getResponse+String(in);
             }
             int startIndexOfGet=getResponse.indexOf("HTTPACTION:")+11;
            int endIndexOfGet=startIndexOfGet+8;
             getResponse = getResponse.substring(startIndexOfGet, endIndexOfGet);
//              //Serial.println("GET RESPONSE IS: "+getResponse);
              lcd.clear();
             lcd.setCursor (0,0); // go to start of 3rd line
             lcd.print("GET : "+getResponse);

                          
             mySerial.println("AT+HTTPREAD");// read the data from the website you access
             delay(4000);
             parseGetRequests();

            //once new API will come
             Serial.println("Before Panic status is "+devicePanicStatus);
             lcd.setCursor (0,1); // go to start of 2nd line
            lcd.print("Lite:"+deviceLightStatus+" Fan:"+deviceFanStatus+" PS:" +devicePanicStatus);
             mySerial.println("AT+HTTPPARA=\"URL\",\"http://139.59.249.126:4242/setstatuses?temperature="+stringTemp+"&humidity="+stringHum+"&door="+stringDoor+"&smoke="+stringSmoke+"&fan="+deviceFanStatus+"&light="+deviceLightStatus+"&panic="+devicePanicStatus+"\""); 
              //the second parameter is the website you want to access
              panicState = 0; // resetting the panic button back to low, as alert is sent.
             delay(1000);
             showSerialData();
             mySerial.println("AT+HTTPACTION=0");//submit the request 
             delay(5000);//the delay is very important, the delay time is base on the return from the website, 
//             showSerialData();


             String setResponse="";
             while(mySerial.available()!=0){
              char in = mySerial.read();
                 setResponse=setResponse+String(in);
             }
             int startIndexOfSet=setResponse.indexOf("HTTPACTION:")+11;
            int endIndexOfSet=startIndexOfSet+8;
             setResponse = setResponse.substring(startIndexOfSet, endIndexOfSet);
              Serial.println("SET RESPONSE IS: "+setResponse);
             lcd.setCursor (0,2); // go to start of 3rd line
             lcd.print("SET : "+setResponse);



            lcd.setCursor (0,3); // go to start of 3rd line
             lcd.print("T:"+stringTemp+"H:"+stringHum +"D:"+stringDoor+ "S:"+stringSmoke);


             mySerial.println("AT+HTTPREAD");// read the data from the website you access
             delay(1000);
//             showSerialData();
             Serial.println("after Panic status is "+devicePanicStatus);
             

}

void showSerialData()
{
 while(mySerial.available()!=0)
  Serial.write(mySerial.read());
 
}

void powerOn(){
 if(sendCommand("AT","OK",500,1) !=1){
 Serial.println("Powering On...");
 pinMode(7, OUTPUT); 
 digitalWrite(7,LOW);
 delay(1000);
 digitalWrite(7,HIGH);
 delay(2000);
 digitalWrite(7,LOW);
 delay(10000);
 //delay(15500);
 if(sendCommand("AT+CREG?","+CREG: 0,1",500,10,10)==1){
 Serial.println("REGISTERED");
 Serial.print(".");
 }
 power = true;
 }
 else{
 Serial.println("ALREADY ON");
 power = true;
 }
}
void powerOff(){
 delay(1000);
 pinMode(7, OUTPUT); 
 digitalWrite(7,LOW);
 delay(1000);
 digitalWrite(7,HIGH);
 delay(2500);
 digitalWrite(7,LOW);
 power = false;
}
int sendCommand(char* command, char* resp, int delayTime, int reps, int numData){
 int returnVal;
 //delay(100);
 for(int i=0;i<reps;i++){
 if (i > 0) delay(500);
 mySerial.flush();
 delay(100);
 mySerial.println(command);
 delay(100);
 Serial.print(command);
 Serial.print(": ");
 //delay(10);
 delay(delayTime);

 long previousMillis = millis();
 //unsigned long currentMillis;
 while(mySerial.available()) {
 readline();
 //Serial.println("AFTER READLINE");
 if (strncmp(buffer, resp,numData) == 0) {
 Serial.println(buffer);
 return 1;
 }
 }
 Serial.print("FAILED");
 Serial.println(buffer);
 }
 return 0;
}
void readline() {
 memset(buffer,0,sizeof(buffer));
 char c;
 int i =0;
 buffidx = 0; // start at begninning
 //Serial.println("BEFORE READLINE");
 long previousMillis = millis();
 while (1) {
 unsigned long currentMillis = millis();
 if(currentMillis - previousMillis > 20000) {
 Serial.println("TIMEOUT");
 return;
 }
 delay(2);
 c=mySerial.read();
 //Serial.print(buffidx);
 if (c == -1)
 continue;
 if (c == '\n')
 continue;
 if ((buffidx == BUFFSIZ-1) || (c == '\r')) {
 buffer[buffidx] = 0;
 return;
 }
 buffer[buffidx++]= c;
 delay(2);
 }
}
void setupCommands(){
 sendCommand("AT&F","OK");
 sendCommand("ATE0","OK");
 sendCommand("AT+CLIP=1","OK");
 sendCommand("AT+CMEE=0","OK");
 sendCommand("AT+CIPSHUT","SHUT");
}

