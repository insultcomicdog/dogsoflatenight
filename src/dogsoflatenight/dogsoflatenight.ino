
#include <SPI.h>
#include <WiFly.h>
#include "Credentials.h"
#include <TextFinder.h>

#define SSWAV 49 //cable select pin for WAV shield
#define SSWIFLY 53 //cable select pin for WIFLY shield

//SpeakJet shield vars
//set up a SoftwareSerial port for Speakjet Shield
//rxPin: the pin on which to receive serial data from TTS256
//txPin: the pin on which to transmit serial data to the TTS256
#include <SoftwareSerial.h>
#define txPin 2
#define rxPin 3
SoftwareSerial sjSerial = SoftwareSerial(rxPin, txPin);
#define speakJetBusyPin 4
//SpeakJet shield vars

//WAV shield vars
#include <FatReader.h>
#include <SdReader.h>
#include <avr/pgmspace.h>
#include "WaveUtil.h"
#include "WaveHC.h"

SdReader card;    // This object holds the information for the card
FatVolume vol;    // This holds the information for the partition on the card
FatReader root;   // This holds the information for the filesystem on the card
FatReader f;      // This holds the information for the file we're play

WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time
//WAV shield vars

//motion sensor vars
int timer = 500;
int alarmPin = 0;
int alarmValue = 0;
//motion sensor vars

//servo vars
int servoPin = 22; 

int minPulse     =  600;  // minimum servo position
int maxPulse     =  2200; // maximum servo position
int turnRate     =  1800;  // servo turn rate increment (larger value, faster rate)
int refreshTime  =  20;   // time (ms) between pulses (50Hz)
int mouthchange = 6;  //checks to see if mouth position needs to be changed


/** The Arduino will calculate these values for you **/
int centerServo;         // center servo position
int pulseWidth;          // servo pulse width
long lastPulse   = 0;    // recorded time (ms) of the last pulse
//servo vars

//this is the service i'm calling
//http://www.suniljohn.com/labs/dogsoflatenight/twitterpoll.php?since_id=0&track=insultcomicdog%20OR%20formetopoopon%20OR%20%22for%20me%20to%20poop%20on%22%20-RT&rpp=3

char twitterSearchTerms[] = "insultcomicdog%20OR%20formetopoopon%20OR%20%22for%20me%20to%20poop%20on%22%20-RT";
char current_since_id_str[20] = "0";
char next_since_id_str[20];

int totalnumsearchresults = 0;
int maxSearchResults = 1;
int maxVOCount= 1;

char totalnumsearchresults_str[200];
char from_user[20];
char profile_image_url[200];
char tweet[140];
char source[200];
char created_at[200];

WiFlyClient client("suniljohn.com", 80);

TextFinder  finder( client, 2 );

boolean enableTwitterSearch = false;
boolean isPlayingSound = false;
int voCount = 0;


void setup() {
  
  Serial.begin(9600);
  
  setupWAVANDWIFLY();

  WiFly.begin();
  
  if (!WiFly.join(ssid, passphrase)) {
    Serial.println("Association failed.");
    while (1) {
     // Hang on failure.
    }
  }  

  Serial.println("Associated!");
  
  initSpeakJet();
  initWAVShield();
  initMotionSensor();
  initServo();

   // connect to Twitter:
  delay(10000);

}

void setupWAVANDWIFLY()
{
  pinMode(SSWAV, OUTPUT);
  pinMode(SSWIFLY, OUTPUT);

  digitalWrite(SSWAV, HIGH);
  digitalWrite(SSWIFLY, HIGH);
}

void initMotionSensor()
{
  pinMode(alarmPin, INPUT);
}

