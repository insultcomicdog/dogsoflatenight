

#include <Wire.h>

#include <SPI.h>
#include <WiFly.h>
#include "Credentials.h"
#include <TextFinder.h>

//SpeakJet shield vars
//set up a SoftwareSerial port for Speakjet Shield
//rxPin: the pin on which to receive serial data from TTS256
//txPin: the pin on which to transmit serial data to the TTS256
#include <SoftwareSerial.h>
#define txPin 2
#define rxPin 3
SoftwareSerial sjSerial = SoftwareSerial(rxPin, txPin);
#define busyPin 4
//SpeakJet shield vars

//Thermal Printer vars
#include <SoftwareSerial.h>
#include <Thermal.h>

int printer_RX_Pin = 24; //green wire
int printer_TX_Pin = 26; //yellow wire

Thermal printer(printer_RX_Pin, printer_TX_Pin, 19200);
//Thermal Printer vars

//this is the service i'm calling
//http://www.suniljohn.com/labs/dogsoflatenight/twitterpoll.php?since_id=0&track=insultcomicdog%20OR%20formetopoopon%20OR%20%22for%20me%20to%20poop%20on%22%20-RT&rpp=1

//char twitterSearchTerms[] = "%22insult%20comic%20dog%22%20OR%20formetopoopon%20OR%20%22for%20me%20to%20poop%20on%22%20-RT";
char twitterSearchTerms[] = "%20formetopoopon%20OR%20%22for%20me%20to%20poop%20on%22%20-RT"; //just using catch phrase now to make search more more granuluar and save on thermal paper!!
char current_since_id_str[20];
char next_since_id_str[20];

int totalnumsearchresults = 0;
int maxSearchResults = 1;

char totalnumsearchresults_str[200];
char from_user[20];
char profile_image_url[200];
char tweet[140];
char source[200];
char created_at[200];

WiFlyClient client("suniljohn.com", 80);

TextFinder  finder( client, 2 );

boolean enableTwitterSearch = true;
boolean isSearching = false;
boolean isPlayingSound = false;
boolean speakJetIsBusy = false;

boolean useEmic2 = true;
boolean emic2IsBusy = false;
int activeEmic2Voice= 0;
boolean isFirstTimeSearching=true;

byte slaveMode = 0;
int motionCounter= 0;
int responseTimeoutCounter= 0;
int maxResponseTimeOut = 5000;


unsigned long currentTime;
unsigned long loopTime;
int loopDuration = 0;


//status CONSTS
int STATUS_MOTION_DETECTION_MODE = 0;
int STATUS_WIFI_CONNECTED = 1;
int STATUS_WIFI_DISCONNECTED = 2;

int statusWifiBlue = 42; //blue
int statusWifiConnected = 40; //green
int statusWifiDisconnected = 44; //red

int statusMotionDectionMode = 43; //blue
int statusMotionDectionModeGreen = 41; //green
int statusMotionDectionModeRed = 45; //red


//Nixie Counter Vars
int tweets = 0;
char buffer[7];

int rockerSwitchPin = 32;
boolean installationIsActive = false;

void setup()
{
  Serial.begin(9600); 
  Serial2.begin(9600); // for nixie counter
  Serial3.begin(9600); //for emic 2

  resetTweetCounter();
  initStatusLEDs();
  setLEDStatus(STATUS_WIFI_DISCONNECTED);
  Wire.begin(); // join i2c bus (address optional for master)
  Serial.println("master setup");
  
  initNixieCounter();
  
  pinMode(rockerSwitchPin, INPUT);      // sets the digital pin 32 as input
  
  if (digitalRead(rockerSwitchPin) == HIGH) {
    installationIsActive = true;
  }
  
  WiFly.begin();
  
  if (!WiFly.join(ssid, passphrase)) {
    Serial.println("Association failed.");
    
    // Speak some text
    readString("The Dogs of Late Night is offline. Check your internet connection and please try again.");  // Send the desired string to convert to speech
    
    while (1) {
     // Hang on failure.
    }
  }  

  Serial.println("Associated!");
  setLEDStatus(STATUS_WIFI_CONNECTED);
  
  delay(10000);
  
  if(useEmic2) {
     initEmic2();
  } else{
     initSpeakJet();
  }
  
  currentTime = millis();
  loopTime = currentTime;
  current_since_id_str[0]='0'; 
}

