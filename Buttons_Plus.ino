/* Buttons to USB MIDI Example

   You must select MIDI from the "Tools > USB Type" menu

   To view the raw MIDI data on Linux: aseqdump -p "Teensy MIDI"

   This example code is in the public domain.
*/

#include <Bounce.h>

// the MIDI channel number to send messages
const int channel = 1;
// the MIDI channel to reflect
const int reflectChannel = 10;
// the MIDI channel to split
const int splitChannel = 12;
// map of split notes:
boolean splitMap[128];
// state of footswitch on pin1
boolean pin1Closed = false;
// counter 
elapsedMillis clock;

// Create Bounce objects for each button.  The Bounce object
// automatically deals with contact chatter or "bounce", and
// it makes detecting changes very simple.
Bounce button0 = Bounce(0, 5);
Bounce button1 = Bounce(1, 5);  // 5 = 5 ms debounce time
Bounce button2 = Bounce(2, 5);  // which is appropriate for good
Bounce button3 = Bounce(3, 5);  // quality mechanical pushbuttons
Bounce button4 = Bounce(4, 5);
Bounce button5 = Bounce(5, 5);  // if a button is too "sensitive"
Bounce button6 = Bounce(6, 5);  // to rapid touch, you can
Bounce button7 = Bounce(7, 5);  // increase this time.
Bounce button8 = Bounce(8, 5);
Bounce button9 = Bounce(9, 5);
//Bounce button10 = Bounce(10, 5);
//Bounce button11 = Bounce(11, 5);

// 8 buttons on the digital multiplex on pin 12
Bounce *buttonsM0[8]; // http://nihlaeth.nl/2014/05/10/declaring-an-array-of-objects-in-arduino-cpp/



void setup() {
  // Configure the pins for input mode with pullup resistors.
  // The pushbuttons connect from each pin to ground.  When
  // the button is pressed, the pin reads LOW because the button
  // shorts it to ground.  When released, the pin reads HIGH
  // because the pullup resistor connects to +5 volts inside
  // the chip.  LOW for "on", and HIGH for "off" may seem
  // backwards, but using the on-chip pullup resistors is very
  // convenient.  The scheme is called "active low", and it's
  // very commonly used in electronics... so much that the chip
  // has built-in pullup resistors!
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);  // Teensy++ 2.0 LED, may need 1k resistor pullup
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP); // Teensy 2.0 LED, may need 1k resistor pullup
  
  // 74HC4051 pins:
  pinMode(12, INPUT_PULLUP); // multiplex read
  pinMode(14, OUTPUT); // multiplex LSB
  pinMode(15, OUTPUT); // multiples MiddlingSignificantBit
  pinMode(16, OUTPUT); // multiplex MSB
  
  
  // set up callbacks for midi messages
  usbMIDI.setHandleNoteOn(OnNoteOn);
  usbMIDI.setHandleNoteOff(OnNoteOff);
  
  // initialize splitMap
  int i;
  for (i = 0; i < 128; i++){
    splitMap[i] = false;
  }
  
  // initialize button objects for multiplex 0
  for (i = 0; i < 8; i++) {
//    buttonsM0[i] = Bounce(12, 5);  // TODO: how to phrase this?
  }

  // say howdy
  Serial.begin(9600); // USB is always 12 Mbit/sec
  Serial.println("Good morning!");
}

// basic note reflector
void OnNoteOn(byte channel, byte note, byte velocity){
  Serial.println("note on");
  if (channel == reflectChannel) {
    // If the footswitch on pin1 is down, toggle the map entry for this note
    if (pin1Closed){
      splitMap[note] = ! splitMap[note];
      Serial.print('/');
    }
    
    // If the entry for this note in the splitMap is true, send it on the split channel
    if (splitMap[note]){
      usbMIDI.sendNoteOn(note, velocity, splitChannel);
      Serial.print('*');
    } else {
    // Otherwise, resend (reflect) it on the channel it came in on)
      usbMIDI.sendNoteOn(note, velocity, channel);
      Serial.print('.');
    }      
  }
  // ignoring all other channels.
}
void OnNoteOff(byte channel, byte note, byte velocity){
  Serial.println("note off");
  if (channel == reflectChannel) {
    // If the entry for this note in the splitMap is true, send it on the split channel
    if (splitMap[note]){
      usbMIDI.sendNoteOn(note, velocity, splitChannel);
      Serial.print('*');
    } else {
    // Otherwise, resend (reflect) it on the channel it came in on)
      usbMIDI.sendNoteOff(note, velocity, channel);
      Serial.print('.');
    }      
  }
  // ignoring all other channels.
}


// TODO: generalize to noteOff as well