void initServo()
{
  // set up servo pin
  pinMode(servoPin, OUTPUT);  // Set servo pin 18 (analog 4) as an output pin
  centerServo = maxPulse - ((maxPulse - minPulse)/2);
  pulseWidth = centerServo;   // Give the servo a starting point (or it floats)
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

// this handy function will return the number of bytes currently free in RAM, great for debugging!   
int freeRam(void)
{
  extern int  __bss_end; 
  extern int  *__brkval; 
  int free_memory; 
  if((int)__brkval == 0) {
    free_memory = ((int)&free_memory) - ((int)&__bss_end); 
  }
  else {
    free_memory = ((int)&free_memory) - ((int)__brkval); 
  }
  return free_memory; 
} 

void sdErrorCheck(void)
{
  if (!card.errorCode()) return;
  putstring("\n\rSD I/O error: ");
  Serial.print(card.errorCode(), HEX);
  putstring(", ");
  Serial.println(card.errorData(), HEX);
  while(1);
}


//TODO: Initialize WAV shield when Twitter searches yeild 0 results
void initWAVShield()
{
  Serial.println("initWAVShield");
  putstring("Free RAM: ");       // This can help with debugging, running out of RAM is bad
  Serial.println(freeRam());      // if this is under 150 bytes it may spell trouble!
  
  // Set the output pins for the DAC control. This pins are defined in the library
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(5, OUTPUT);
 
  //  if (!card.init(true)) { //play with 4 MHz spi if 8MHz isn't working for you
  if (!card.init()) {         //play with 8 MHz spi (default faster!)  
    putstring_nl("Card init. failed!");  // Something went wrong, lets print out why
    sdErrorCheck();
    while(1);                            // then 'halt' - do nothing!
  }
  
  // enable optimize read - some cards may timeout. Disable if you're having problems
  card.partialBlockRead(true);
 
// Now we will look for a FAT partition!
  uint8_t part;
  for (part = 0; part < 5; part++) {     // we have up to 5 slots to look in
    if (vol.init(card, part)) 
      break;                             // we found one, lets bail
  }
  if (part == 5) {                       // if we ended up not finding one  :(
    putstring_nl("No valid FAT partition!");
    sdErrorCheck();      // Something went wrong, lets print out why
    while(1);                            // then 'halt' - do nothing!
  }
  
  // Lets tell the user about what we found
  putstring("Using partition ");
  Serial.print(part, DEC);
  putstring(", type is FAT");
  Serial.println(vol.fatType(),DEC);     // FAT16 or FAT32?
  
  // Try to open the root directory
  if (!root.openRoot(vol)) {
    putstring_nl("Can't open root dir!"); // Something went wrong,
    while(1);                             // then 'halt' - do nothing!
  }
  
  // Whew! We got past the tough parts.
  putstring_nl("Ready!");
}


//this enables the cable selection pin for WiFly shield
void enableWiFlyShield()
{
  digitalWrite(SSWAV, HIGH);
  digitalWrite(SSWIFLY, LOW);
}

//this enables the cable selection pin for WAV shield
void enableWAVShield()
{
  
  //digitalWrite(10, HIGH); //physical SS pin high before setting SPCR
  digitalWrite(SSWIFLY, HIGH);
  digitalWrite(SSWAV, LOW);
}

void loop() {

  while(digitalRead(speakJetBusyPin)){
    // code to move mouth which speakjet is busy
    Serial.println("****************speakJet is busy");
  } 
  
  if(enableTwitterSearch==true) {
        enableWiFlyShield();
        Serial.println("twitter search enabled");
        sjSerial.println("Serching Twitter for poop."); //this is not a typo...it actually sounds better read by the SpeakJet shield
        
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
            sjSerial.println("There is no poop on twitter. Switching to motion detection mode.");
            enableWAVShield();
        }
        
        Serial.println();
        Serial.println("delay...");
        delay (60000); 
        // don't make this less than 30000 (30 secs), because you can't connect to the twitter servers faster (you'll be banned)
  } else{
    
     alarmValue = analogRead(alarmPin);

      if(alarmValue < 100) {
         Serial.println("motion detected");
         voCount++;
         playRandomSound();
      } 
      
      delay(timer);
  }
}

void readTweet(String thisString)
{
  sjSerial.println(thisString); 
}