void initNixieCounter()
{

  String num;
  int count;
  
  for (count=0;count<=6;count++){
    
    if(count==0){
        for (int i = 9;i>=0;i--){
          num = String(i) + "99999";
          updateNixieCounter(num);
        }
        //playCoinSFX();
    } else if(count==1){
        for (int i = 9;i>=0;i--){
          num = "0" + String(i) + "9999";
          updateNixieCounter(num);
        } 
        //playCoinSFX();
    } else if(count==2){
        for (int i = 9;i>=0;i--){
          num = "00" + String(i) + "999";
          updateNixieCounter(num);
        } 
         //playCoinSFX();
    } else if(count==3){
         for (int i = 9;i>=0;i--){
          num = "000" + String(i) + "99";
          updateNixieCounter(num);
         }
         //playCoinSFX();
    } else if(count==4){
         for (int i = 9;i>=0;i--){
          num = "0000" + String(i) + "9";
          updateNixieCounter(num);
         }
         //playCoinSFX();
    } else if(count==5){
         for (int i = 9;i>=0;i--){
          num = "00000" + String(i) + "";
          updateNixieCounter(num);
         }
         //play1UPSFX();
    }
  }
}

void updateNixieCounter(String num)
{
   //Serial.println(num);
   Serial2.println(num);
   delay(98);
}

void playCoinSFX(){
    byte sfxCode = 3;
    Wire.beginTransmission(4); // transmit to device #4
    Wire.write(sfxCode);              // sends one byte  
    Wire.endTransmission();    // stop transmitting 
}

void play1UPSFX(){
    byte sfxCode = 4;
    Wire.beginTransmission(4); // transmit to device #4
    Wire.write(sfxCode);              // sends one byte  
    Wire.endTransmission();    // stop transmitting 
}


void initStatusLEDs()
{
  pinMode(statusWifiBlue, OUTPUT);
  pinMode(statusWifiConnected, OUTPUT);     
  pinMode(statusWifiDisconnected, OUTPUT); 
  
  digitalWrite(statusWifiBlue, HIGH);
  digitalWrite(statusWifiConnected, HIGH);
  digitalWrite(statusWifiDisconnected, HIGH);
  
  pinMode(statusMotionDectionMode, OUTPUT);
  pinMode(statusMotionDectionModeGreen, OUTPUT);     
  pinMode(statusMotionDectionModeRed, OUTPUT); 
  
  digitalWrite(statusMotionDectionMode, HIGH);
  digitalWrite(statusMotionDectionModeGreen, HIGH);
  digitalWrite(statusMotionDectionModeRed, HIGH);
}

void setLEDStatus(int status)
{
    if(status==STATUS_MOTION_DETECTION_MODE){
      digitalWrite(statusMotionDectionMode, LOW);
    } else if(status==STATUS_WIFI_CONNECTED){
      digitalWrite(statusMotionDectionMode, HIGH);  
      digitalWrite(statusWifiConnected, LOW);
      digitalWrite(statusWifiDisconnected, HIGH);
    } else if(status==STATUS_WIFI_DISCONNECTED){
      digitalWrite(statusMotionDectionMode, HIGH);  
      digitalWrite(statusWifiConnected, HIGH);
      digitalWrite(statusWifiDisconnected, LOW);
    }
}

void initSpeakJet()
{
  Serial.println("initSpeakJet");
 // initialize the serial communications with the SpeakJet-TTS256
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  pinMode(busyPin, INPUT);
  sjSerial.begin(9600);// set the data rate for the SoftwareSerial port
  delay(1000); // wait a second for the Arduino resets to finish (speaks "ready")
  readString("The Dogs of Late Night is now online. All your poop are belong to us."); // send it to the SpeakJet
  
  //readString("Now this is the story all about how My life got flipped, turned upside down And I'd like to take a minute just sit right there I'll tell you how I became the prince of a town called Bel-air In west Philadelphia born and raised On the playground where I spent most of my days Chilling out, maxing, relaxing all cool And all shooting some b-ball outside of the school When a couple of guys, they were up to no good Started making trouble in my neighbourhood I got in one little fight and my mom got scared And said You're moving with your auntie and uncle in Bel-air I whistled for a cab and when it came near the License plate said fresh and had a dice in the mirror If anything I could say that this cab was rare But I thought nah, forget it, yo homes to Bel-air! I pulled up to a house about seven or eight And I yelled to the cabby Yo, homes smell you later! Looked at my kingdom I was finally there To sit on my throne as the prince of Bel-air");  // Send the desired string to convert to speech

}