void loop() {
  
  /*
  // prove to me that you exist:
  if (clock > 2000) {
    clock = clock - 1000;
    Serial.println(".");
  }
  */
  
  // Update all the buttons.  There should not be any long
  // delays in loop(), so this runs repetitively at a rate
  // faster than the buttons could be pressed and released.
  button0.update();
  button1.update();
  button2.update();
  button3.update();
  button4.update();
  button5.update();
  button6.update();
  button7.update();
  button8.update();
  button9.update();
  
  
  // multiplex read:
  int i;
  for (i = 0; i < 8; i++){
     // select 74HC4051 channel
    digitalWrite(14, (i >> 2) & 1); 
    digitalWrite(15, (i >> 1) & 1);
    digitalWrite(16, i & 1);
      // allow 50 us for signals to stablize
    delayMicroseconds(50);
    // read pin
    buttonsM0[i]->update();
    
    if (buttonsM0[i]->fallingEdge()){
      Serial.print("pin M0.");
      Serial.print(i, DEC); 
      Serial.println(" on");
    }
    
    if (buttonsM0[i]->risingEdge()){
      Serial.print("pin M0.");
      Serial.print(i, DEC); 
      Serial.println(" off");
    }
  }
  
  
  // Check each button for "falling" edge.
  // Send a MIDI Note On message when each button presses
  // Update the Joystick buttons only upon changes.
  // falling = high (not pressed - voltage from pullup resistor)
  //           to low (pressed - button connects pin to ground)
  if (button0.fallingEdge()) {
    usbMIDI.sendNoteOn(60, 99, channel);  // 60 = C4
    Serial.println("pin 1 on");
    pin1Closed = true;
  }
  if (button1.fallingEdge()) {
    usbMIDI.sendNoteOn(61, 99, channel);  // 61 = C#4
       Serial.println("pin 2 on");
  }
  if (button2.fallingEdge()) {
    usbMIDI.sendNoteOn(62, 99, channel);  // 62 = D4
   Serial.println("pin 3 on");
   }
  if (button3.fallingEdge()) {
    usbMIDI.sendNoteOn(63, 99, channel);  // 63 = D#4
  }
  if (button4.fallingEdge()) {
    usbMIDI.sendNoteOn(64, 99, channel);  // 64 = E4
  }
  if (button5.fallingEdge()) {
    usbMIDI.sendNoteOn(65, 99, channel);  // 65 = F4
  }
  if (button6.fallingEdge()) {
    usbMIDI.sendNoteOn(66, 99, channel);  // 66 = F#4
  }
  if (button7.fallingEdge()) {
    usbMIDI.sendNoteOn(67, 99, channel);  // 67 = G4
  }
  if (button8.fallingEdge()) {
    usbMIDI.sendNoteOn(68, 99, channel);  // 68 = G#4
  }
  if (button9.fallingEdge()) {
    usbMIDI.sendNoteOn(69, 99, channel);  // 69 = A5
  }

  // Check each button for "rising" edge
  // Send a MIDI Note Off message when each button releases
  // For many types of projects, you only care when the button
  // is pressed and the release isn't needed.
  // rising = low (pressed - button connects pin to ground)
  //          to high (not pressed - voltage from pullup resistor)
  if (button0.risingEdge()) {
    usbMIDI.sendNoteOff(60, 0, channel);  // 60 = C4
       Serial.println("pin 1 off");
       pin1Closed = false;
  }
  if (button1.risingEdge()) {
    usbMIDI.sendNoteOff(61, 0, channel);  // 61 = C#4
       Serial.println("pin 2 off");
  }
  if (button2.risingEdge()) {
    usbMIDI.sendNoteOff(62, 0, channel);  // 62 = D4
  }
  if (button3.risingEdge()) {
    usbMIDI.sendNoteOff(63, 0, channel);  // 63 = D#4
  }
  if (button4.risingEdge()) {
    usbMIDI.sendNoteOff(64, 0, channel);  // 64 = E4
  }
  if (button5.risingEdge()) {
    usbMIDI.sendNoteOff(65, 0, channel);  // 65 = F4
  }
  if (button6.risingEdge()) {
    usbMIDI.sendNoteOff(66, 0, channel);  // 66 = F#4
  }
  if (button7.risingEdge()) {
    usbMIDI.sendNoteOff(67, 0, channel);  // 67 = G4
  }
  if (button8.risingEdge()) {
    usbMIDI.sendNoteOff(68, 0, channel);  // 68 = G#4
  }
  if (button9.risingEdge()) {
    usbMIDI.sendNoteOff(69, 0, channel);  // 69 = A5
  }

  // MIDI Controllers should discard incoming MIDI messages.
  // http://forum.pjrc.com/threads/24179-Teensy-3-Ableton-Analog-CC-causes-midi-crash
  while (usbMIDI.read()) {
    // ignore incoming messages
  }
  
}