void playRandomSound(){
  
      int i;
      i = random(40);
      
      Serial.println (i);
      switch (i) {
        case 1:
          playcomplete("2poops.wav");
          break;
        case 2:
          playcomplete("8boobs.wav");
          break;
        case 3:
          playcomplete("30Secon~.wav");
          break;
        case 4:
          playcomplete("Arnold.wav");
          break;
        case 5:
          playcomplete("Benji.wav");
          break;
        case 6:
          playcomplete("Butt~.wav");
          break;
        case 7:
          playcomplete("Conten~.wav");
           break;
        case 8:
          playcomplete("Crowd~.wav");
          break;
        case 9:
          playcomplete("Dachshu~.wav");
          break;
        case 10:
          playcomplete("Dealwit~.wav");
          break;
        case 11:
          playcomplete("ForMeto~.wav");
          break; 
        case 12:
          playcomplete("Good2~.wav");
          break;
        case 13:
          playcomplete("Great2~.wav");
          break;
        case 14:
          playcomplete("Hair~.wav");
          break;
        case 15:
          playcomplete("Hockey.wav");
          break;
         case 16:
          playcomplete("Hottes~.wav");
          break;
        case 17:
          playcomplete("IchEin~.wav");
          break;
        case 18:
          playcomplete("IKeed~.wav");
          break;
        case 19:
          playcomplete("Kennel.wav");
          break;
         case 20:
          playcomplete("Crowd~.wav");
          break;
        case 21:
          playcomplete("Lassie.wav");
          break;
        case 22:
          playcomplete("Leg.wav");
          break;
        case 23:
          playcomplete("Lickin~.wav");
          break;
        case 24:
          playcomplete("LickMy~.wav");
          break;
        case 25:
          playcomplete("Magic.wav");
          break;
        case 26:
          playcomplete("Manage~wav");
          break;
        case 27:
          playcomplete("Neute~.wav");
          break;
        case 28:
          playcomplete("Nuts~.wav");
          break;
        case 29:
          playcomplete("PoopMe.wav");
          break;
        case 30:
          playcomplete("Nuts~.wav");
          break;
        case 31:
          playcomplete("PoopSL~.wav");
          break;     
        case 32:
          playcomplete("SanFran~.wav");
          break;
        case 33:
          playcomplete("Shithol~.wav");
          break;
        case 34:
          playcomplete("StarWa~.wav");
          break;
        case 35:
          playcomplete("STD.wav");
          break;
        case 36:
          playcomplete("Terrific.wav");
          break;
        case 37:
          playcomplete("Undera~.wav");
          break;
        case 38:
          playcomplete("Variety~.wav");
          break;
        case 39:
          playcomplete("WhoSong.wav");
          break;
        case 40:
          playcomplete("Youloo~.wav");
          break;
      }
}

// Plays a full file from beginning to end with no pause.
void playcomplete(char *name) {
  char i;
  uint8_t volume;
  int v2;
  
  // call our helper to find and play this name
  playfile(name);
  while (wave.isplaying) {
  // do nothing while its playing
 
       volume = 0;
          for (i=0; i<8; i++)
          {
            v2 = analogRead(1);
            delay(5);
          }
          
       //Serial.println (v2);
    
       if (v2 > 268)
            {
               //pulseWidth = 1800;
               pulseWidth = maxPulse;
               mouthchange = 1;
            }
               else
            {
              // pulseWidth = 800;
              pulseWidth = minPulse;
               mouthchange = 1;
            }
    
      
      digitalWrite(servoPin, HIGH);   // start the pulse
      delayMicroseconds(pulseWidth);  // pulse width
      digitalWrite(servoPin, LOW);    // stop the pulse
  }
  // now its done playing
  isPlayingSound=false;
  
   Serial.println("voCount");
   Serial.println(voCount);
    
  if(voCount==maxVOCount){
       voCount=0;
       enableWiFlyShield();
       enableTwitterSearch=true; 
  }
}

void playfile(char *name) {
  
  Serial.println(name);
  // see if the wave object is currently doing something
  if (wave.isplaying) {// already playing something, so stop it!
    wave.stop(); // stop it
  }
  // look in the root directory and open the file
  if (!f.open(root, name)) {
    putstring("Couldn't open file "); Serial.print(name); return;
  }
  // OK read the file and turn it into a wave object
  if (!wave.create(f)) {
    putstring_nl("Not a valid WAV"); return;
  }
  
  // ok time to play! start playback
  wave.play();
}
