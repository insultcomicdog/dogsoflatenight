

#include <Wire.h>

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

#include <ServoTimer2.h>  // the servo library

//servo vars
int activeServo= 0;

ServoTimer2 myservo1;
ServoTimer2 myservo2;
ServoTimer2 myservo3;
ServoTimer2 myservo4;

int servoPin1 = 6;//9; 
int servoPin2 = 7;//10;
int servoPin3 = 8;//10;
int servoPin4 = 9;//10;
#define degreesToUS( _degrees) (_degrees * 6 + 900) // macro to convert degrees to microseconds
int pos = 50;    // variable to store the servo position 

int inPos = 1400;
int outPos = 500;
int servoSpd = 30;

//servo vars

boolean motionIsOn = false;
boolean speakJetIsOn = false;

void setup()
{
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);           // start serial for output
  Serial.println("slave setup");
  
    
  initWAVShield();
  initMotionSensor();
  initServo();
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
//  pinMode(6, OUTPUT);
//  pinMode(7, OUTPUT);
//  pinMode(8, OUTPUT);
//  pinMode(5, OUTPUT);
  
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
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

void initMotionSensor()
{
  Serial.println("initMotionSensor");
  pinMode(alarmPin, INPUT);
}

void initServo()
{
  Serial.println("initServo");
  
  pinMode(servoPin1, OUTPUT);
  pinMode(servoPin2, OUTPUT);
  pinMode(servoPin3, OUTPUT);
  pinMode(servoPin4, OUTPUT);

  // set up servo pin
  myservo1.attach(servoPin1);
  myservo2.attach(servoPin2);
  myservo3.attach(servoPin3);
  myservo4.attach(servoPin4);
  
  stepper1();
  stepper2();
  stepper3();
  stepper4();
}

void loop()
{
  if(motionIsOn==true){
      alarmValue = analogRead(alarmPin);
          
      //Serial.println(alarmValue);
    
      if(alarmValue < 100) {
         Serial.println("motion detected");
         playRandomSound();
      } 
      
      delay(timer);
  }
  
  if(speakJetIsOn==true){
     animateMouth();
  }
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
  // call our helper to find and play this name
  playfile(name);
  while (wave.isplaying) {
  // do nothing while its playing
      //animateMouth();
      animateMouthWAV();
  }
  // now its done playing
 
}

void animateMouthWAV(){
  Serial.println("animateMouthWAV");

  char i;
  uint8_t volume;
  int v2;
  
  volume = 0;
   for (i=0; i<8; i++) {
     v2 = analogRead(1) - 512;
     if (v2 < 0) 
        v2 *= -1;
     if (v2 > volume)
       volume = v2;
     delay(5);
   }
   
   if (volume > 200) {
     for(pos = 1400; pos < 2300; pos += servoSpd)  // goes from 0 degrees to 180 degrees 
      {                                  // in steps of 1 degree 
        myservo1.write(pos);              // tell servo to go to position in variable 'pos' 
        delay(15);                       // waits 15ms for the servo to reach the position 
      } 
   } else {
      for(pos = 1700; pos>=2300; pos -=servoSpd)     // goes from 180 degrees to 0 degrees 
      {                                
        myservo1.write(pos);              // tell servo to go to position in variable 'pos' 
        delay(15);                       // waits 15ms for the servo to reach the position 
      } 
   }
   
//     Serial.println("volume");
//     Serial.println(volume);
}

void animateMouth(){
  //stepper1();
  
  //stepper2();

  if(activeServo==0){
     stepper2();
  } else if(activeServo==1){
     stepper3();
  } else if(activeServo==2){
     stepper4();
  } 
}

int stepper1(){
  Serial.println("stepper1");

  for(pos = 1400; pos < 2300; pos += servoSpd) // goes from 0 degrees to 180 degrees
  { // in steps of 1 degree
    myservo1.write(pos); // tell servo to go to position in variable 'pos'
    delay(15); // waits 15ms for the servo to reach the position
  }
  for(pos = 2300; pos>=1400; pos -=servoSpd) // goes from 180 degrees to 0 degrees
  {
    myservo1.write(pos); // tell servo to go to position in variable 'pos'
    delay(15); // waits 15ms for the servo to reach the position
  }
}

int stepper2(){
  Serial.println("stepper2");

  for(pos = inPos; pos >=outPos; pos -= servoSpd)  // goes from 0 degrees to 180 degrees 
  {                                  // in steps of 1 degree 
    myservo2.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(15);                       // waits 15ms for the servo to reach the position 
  } 
  for(pos = outPos; pos<inPos; pos +=servoSpd)     // goes from 180 degrees to 0 degrees 
  {                                
    myservo2.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(15);                       // waits 15ms for the servo to reach the position 
  } 
}

int stepper3(){
  Serial.println("stepper3");

  for(pos = inPos; pos >=outPos; pos -= servoSpd)  // goes from 0 degrees to 180 degrees 
  {                                  // in steps of 1 degree 
    myservo3.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(15);                       // waits 15ms for the servo to reach the position 
  } 
  for(pos = outPos; pos<inPos; pos +=servoSpd)     // goes from 180 degrees to 0 degrees 
  {                                
    myservo3.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(15);                       // waits 15ms for the servo to reach the position 
  } 
}

int stepper4(){
  Serial.println("stepper4");

  for(pos = inPos; pos >=outPos; pos -= servoSpd)  // goes from 0 degrees to 180 degrees 
  {                                  // in steps of 1 degree 
    myservo4.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(15);                       // waits 15ms for the servo to reach the position 
  } 
  for(pos = outPos; pos<inPos; pos +=servoSpd)     // goes from 180 degrees to 0 degrees 
  {                                
    myservo4.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(15);                       // waits 15ms for the servo to reach the position 
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

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
  while(1 < Wire.available()) // loop through all but the last
  {
    char c = Wire.read(); // receive byte as a character
    //Serial.print(c);         // print the character
  }
  int x = Wire.read();    // receive byte as an integer
  //Serial.println(x);         // print the integer
  
  if(x==0){
    motionIsOn=false;
    speakJetIsOn=false;
    
    if (wave.isplaying) {// already playing something, so stop it!
        wave.stop(); // stop it
    }
    
  } else if (x==1) {
    motionIsOn=true;
  } else if (x==2 && speakJetIsOn == false) {
    
    if (wave.isplaying) {// already playing something, so stop it!
        wave.stop(); // stop it
    }
    
    activeServo = random(3);
    Serial.println("activeServo");
    Serial.println(activeServo);
    speakJetIsOn=true;
  }
  
  //Serial.println(x);

}
