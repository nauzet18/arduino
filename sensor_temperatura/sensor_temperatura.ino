#include "DHT.h"
#define DHTPIN 8
#define DHTTYPE DHT22   // DHT 22  (AM2302)
// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHTPIN, DHTTYPE);


#include <SD.h>
const int chipSelect = 53;
void setupSD()
{
  pinMode(53, OUTPUT);
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) 
  {
    Serial.print("SD begin: ERROR");
    return;
  }
}

void writeDatas( String data )
{
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile) 
  {
    Serial.print("SD escribir:");
    Serial.println( data );
    
    dataFile.println(data);
    dataFile.close();
  }
  else
  {
    Serial.println("No se escribio dato en la SD");
  }
}

#include <EEPROM.h>
int Tc_100 = 0;
int count = 0;
boolean bAlternarDisplay = true;

int DHpin = 8; //humedad
byte dat [5]; //datos de humedad

int digit1 = 22; //PWM Display pin 1
int digit2 = 24; //PWM Display pin 2
int digit3 = 26; //PWM Display pin 6
int digit4 = 28; //28; //53 1º //PWM Display pin 8

//Pin mapping from Arduino to the ATmega DIP28 if you need it
//http://www.arduino.cc/en/Hacking/PinMapping
int segA = 31;//pin 11 LCD; //Display pin 14
int segB = 35;//pin 7  LCD; //Display pin 16
int segC = 43;//pin 4  LCD; //Display pin 13
int segD = 39;//pin 2  LCD; //Display pin 3
int segE = 37;//pin 1  LCD; //Display pin 5
int segF = 33;//pin 10 LCD; //Display pin 11
int segG = 45;//pin 5  LCD; //Display pin 15

void setup(void) {
  // initialize inputs/outputs
  // start serial port
  Serial.begin(9600);
  
  pinMode (DHpin, OUTPUT); //Humedad y temp
  
  
  pinMode(segA, OUTPUT);
  pinMode(segB, OUTPUT);
  pinMode(segC, OUTPUT);
  pinMode(segD, OUTPUT);
  pinMode(segE, OUTPUT);
  pinMode(segF, OUTPUT);
  pinMode(segG, OUTPUT);

  pinMode(digit1, OUTPUT);
  pinMode(digit2, OUTPUT);
  pinMode(digit3, OUTPUT);
  pinMode(digit4, OUTPUT);

  //Configuración de la SD
  setupSD();
  //Configuracion de DHT22
  dht.begin();
}

float fHumedad =0.0;
float fTemperatura =0.0;

void loop(void) {

  //For conversion of raw data to C
  int HighByte, LowByte, TReading, SignBit, Whole, Fract;
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];  
if ( count > 200 )
{
  count = 0;
 
  //-------- humedad y temperatura DHT22
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  fHumedad = dht.readHumidity();
  // Read temperature as Celsius
  fTemperatura = dht.readTemperature();
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(fHumedad) || isnan(fTemperatura ) ) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

   Serial.println( "fHumedad:" );
   Serial.println( fHumedad );
   
   Serial.println( "fTemperatura :" );
   Serial.println( fTemperatura );
  //------- Fin humedad
  //Escrimos los datos de temperatura y de humedad en la SD
  String data = "";
  data = "T:"+(String)fTemperatura +"|HT:"+(String)fHumedad;
  
  writeDatas( data );
  
  //Cada vez que sacamos los datos, señalizamos para alternar el dato a mostrar por el displey  
  bAlternarDisplay = !bAlternarDisplay;
}
else
{
  count= count+1;
  
//Alterno entre la impresión de la temperatura y la humedad  
  if ( bAlternarDisplay )
  {
    int iTemperaturaPrint = fTemperatura*100 ;
    displayNumber( iTemperaturaPrint );
  }
  else
  {
    int iHumedadPrint = fHumedad*100 ;
    displayNumber( iHumedadPrint );  
  }

  
}
  
}


//Given a number, we display 10:22
//After running through the 4 numbers, the display is left turned off

//Display brightness
//Each digit is on for a certain amount of microseconds
//Then it is off until we have reached a total of 20ms for the function call
//Let's assume each digit is on for 1000us
//Each digit is on for 1ms, there are 4 digits, so the display is off for 16ms.
//That's a ratio of 1ms to 16ms or 6.25% on time (PWM).
//Let's define a variable called brightness that varies from:
//5000 blindingly bright (15.7mA current draw per digit)
//2000 shockingly bright (11.4mA current draw per digit)
//1000 pretty bright (5.9mA)
//500 normal (3mA)
//200 dim but readable (1.4mA)
//50 dim but readable (0.56mA)
//5 dim but readable (0.31mA)
//1 dim but readable in dark (0.28mA)

