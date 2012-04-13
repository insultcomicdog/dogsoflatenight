
#include <SPI.h>
#include <WiFly.h>

#include "Credentials.h"

#include <TextFinder.h>

//this is the twitter service I will be using
//http://www.suniljohn.com/labs/dogsoflatenight/twitterpoll.php?since_id=0&track=insultcomicdog%20OR%20formetopoopon

char twitterSearchTerms[] = "insultcomicdog OR formetopoopon";
char tweet[140];
char serverName[] = "http://www.suniljohn.com";  // twitter URL
int since_id = 0;

WiFlyClient client("suniljohn.com", 80);

void setup() {

  Serial.begin(9600);
  Serial.println("\n\r\n\rWiFly Shield Terminal Routine");
  
  WiFly.begin();
  
  if (!WiFly.join(ssid, passphrase)) {
    Serial.println("Association failed.");
    while (1) {
      // Hang on failure.
    }
  }
  
  Serial.println("Associated!");
  
   // connect to Twitter:
  delay(3000);
}


void loop() {
  // Terminal routine

  // Always display a response uninterrupted by typing
  // but note that this makes the terminal unresponsive
  // while a response is being received.
  while(SpiSerial.available() > 0) {
    Serial.write(SpiSerial.read());
  }
  
  if(Serial.available()) { // Outgoing data
    SpiSerial.write(Serial.read());
  }
  
   if (client.connect()) {
          TextFinder  finder( client,2 );
          Serial.println("making HTTP request...");
          // make HTTP GET request to server:
          client.print("GET /labs/dogsoflatenight/twitterpoll.php?since_id=");
       
          client.print(since_id);
          client.print("&track=");
          client.print(twitterSearchTerms);
         
          client.println("&count=1 HTTP/1.1");
          client.println("HOST: www.suniljohn.com");
          client.println();
          Serial.println("sent HTTP request...");
   
          while (client.connected()) {
              if (client.available()) {
                Serial.println("looking for tweet...");
                if((finder.find("<tweet>")&&(finder.getString("<text>","</text>",tweet,140)!=0)));
                Serial.println(tweet);
                break;
              }
          }
    }
    
    delay(1);
    client.stop();
    
    Serial.println("delay...");
    delay (60000); 
    // don't make this less than 30000 (30 secs), because you can't connect to the twitter servers faster (you'll be banned)
    // off course it would be better to use the "Blink without delay", but I leave that to you.

}
