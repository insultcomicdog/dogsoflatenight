
#include <SPI.h>
#include <WiFly.h>

#include "Credentials.h"

#include <TextFinder.h>

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
char current_since_id_str[200] = "0";
char next_since_id_str[200];

int totalnumsearchresults = 0;

char totalnumsearchresults_str[200];
char from_user[20];
char profile_image_url[200];
char tweet[140];
char source[200];
char created_at[200];


byte server[] = { 69, 163, 223, 246 };

//Client client(server, 80);

WiFlyClient client("suniljohn.com", 80);

TextFinder  finder( client, 2 );

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
  delay(3000);
  
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
void initWaveSheild()
{
  
}

void loop() {
  
  if (client.connect()) {
  
    Serial.println("making HTTP request...");
    sjSerial.println("Searching Twitter for poop.");
//    client.println("GET /labs/dogsoflatenight/twitterpoll.php?since_id=0&track=insultcomicdog%20OR%20formetopoopon HTTP/1.1");
//    client.println("HOST: www.suniljohn.com");
//    client.println();

//    Serial.print("GET /labs/dogsoflatenight/twitterpoll.php?since_id=");
//    Serial.print(current_since_id_str);
//    Serial.print("&track=");
//    Serial.print(twitterSearchTerms);
//    Serial.println(" HTTP/1.1");
//    Serial.println("HOST: www.suniljohn.com");
//    Serial.println();

    client.print("GET /labs/dogsoflatenight/twitterpoll.php?since_id=");
    client.print(current_since_id_str); //this launches board into reset cycle
    client.print("&track=");
    client.print(twitterSearchTerms);
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
      if((finder.getString("<since_id>","</since_id>",next_since_id_str,200)!=0));
    }
  }
}
  
  delay(1);

  client.stop();
 
  Serial.println();
  Serial.println();
  Serial.println("parsing twitter data...");
  
  
  //totalnumsearchresults = stringToNumber(totalnumsearchresults_str); //this works
  //totalnumsearchresults = getInt(totalnumsearchresults_str); //this works
  totalnumsearchresults = atof(totalnumsearchresults_str);

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

      for (int i=0; i <= 200; i++){
        current_since_id_str[i] = next_since_id_str[i];
      }
      
  } else{
      //there are no results for my keywords so lets switch the motion sensor/WAV shield here  
  }
  
  Serial.println();
  Serial.println("delay...");
  delay (60000); 
  // don't make this less than 30000 (30 secs), because you can't connect to the twitter servers faster (you'll be banned)
  // off course it would be better to use the "Blink without delay", but I leave that to you.
}

void readTweet(String thisString)
{
  sjSerial.println(thisString); 
}


//int stringToNumber(String thisString) {
//  int i, value = 0, length;
//  length = thisString.length();
//  for(i=0; i<length; i++) {
//    value = (10*value) + thisString.charAt(i)-(int) '0';
//  }
//  return value;
//}
//
//int getInt(String text)
//{
//  char temp[6];
//  text.toCharArray(temp, 5);
//  int x = atoi(temp);
//  return x;
//} 