void initEmic2(){
  
  Serial.println("initEmic2");

  /*
    When the Emic 2 powers on, it takes about 3 seconds for it to successfully
    intialize. It then sends a ":" character to indicate it's ready to accept
    commands. If the Emic 2 is already initialized, a CR will also cause it
    to send a ":"
  */
  Serial3.print('\n');             // Send a CR in case the system is already up
  while (Serial3.read() != ':');   // When the Emic 2 has initialized and is ready, it will send a single ':' character, so wait here until we receive it
  delay(10);                          // Short delay
  Serial3.flush();                 // Flush the receive buffer
  
//  Serial3.print('N');
//  Serial3.print(0);
//  Serial3.print('\n');

  // Speak some text
  readString("The Dogs of Late Night is now online. All your poop are belong to us.");  // Send the desired string to convert to speech
  
  //readString("Now this is the story all about how My life got flipped, turned upside down And I'd like to take a minute just sit right there I'll tell you how I became the prince of a town called Bel-air In west Philadelphia born and raised On the playground where I spent most of my days Chilling out, maxing, relaxing all cool And all shooting some b-ball outside of the school When a couple of guys, they were up to no good Started making trouble in my neighbourhood I got in one little fight and my mom got scared And said You're moving with your auntie and uncle in Bel-air I whistled for a cab and when it came near the License plate said fresh and had a dice in the mirror If anything I could say that this cab was rare But I thought nah, forget it, yo homes to Bel-air! I pulled up to a house about seven or eight And I yelled to the cabby Yo, homes smell you later! Looked at my kingdom I was finally there To sit on my throne as the prince of Bel-air");  // Send the desired string to convert to speech

}

void SJBusy(){
  delay(20); // wait 12ms minimum before checking SpeakJet busy pin
  while(digitalRead(busyPin)){
    speakJetIsBusy=true;
    slaveMode=2;
    Wire.beginTransmission(4); // transmit to device #4
    Wire.write(slaveMode);              // sends one byte  
    Wire.endTransmission();    // stop transmitting
    delay(250); // wait here while SpeakJet is busy (pin 4 is true)
    Serial.println("SPEAKJET IS BUSY");
  }
  
  
  if(speakJetIsBusy==true){
    speakJetIsBusy=false;
    slaveMode=0;
    Wire.beginTransmission(4); // transmit to device #4
    Wire.write(slaveMode);              // sends one byte  
    Wire.endTransmission();    // stop transmitting
    delay(250); // a bit more delay after busyPin goes false
  }
}

void checkEmic2Complete(){
  while (Serial3.read() != ':' && installationIsActive == true){
     Serial.println("emic2IsBusy");

     slaveMode=2;
     Wire.beginTransmission(4); // transmit to device #4
     Wire.write(slaveMode);              // sends one byte  
     Wire.endTransmission();    // stop transmitting
  } 
  
  Serial.println("done speech");
  
  emic2IsBusy = false;
  slaveMode=0;
  Wire.beginTransmission(4); // transmit to device #4
  Wire.write(slaveMode);              // sends one byte  
  Wire.endTransmission();    // stop transmitting
}

void stopApp(){
  slaveMode=0;
  motionCounter=0;
}

