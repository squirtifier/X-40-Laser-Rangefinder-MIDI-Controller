/*
  Laser MIDI Theremin created by James Cochrane/BD594 August 18, 2019 https://www.YouTube.com/BD594 
  
  LaserRangeFinder X-40 (703A) firmware was designed by iliasam  
  https://www.hackster.io/iliasam/making-a-cheap-laser-rangefinder-for-arduino-4dd849
  
  Hardware:  Arduino NANO V3
             X-40 (512, 701 or 703A) laser rangefinder module (requires iliasam firmware update)
             OLED Display 0.96" I2C IIC Serial 128X64 
             2 x 10K ohm resistors (required pull-up resistors for the cheap Chinese OLED displays)
             2 x 220 ohm resistors
             5 pin DIN MIDI connector
             If you can't build your own Tesla coil with a MIDI interuppter than the oneTesla-TS will work
      
  The X-40 (703A) module sends distance data in the following format "DIST;01574;AMP;0993;TEMP;1343;VOLT;082\r\n" 
  In this sketch we are only interested in extracting the distance data.

  The Arduino NANO only has one serial port and in this project we need two ports one for X-40 RX data and the other for 
  MIDI TX data.  In this sketch I am using SoftwareSerial for the lower speed MIDI TX.  
  
  The musical notes in this example will begin at F5(77) and end at G#6(92). This range can be modified in order to suit your requirements.
  When the Arduino receives the distance data it will translate it to the corresponding MIDI note value.
  Since I am modifying the Tesla's arc length with my faraday glove the longer the arc the lower the note.  The shorter the arc the higher the note.
  This is the same effect as a Theremin.  So in this experiment I all be using a distance of 10 mm to 240 mm.  If I get too close to the 
  Tesla's toroid I can short out the arc and damage the coil. 

  The great thing about using a laser it is not affectted by the Tesla's EMF or High Voltage arcs.  In the past I tried Ultrasound and TOF
  sensors however they were only effective at close ranges.  No matter how much shielding and insulation the Tesla coil would
  knock out these sensors.

  When zeroing the Laser rangefinder place a white banner at the point where you want the distance to equal 0 mm.  Push the calibrate switch 
  (SW4) and wait for the X-40 (703A) module to beep.  This process can take up to 10 seconds.  
  
 
*/

// Updated September 7, 2019

#include <OLED.h>
OLED Oled;

#include <SoftwareSerial.h>
SoftwareSerial MIDIOUT(10, 11); // Digital Pins assigned for MIDI RX, TX

int x = 0;
int16_t sam_cnt = 0; // SAMPLE Counter - used to see how fast the Arduino code is running
int16_t cur_distance = 0; // Distance reading from X-40 (703A) in mm's
int16_t new_midi_note = 0;
int16_t old_midi_note = 0;
String inputString = ""; // a String to hold incoming data from the laser rangefinder
bool stringComplete = false;  // whether the string is complete

void setup() {
  MIDIOUT.begin(31250);
  Serial.begin(256000);  // Receive baud rate for the X-40 (703A) module
  Oled.init();  //initialze OLED display
  Oled.clearDisplay();
  inputString.reserve(200); // reserve 200 bytes for the inputString:
  Oled.printString("   World's First",1,0);
  Oled.printString("   Tesla Theremin",1,1);
  Oled.printString("   James Cochrane",1,5); 
  Oled.printString("www.YouTube.com/BD594",1,6);
  delay (5000);
  Oled.clearDisplay();
//  Oled.printString("SAMPLES:",1,0); // Print SAMPLES: // USED FOR TROUBLESHOOTING
//  Oled.printString("DISTANCE:",1,2); // Print DISTANCE: // USED FOR TROUBLESHOOTING
  Oled.printString("MIDI NOTE:",1,4);  // Print MIDI NOTE:
  Oled.setBrightness(255);  // Change the brightness 0 - 255
             
}

void loop() {
  // extract string when a newline arrives:
  if (stringComplete) {
      String dist_str = inputString.substring(5, 10); // Extract Distance string
      cur_distance = (int16_t)dist_str.toInt(); // Convert distance string to an integer

      //new_midi_note = map(constrain(cur_distance, min dis, max dis), min dis, max dis, highest midi note, lowest midi note) // tesla's arc min and max are in millmeters
      new_midi_note = map(constrain(cur_distance, 1, 240),1, 240, 92, 77); // 1mm to 240mm = arc length, 92 to 77 midi note range // 76 is used for out of range limit
  
//      Oled.printNumber((long)sam_cnt, 8, 0);//  Print sample counter // USED FOR TROUBLESHOOTING
//      Oled.printString("       ", 8, 2); // Clear previous distance reading // USED FOR TROUBLESHOOTING
//      Oled.printNumber((long)cur_distance, 8, 2); //Print current distance // USED FOR TROUBLESHOOTING
      
      inputString = ""; //Clear string
      stringComplete = false;     
      sam_cnt++;  //Increase sample counter by 1
                      }
 
  // only if a new sensor value is detected, a new note will be sent. This will prevent looping the same note
  if (new_midi_note != old_midi_note) {
        
        Oled.printString("        ", 9, 4); // Clear previous MIDI data
        Oled.printNumber((long)new_midi_note, 9, 4); //Print the MIDI note
        MIDIOUT.write(0x80); //Silence old note (1 of 3)
        MIDIOUT.write(old_midi_note); //Silence old note (2 of 3)
        MIDIOUT.write(1); //Silence old note (3 of 3)
        
        MIDIOUT.write(0x90); // Send new note (1 of 3)
        MIDIOUT.write(new_midi_note); // Send new note (2 of 3)
        MIDIOUT.write(127); // Send new note (3 of 3)
        old_midi_note = new_midi_note;
                                      } 
  }                                


/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
   
    // add it to the inputString:
    inputString += inChar;
    
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {stringComplete = true;}
  }
}
