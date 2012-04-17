
#include <SPI.h>
#include <WiFly.h>

#include "Credentials.h"

#include <TextFinder.h>

#define SSWAV 33 //cable select pin for WAV shield
#define SSWIFLY 53 //cable select pin for WIFLY shield

//set up a SoftwareSerial port for Speakjet Shield
//rxPin: the pin on which to receive serial data from TTS256
//txPin: the pin on which to transmit serial data to the TTS256
#include <SoftwareSerial.h>
#define txPin 2
#define rxPin 3
SoftwareSerial sjSerial = SoftwareSerial(rxPin, txPin);


//this is the service i'm calling
//http://www.suniljohn.com/labs/dogsoflatenight/twitterpoll.php?since_id=0&track=insultcomicdog%20OR%20formetopoopon%20OR%20%22for%20me%20to%20poop%20on%22%20-RT

char twitterSearchTerms[] = "insultcomicdog%20OR%20formetopoopon%20OR%20%22for%20me%20to%20poop%20on%22%20-RT";
char current_since_id_str[20] = "0";
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

void setup() {
  
  Serial.begin(9600);

  WiFly.begin();
  
  if (!WiFly.join(ssid, passphrase)) {
    Serial.println("Association failed.");
    while (1) {
     // Hang on failure.
    }
  }  

  Serial.println("Associated!");
  
  initSpeakJet();

   // connect to Twitter:
  delay(10000);
  
}

void initSpeakJet()
{
 // initialize the serial communications with the SpeakJet-TTS256
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  sjSerial.begin(9600);// set the data rate for the SoftwareSerial port
  delay(1000); // wait a second for the Arduino resets to finish (speaks "ready")
  sjSerial.println("The Dogs of Late Night is now online. All your poop are belong to us."); // send it to the SpeakJet
}


//TODO: Initialize WAV shield when Twitter searches yeild 0 results
void initWAVShield()
{
  
}

//this enables the cable selection pin for WiFly shield
void enableWiFlyShield()
{
  digitalWrite(SSWIFLY, LOW);
  digitalWrite(SSWAV, HIGH);
}

//this enables the cable selection pin for WAV shield
void enableWAVShield()
{
  digitalWrite(SSWIFLY, HIGH);
  digitalWrite(SSWAV, LOW);
}

void loop() {
  
  if(enableTwitterSearch==true) {
        Serial.println("twitter search enabled");
        sjSerial.println("Searching Twitter for poop.");
        
        if (client.connect()) {
        
          Serial.println("making HTTP request...");
          
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
      
          
          client.print("GET /labs/dogsoflatenight/twitterpoll.php?since_id=");
          client.print(current_since_id_str); //this launches board into reset cycle
          client.print("&track=");
          client.print(twitterSearchTerms);
          client.print("&rpp=");
          client.print(maxSearchResults); //looking for max 100 results
          client.println(" HTTP/1.1");
          client.println("HOST: www.suniljohn.com");
          client.println();
          
          Serial.println("sent HTTP request...");
         
          
          while (client.connected()) {
          if (client.available()) {
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
        
        if(totalnumsearchresults!=0){
            readTweet(tweet);
      
            for (int i=0; i <= 20; i++){
              current_since_id_str[i] = next_since_id_str[i];
            }
            
        } else {
            //there are no new results for my keywords so lets switch the motion sensor/WAV shield here  
            Serial.println("there are no new results for my keywords");
            enableTwitterSearch=false;
            Serial.println("twitter search disabled");
            sjSerial.println("Switching to motion detection mode.");
            enableWAVShield();
        }
        
        Serial.println();
        Serial.println("delay...");
        delay (60000); 
        // don't make this less than 30000 (30 secs), because you can't connect to the twitter servers faster (you'll be banned)
  } else{
     //Serial.println("motion detection mode");
     
  }
}

void readTweet(String thisString)
{
  sjSerial.println(thisString); 
}
