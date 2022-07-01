#include <Arduino.h>
#include "HX711.h"
#include "Wire.h" //allows communication over i2c devices

HX711 scale;

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;
/////////////////////////////////////////////////////////////////////////////////////////////
const int pressureInput = A5; //select the analog input pin for the pressure transducer
const int pressureZero = 102.4; //analog reading of pressure transducer at 0psi
const int pressureMax = 921.6; //analog reading of pressure transducer at 100psi
const int pressuretransducermaxPSI = 100; //psi value of transducer being used
const int sensorreadDelay = 1; //constant integer to set the sensor read delay in milliseconds

int runtime = 2000; // in milliseconds

long reading = 0; // Initial reading
long int t1 = 0;
long int t2 = 0;
int solenoidPin=12; //solenoid pin

float pressureValue = 0; //variable to store the value coming from the pressure transducer
float Load_N = 0; // Start value of Load cell

byte isValveOpen = 0; // On and Off state of valve
byte ints_counter = 0;


 

void setup() {

  Serial.begin(9600); // Baud Rate
  Serial.println("HX711 Demo");
  Serial.println("Initializing the scale");

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

//  Serial.println("Before setting up the scale:");
//  Serial.print("read: \t\t");
//  Serial.println(scale.read());      // print a raw reading from the ADC

//  Serial.print("read average: \t\t");
//  Serial.println(scale.read_average(20));   // print the average of 20 readings from the ADC

//  Serial.print("get value: \t\t");
//  Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight (not set yet)

//  Serial.print("get units: \t\t");
//  Serial.println(scale.get_units(5), 1);  // print the average of 5 readings from the ADC minus tare weight (not set) divided
            // by the SCALE parameter (not set yet)
            
  scale.set_scale(107.64); // Value found by calibrating the scale (DONT CHANGE!)
  scale.tare();            // reset the scale to 0

//  Serial.println("After setting up the scale:");

//  Serial.print("read: \t\t");
//  Serial.println(scale.read());                   // print a raw reading from the ADC

//  Serial.print("read average: \t\t");
//  Serial.println(scale.read_average(20));       // print the average of 20 readings from the ADC

//  Serial.print("get value: \t\t");
//  Serial.println(scale.get_value(5));          // print the average of 5 readings from the ADC minus the tare weight, set with tare()

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(1), 1);        // print the average of 5 readings from the ADC minus tare weight, divided
            // by the SCALE parameter set with set_scale

  Serial.println("Readings:");
  
  pinMode(solenoidPin, OUTPUT); //Sets the pin as an output
  digitalWrite(solenoidPin, HIGH); //Switch Solenoid OFF
  Serial.print(-.1); // To freeze the lower limit of Serial Plotter (Set Window Values)
  Serial.print(.1); // To freeze the upper limit of Serial Plotter (Set Window Values)
  Serial.print(" "); 

  delay(5000); // Initial 5 second delay before valve testing commences


// Interrupt code - Allows Arduino to pause readings, open valve and resume readings as fast as possible without multithreading
cli();//stop interrupts
//set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 65000;// = (16*10^6) / (50*1024) - 1 (must be <65536) // Main counter for time between Solenoid Valve on and Off
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

sei();//allow interrupts



/////////////////////////////////////////////////////////////////////////////////////////////
  
}

void loop() {//loop routine runs over and over again forever

/////////////////////////////////////////////////////////////////////////////////////////////
 // Load_N = scale.get_units(1), 1))*0.0098066500286389); Load conversion in Newtowns
  Serial.print(scale.get_units(1), 1);
  Serial.print(" g ");
  pressureValue = analogRead(pressureInput); //reads value from input pin and assigns to variable
  pressureValue = (((pressureValue-pressureZero)*pressuretransducermaxPSI)/(pressureMax-pressureZero))*1.4467+.036; //conversion equation to convert analog reading to psi
  Serial.print(pressureValue, 1); //prints value from previous line to serial
  Serial.println(" psi"); //prints label to serial   
 //delay(sensorreadDelay); //delay in milliseconds between read values
/////////////////////////////////////////////////////////////////////////////////////////////                                                                                                                                                     
  
  delay(sensorreadDelay);
}


// Allows Arduino to count time to open and close the valves with interruptions
ISR(TIMER1_COMPA_vect){

  ints_counter++;

  if(ints_counter >1){
    ints_counter = 0;
    if (isValveOpen){
    digitalWrite(solenoidPin, HIGH); //Switch Solenoid OFF
    isValveOpen = 0;
    
   }else{
    digitalWrite(solenoidPin, LOW);
    isValveOpen = 1;
   }
  } 
}
