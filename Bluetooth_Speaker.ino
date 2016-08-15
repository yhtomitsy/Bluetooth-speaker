#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>

#define jackIN 0 // jack connected to analog pin 0

Adafruit_NeoPixel strip = Adafruit_NeoPixel(12, 6, NEO_GRB + NEO_KHZ800); //declaring the neopixel strip
SoftwareSerial BLE(10,11); // RX,TX

///////////////////////////////////////////LED pattern variables////////////////////////////////////////////

volatile int threshold  = 50; // holds value of analog reading that will be recognised as beat
volatile boolean throbbed = false; // indicates when a throb has been detected
volatile int analogValue = 0;
uint8_t r = 0, g = 0, b = 0;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t patternIndex = 0;         // holds the pattern number
String command = "";

void setup() {
  Serial.begin(9600); // set baud rate
  BLE.begin(38400); // set BLE baud rate
  strip.begin(); // initialize neopixel strip
  strip.show(); // clear neopixel strip

  randomSeed(analogRead(5)); // set random seed
  
  cli();//stop interrupts
  //set timer1 interrupt at 10Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 6252;// = (16*10^6) / (10*256) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 bit for 256 prescaler
  TCCR1B |= (1 << CS12);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();//allow interrupts
}

// sampling the audio analog signal at 10Hz
ISR(TIMER1_COMPA_vect){//timer1 interrupt 10Hz 
  analogValue = analogRead(jackIN);
  if((analogValue>310&&analogValue<378)||(analogValue>624&&analogValue<694)){
    throbbed = true;
  }
}

void loop() {
  
  if(BLE.available()){
    command = BLE.readString();
    parseData();   
  }
  if(patternIndex == 1)VuMeter();
  else if(patternIndex == 2)colorThrob();
  else if(patternIndex == 3)colorVU();
  else if(patternIndex == 4)solid();
  delay(10);
}

/*
 * VU meter pattern
 */
void VuMeter(){
  VUFill12(0);
  //1st layer green
  if(analogValue>454&&analogValue<556) 
  {
    clearPixels(5, 6, 0, 12);
  }
  else if((analogValue>377&&analogValue<455)||(analogValue>555&&analogValue<625)) 
  {
    clearPixels(4, 7, 0, 12);
  }  
  //2nd layer yellow
  else if((analogValue>310&&analogValue<378)||(analogValue>624&&analogValue<694)) 
  {
    clearPixels(3, 8, 0, 12);
  }
  //3rd layer yellow
  else if((analogValue>243&&analogValue<311)||(analogValue>693&&analogValue<763))
  {
    clearPixels(2, 9, 0, 12);
  }
  //4th layer orange
  else if((analogValue>176&&analogValue<244)||(analogValue>762&&analogValue<832))
  {
    clearPixels(1, 10, 0, 12);
  }
  //5th layer red
  else if(analogValue<177||analogValue>831) 
  {
    clearPixels(0, 11, 0, 12);
  }
  strip.show(); // glow neopixels
}

/*
 * fill 12 pixel strip segments with VU meter colors
 */
void VUFill12(int index){
  uint32_t c = 0;
  for(uint8_t i = 0; i < 12; i++){
    if(i == 0 || i == 11)c = strip.Color(50,0,0); // red
    else if(i == 1 || i == 10)c = strip.Color(50,25,0); // orange
    else if(i == 2 || i == 3 || i == 8 || i == 9)c = strip.Color(50,50,0); // yellow
    else if(i > 3 && i < 8)c = strip.Color(0,50,0); //green
    strip.setPixelColor(i+index, c);
  }
}

/*
 * selective clearing for VU meter
 */
void clearPixels(int Start, int Stop, int index, int range){
  for(uint8_t i = index; i < range+index; i++){
     if(i >= Start && i <= Stop){}
     else strip.setPixelColor(i , 0);
  } 
}

/*
 * color throb pattern
 */
void colorThrob(){
  uint32_t c;
  int divider = 0;
  //1st layer green
  if(analogValue>454&&analogValue<556) 
  {
    divider=32;
  }
  else if((analogValue>377&&analogValue<455)||(analogValue>555&&analogValue<625)) 
  {
    divider=16;
  }  
  //2nd layer yellow
  else if((analogValue>310&&analogValue<378)||(analogValue>624&&analogValue<694)) 
  {
    divider=8;
  }
  //3rd layer yellow
  else if((analogValue>243&&analogValue<311)||(analogValue>693&&analogValue<763))
  {
    divider=4;
  }
  //4th layer orange
  else if((analogValue>176&&analogValue<244)||(analogValue>762&&analogValue<832))
  {
    divider=2;
  }
  //5th layer red
  else if(analogValue<177||analogValue>831) 
  {
    divider=1;
  }
  for(uint8_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(r/divider,g/divider,b/divider));
  }
  strip.show(); // glow neopixels
}

//colorVU
void colorVU(){
  for(uint8_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(r,g,b));
  }
  
  //1st layer green
  if(analogValue>454&&analogValue<556) 
  {
    clearPixels(0, 1, 0, 12);
  }
  else if((analogValue>377&&analogValue<455)||(analogValue>555&&analogValue<625)) 
  {
    clearPixels(0, 3, 0, 12);
  }  
  //2nd layer yellow
  else if((analogValue>310&&analogValue<378)||(analogValue>624&&analogValue<694)) 
  {
    clearPixels(0, 5, 0, 12);
  }
  //3rd layer yellow
  else if((analogValue>243&&analogValue<311)||(analogValue>693&&analogValue<763))
  {
    clearPixels(0, 7, 0, 12);
  }
  //4th layer orange
  else if((analogValue>176&&analogValue<244)||(analogValue>762&&analogValue<832))
  {
    clearPixels(0, 9, 0, 12);
  }
  //5th layer red
  else if(analogValue<177||analogValue>831) 
  {
    clearPixels(0, 11, 0, 12);
  }
  strip.show(); // glow neopixels
  
}

/*
 * Solid color pattern
 */

void solid() {
  for(uint8_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(r,g,b));
  }
  strip.show();
}


// convert from string to int
int stringtoInt(String buff){
  int r = 0;
  for (uint8_t i = 0; i < buff.length(); i++){
   r = (r*10) + (buff[i] - '0');
  }
  return r;
}

/*
 * get rgb values and pattern index from incoming string
 */
void parseData(){
  if(command[0] == 'C'){    
    // clear rgb values
    r=0;
    g=0;
    b=0;

    // get starting indices
    uint8_t startIndex = command.indexOf("C");
    uint8_t stopIndex = command.indexOf(",", startIndex);
    
    // get r value
    String comandBuffer  = command.substring(startIndex+1, stopIndex);
    r = stringtoInt(comandBuffer);
    
    // get indices
    startIndex = stopIndex;
    stopIndex = command.indexOf(",", startIndex+1);
    
    // get g value
    comandBuffer  = command.substring(startIndex+1, stopIndex);
    g = stringtoInt(comandBuffer);
    
    // get indices
    startIndex = stopIndex;
    stopIndex = command.indexOf("P", startIndex+1);
    
    // get b value
    comandBuffer  = command.substring(startIndex+1, stopIndex);
    b = stringtoInt(comandBuffer);

    patternIndex = stringtoInt(command.substring(command.indexOf("P")+1, command.indexOf("\n"))); // get the pattern index
  }
}