void displayNumber(int toDisplay) {
#define DISPLAY_BRIGHTNESS  100

#define DIGIT_ON  HIGH
#define DIGIT_OFF  LOW

  long beginTime = millis();

  for(int digit = 4 ; digit > 0 ; digit--) {

    //Turn on a digit for a short amount of time
    switch(digit) {
    case 1:
      digitalWrite(digit1, DIGIT_ON);
      break;
    case 2:
      digitalWrite(digit2, DIGIT_ON);
      break;
    case 3:
      digitalWrite(digit3, DIGIT_ON);
      break;
    case 4:
      digitalWrite(digit4, DIGIT_ON);
      break;
    }

    //Turn on the right segments for this digit
    lightNumber(toDisplay % 10);
    
    toDisplay /= 10;

    delayMicroseconds(DISPLAY_BRIGHTNESS); 
    //Display digit for fraction of a second (1us to 5000us, 500 is pretty good)

    //Turn off all segments
    lightNumber(10); 

    //Turn off all digits
    digitalWrite(digit1, DIGIT_OFF);
    digitalWrite(digit2, DIGIT_OFF);
    digitalWrite(digit3, DIGIT_OFF);
    digitalWrite(digit4, DIGIT_OFF);

  }

  while( (millis() - beginTime) < 10) ; 
  //Wait for 20ms to pass before we paint the display again
}

//Given a number, turns on those segments
//If number == 10, then turn off number
void lightNumber(int numberToDisplay) {

#define SEGMENT_ON  LOW
#define SEGMENT_OFF HIGH

  switch (numberToDisplay){

  case 0:
    digitalWrite(segA, SEGMENT_ON);
    digitalWrite(segB, SEGMENT_ON);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_ON);
    digitalWrite(segE, SEGMENT_ON);
    digitalWrite(segF, SEGMENT_ON);
    digitalWrite(segG, SEGMENT_OFF);
    break;

  case 1:
    digitalWrite(segA, SEGMENT_OFF);
    digitalWrite(segB, SEGMENT_ON);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_OFF);
    digitalWrite(segE, SEGMENT_OFF);
    digitalWrite(segF, SEGMENT_OFF);
    digitalWrite(segG, SEGMENT_OFF);
    break;

  case 2:
    digitalWrite(segA, SEGMENT_ON);
    digitalWrite(segB, SEGMENT_ON);
    digitalWrite(segC, SEGMENT_OFF);
    digitalWrite(segD, SEGMENT_ON);
    digitalWrite(segE, SEGMENT_ON);
    digitalWrite(segF, SEGMENT_OFF);
    digitalWrite(segG, SEGMENT_ON);
    break;

  case 3:
    digitalWrite(segA, SEGMENT_ON);
    digitalWrite(segB, SEGMENT_ON);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_ON);
    digitalWrite(segE, SEGMENT_OFF);
    digitalWrite(segF, SEGMENT_OFF);
    digitalWrite(segG, SEGMENT_ON);
    break;

  case 4:
    digitalWrite(segA, SEGMENT_OFF);
    digitalWrite(segB, SEGMENT_ON);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_OFF);
    digitalWrite(segE, SEGMENT_OFF);
    digitalWrite(segF, SEGMENT_ON);
    digitalWrite(segG, SEGMENT_ON);
    break;

  case 5:
    digitalWrite(segA, SEGMENT_ON);
    digitalWrite(segB, SEGMENT_OFF);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_ON);
    digitalWrite(segE, SEGMENT_OFF);
    digitalWrite(segF, SEGMENT_ON);
    digitalWrite(segG, SEGMENT_ON);
    break;

  case 6:
    digitalWrite(segA, SEGMENT_ON);
    digitalWrite(segB, SEGMENT_OFF);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_ON);
    digitalWrite(segE, SEGMENT_ON);
    digitalWrite(segF, SEGMENT_ON);
    digitalWrite(segG, SEGMENT_ON);
    break;

  case 7:
    digitalWrite(segA, SEGMENT_ON);
    digitalWrite(segB, SEGMENT_ON);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_OFF);
    digitalWrite(segE, SEGMENT_OFF);
    digitalWrite(segF, SEGMENT_OFF);
    digitalWrite(segG, SEGMENT_OFF);
    break;

  case 8:
    digitalWrite(segA, SEGMENT_ON);
    digitalWrite(segB, SEGMENT_ON);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_ON);
    digitalWrite(segE, SEGMENT_ON);
    digitalWrite(segF, SEGMENT_ON);
    digitalWrite(segG, SEGMENT_ON);
    break;

  case 9:
    digitalWrite(segA, SEGMENT_ON);
    digitalWrite(segB, SEGMENT_ON);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_ON);
    digitalWrite(segE, SEGMENT_OFF);
    digitalWrite(segF, SEGMENT_ON);
    digitalWrite(segG, SEGMENT_ON);
    break;

  case 10:
    digitalWrite(segA, SEGMENT_OFF);
    digitalWrite(segB, SEGMENT_OFF);
    digitalWrite(segC, SEGMENT_OFF);
    digitalWrite(segD, SEGMENT_OFF);
    digitalWrite(segE, SEGMENT_OFF);
    digitalWrite(segF, SEGMENT_OFF);
    digitalWrite(segG, SEGMENT_OFF);
    break;
  }
}