void checkRockerSwitch(){
  if (digitalRead(rockerSwitchPin) == HIGH) {
    if(!installationIsActive){
        Serial.println("LEDS ON");
        installationIsActive=true;
        Serial3.print('X');
        Serial3.print('\n');
        
        Serial3.print('S');
        Serial3.print("The Dogs of Late Night is active."); 
        Serial3.print('\n');
    }
  } else {
    if(installationIsActive){
        Serial.println("LEDS OFF");
        installationIsActive=false;
        stopApp();
        Serial3.print('X');
        Serial3.print('\n');
        
        Serial3.print('S');
        Serial3.print("The Dogs of Late Night is inactive."); 
        Serial3.print('\n');
    }
  }
}

void loop()
{
  
  checkRockerSwitch();
  
  if(useEmic2 && emic2IsBusy) {
     checkEmic2Complete();
  } else{
     SJBusy();
  }

  currentTime = millis();
  
  if(enableTwitterSearch==true) {
    
        if(isSearching==false){
          Serial.println("twitter search enabled");
          //readString("Serching Twitter for poop."); //this is not a typo...it actually sounds better read by the SpeakJet shield
          readString("Searching Twitter for poop.");

          isSearching=true;
          showTweetCounter();
        }

        if(currentTime >= (loopTime + 60000)){ 
        
        if (client.connect()) {
        
            Serial.println("making HTTP request...");
            
//            if(isFirstTimeSearching){
//              isFirstTimeSearching=false; 
//              loopDuration = 60000;
//             }
 
      //    client.println("GET /labs/dogsoflatenight/twitterpoll.php?since_id=0&track=insultcomicdog%20OR%20formetopoopon HTTP/1.1");
      //    client.println("HOST: www.suniljohn.com");
      //    client.println();
      
//          Serial.print("GET /labs/dogsoflatenight/twitterpoll.php?since_id=");
//          Serial.print(current_since_id_str);
//          Serial.print("&track=");
//          Serial.print(twitterSearchTerms);
//          Serial.print("&rpp=");
//          Serial.print(maxSearchResults); //looking for max 100 results
//          Serial.println(" HTTP/1.1");
//          Serial.println("HOST: www.suniljohn.com");
//          Serial.println();
          Serial.println("current_since_id_str");
          Serial.println(current_since_id_str);
      
          client.print("GET /labs/dogsoflatenight/twitterpoll.php?since_id=");
          client.print(current_since_id_str);
          client.print("&track=");
          client.print(twitterSearchTerms);
          client.print("&rpp=");
          client.print(maxSearchResults); //looking for max 100 results
          client.println(" HTTP/1.1");
          client.println("HOST: www.suniljohn.com");
          client.println();
 
          Serial.println("sent HTTP request...");
          
          while (client.connected()) {
            
           checkRockerSwitch();
 
           Serial.println("client is connected");
           responseTimeoutCounter++;
           Serial.println(responseTimeoutCounter);
           
           if(responseTimeoutCounter >= maxResponseTimeOut){ 
               Serial.println("we timed out");
               responseTimeoutCounter=0;
               totalnumsearchresults_str[0] = '0';
               delay(1);
               client.stop();
            }
                  
            if (client.available()) {
                  Serial.println("waiting for response");
                  
                  //char c = client.read();
                  //Serial.print(c);
                  if((finder.getString("<totalnumsearchresults>","</totalnumsearchresults>",totalnumsearchresults_str,200)!=0));
                  if((finder.getString("<from_user>","</from_user>",from_user,20)!=0));
                  if((finder.getString("<profile_image_url>","</profile_image_url>",profile_image_url, 200)!=0));
                  if((finder.getString("<text>","</text>",tweet,140)!=0));
                  if((finder.getString("<source>","</source>",source,200)!=0));  
                  if((finder.getString("<created_at>","</created_at>",created_at,200)!=0));
                  if((finder.getString("<since_id>","</since_id>",next_since_id_str,20)!=0));
            }
          }
      }
        
        delay(1);
      
        client.stop();
        
        totalnumsearchresults = atof(totalnumsearchresults_str);
        Serial.println();
        Serial.println();
        Serial.println("parsed twitter data...");
        Serial.println(totalnumsearchresults_str);
        Serial.println(from_user);
        Serial.println(profile_image_url);
        Serial.println(tweet);
        Serial.println(source);
        Serial.println(created_at);
        Serial.println(current_since_id_str);
        Serial.println(next_since_id_str);
        Serial.println(totalnumsearchresults);
         
        Serial.println("is a repeat");
        Serial.println(strcmp(current_since_id_str, next_since_id_str)); // if this is 0 the two strings are the same
        
        if(totalnumsearchresults!=0 && strcmp(current_since_id_str, next_since_id_str) != 0){ // sometimes twitter api is returning the same since id back as previous search
            Serial.println("we found a new tweet");
            
            if(installationIsActive){
                tweets++; //increment tweet count for Nixie counter
                showTweetCounter();
    
                printSRC(created_at); 
                delay(3000);
                printSRC(from_user); 
                delay(3000);
                printTweet(tweet); 
                delay(3000);
                advancePrinter();
                
                delay (5000);
                readString(tweet);
            }

            strncpy(current_since_id_str, next_since_id_str, 20);
            
        } else {
            //there are no new results for my keywords so lets switch the motion sensor/WAV shield here  
            Serial.println("there are no new results for my keywords");
            enableTwitterSearch=false;
            Serial.println("twitter search disabled");
            readString("There is no poop on twitter. Switching to motion detection mode.");
            setLEDStatus(STATUS_MOTION_DETECTION_MODE);
            hideTweetCounter();
            showNixieClock();
        }
        
          Serial.println();
          Serial.println("delay...");
        
          isSearching=false;
          loopTime = currentTime;
          responseTimeoutCounter=0;
          totalnumsearchresults=0;
          totalnumsearchresults_str[0] = '0';
        }
        
        //delay (60000); 
        // don't make this less than 30000 (30 secs), because you can't connect to the twitter servers faster (you'll be banned)
  } else{
    
    if(speakJetIsBusy==false){
      
        if(installationIsActive){
           slaveMode=1;
        }  
       
        motionCounter++;
        
        if(motionCounter==1500) {
          
           if(useEmic2){
             //randomizeEmic2Voice();
           }
        }
        
        if(motionCounter==3000) {
            slaveMode=0;
            motionCounter=0;
            enableTwitterSearch=true;
            loopTime = currentTime;
            setLEDStatus(STATUS_WIFI_CONNECTED);
        }
        
        Serial.println(motionCounter);
     }
  }
  
  Wire.beginTransmission(4); // transmit to device #4
  Wire.write(slaveMode);              // sends one byte  
  Wire.endTransmission();    // stop transmitting
}

