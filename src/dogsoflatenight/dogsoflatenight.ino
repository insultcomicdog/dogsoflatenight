
#include <SPI.h>
#include <WiFly.h>

#include "Credentials.h"

#include <TextFinder.h>

char twitterSearchTerms[] = "insultcomicdog%20OR%20formetopoopon";
int since_id = 0;
int totalnumsearchresults = 0;

char totalnumsearchresults_str[200];
char from_user[20];
char profile_image_url[200];
char tweet[140];
char source[200];
char created_at[200];
char since_id_str[200];


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

   // connect to Twitter:
  delay(3000);
  
}

void loop() {
  
  if (client.connect()) {
  
    Serial.println("making HTTP request...");
//    client.println("GET /labs/dogsoflatenight/twitterpoll.php?since_id=0&track=insultcomicdog%20OR%20formetopoopon HTTP/1.1");
//    client.println("HOST: www.suniljohn.com");
//    client.println();

//      Serial.print("GET /labs/dogsoflatenight/twitterpoll.php?since_id=");
//      Serial.print(since_id);
//      Serial.print("&track=");
//      Serial.print(twitterSearchTerms);
//      Serial.println(" HTTP/1.1");
//      Serial.println("HOST: www.suniljohn.com");
//      Serial.println();

    client.print("GET /labs/dogsoflatenight/twitterpoll.php?since_id=");
    client.print(since_id);
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
      if((finder.getString("<since_id>","</since_id>",since_id_str,200)!=0));
      
     
    }
  }
}
  
  delay(1);

  client.stop();
 
  Serial.println();
  Serial.println();
  Serial.println("parsing twitter data...");
  
  //this doesn't work
  //totalnumsearchresults = int(totalnumsearchresults_str);
  //since_id = int(since_id_str);
  
  
  //totalnumsearchresults = stringToNumber(totalnumsearchresults_str); //this works
  //since_id = stringToNumber(since_id_str); //this doesn't work...189148246403325952 ---> 8192
  
  totalnumsearchresults = getInt(totalnumsearchresults_str); //this works
  since_id = getInt(since_id_str); //this kind of works but doesn't convert full int...189148246403325952 ---> 1891
  
  
  Serial.println(totalnumsearchresults_str);
  Serial.println(from_user);
  Serial.println(profile_image_url);
  Serial.println(tweet);
  Serial.println(source);
  Serial.println(created_at);
  Serial.println(since_id_str);
  
  Serial.println(totalnumsearchresults);
  Serial.println(since_id);

  Serial.println();
  Serial.println("delay...");
  delay (60000); 
  // don't make this less than 30000 (30 secs), because you can't connect to the twitter servers faster (you'll be banned)
  // off course it would be better to use the "Blink without delay", but I leave that to you.
}


int stringToNumber(String thisString) {
  int i, value = 0, length;
  length = thisString.length();
  for(i=0; i<length; i++) {
    value = (10*value) + thisString.charAt(i)-(int) '0';
  }
  return value;
}

int getInt(String text)
{
  char temp[6];
  text.toCharArray(temp, 5);
  int x = atoi(temp);
  return x;
} 