void showTweetCounter()
{
  sprintf(buffer, "%06d", tweets); 
  Serial2.println(buffer);
}

void hideTweetCounter()
{
  Serial2.println("b"); //turn off Nixie counter
}

void showNixieClock()
{
  Serial2.println("t"); //switch into Time Display Mode
}

void resetTweetCounter()
{
    Serial2.println("r"); //zero pad the Nixie counter
}

void randomizeEmic2Voice()
{
      Serial.println("randomizeEmic2Voice");
      // print a random number from 0 to 8
      activeEmic2Voice  = random(9);
      Serial3.print('N');
      Serial3.print(activeEmic2Voice);
      Serial3.print('\n');
      
      Serial.println("activeEmic2Voice");
      Serial.println(activeEmic2Voice);
}

void readString(String thisString)
{ 
  
  if(installationIsActive){
      if(useEmic2) {
          Serial.println("readString");
          Serial.println(thisString);
          emic2IsBusy=true;
    
          Serial.println("starting speech");
          Serial3.print('S');
          Serial3.print(thisString); 
          Serial3.print('\n');
      } else{
           sjSerial.println(thisString); 
      }
  } 
}

void printTweet(String thisString)
{
  //printer.justify('L'); //sets text justification (right, left, center) accepts 'L', 'C', 'R'
   
  //printer.setSize('S'); // set type size, accepts 'S', 'M', 'L'
  printer.println(thisString); //print 
  
  //printer.feed(); //advance one line
  //printer.feed(); //advance one line
}

void printSRC(String thisString)
{
  printer.boldOn();
  printer.println(thisString);
  printer.boldOff(); 
  //printer.feed();
}

void advancePrinter()
{
  printer.feed(); //advance one line
  printer.feed(); //advance one line
}
