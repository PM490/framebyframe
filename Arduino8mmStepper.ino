/*
=================================
Frame by Frame Film Scanner
FBFFS
Control NEMA Stepper with TMC2208
by PM490 8/31/2021
=================================

NOTE: THIS VERSION KEEPS THE DRIVER DISABLED EXCEPT WHEN MOVING. 
THE STEPPER DRIVER IS DISENGAGED AFTER EACH COMMAND/ENGAGED FOR MOVING ONLY.

If one wishes the Stepper to be enabled, edit TMC_EN_PIN.

*/

// ==============================
// TMC2208 - Stepper Driver Pins
// ==============================

// TMC - Define Pins
#define TMC_EN_PIN 5
#define TMC_STEP_PIN 6
#define TMC_DIR_PIN 7

// ==============================
// DigicamControl (DCC)
// Interface with
// Arduino Plugin
// Serial Commands
// ==============================

// From DCC to Arduino
// W - Windows command after a file is transferred
#define dccFileXferComm 'W'

// From Arduino to DCC

// A - Advance one turn
#define dccFwdComm 'A'

// B - Back one turn
#define dccBackComm 'B'

// S - Start - Start Scan Sequence
#define dccStartComm 'S'

// E - End - End Scan Sequence
#define dccEndComm 'E'

#define dccLeadComm '#'
#define dccLagComm '/'

// C - Capture
#define dccTakeComm "#T/"


// ==============================
// Global Variables
// ==============================

//Flags indicating command received is End
bool dccEndRcvd = false;

void setup() {
  // initialize the serial port:
  Serial.begin(9600);

  // initialize TMC PINS
  pinMode(TMC_EN_PIN, OUTPUT);
  digitalWrite (TMC_EN_PIN, HIGH); //Deactivate driver - Active LOW
  pinMode(TMC_DIR_PIN, OUTPUT);
  digitalWrite (TMC_DIR_PIN, LOW);
  pinMode(TMC_STEP_PIN, OUTPUT);
  digitalWrite (TMC_STEP_PIN, LOW);
  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  char dccComm = 0;
  delay(250);
  //Receive a DigicamControl Serial Command (three character sequence).
  dccComm = rcvSerialComm();

  //Process DCC Command Received
  switch (dccComm) {
    case dccFwdComm :    // Advance Film 1 frame Forward - It requires Stepper Home - it does not require that film is detected
      tmcAccelFullTurn(1,0);
      break;
    case dccBackComm :    // Advance Film 1 frame Back - It requires Stepper Home - it does not require that film is detected
      tmcAccelFullTurn(0,0);
      break;
    case dccStartComm :    // Start Scanning Sequence - Arduino Sends Take to DCC, DCC returnes file transferred, One Turn Forward. Then repeats the sequence until the End Command is received from DCC.
	  dcccommS();
      break;
	  
	// Additional case maybe added for implementing DCC new commands - Built In LED used as Example below.
    case '1' :    // Turn Builin LED ON
	  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      break;
	case '0' :    // Center Stepper - It requires Stepper Home - it does not require that film is detected
	  digitalWrite(LED_BUILTIN, LOW);    // turn the LED OFF (LOW is the voltage level)
      break;  
    }
  }  
  
// ==============================
// Implementation of DCC Command  
// Digitize Sequence  
// ==============================

void dcccommS ( void ) {
  bool FilmStatus = false ;

  do {
    // Send Capture command to DigicamControl
    Serial.println (dccTakeComm);
	delay(250); //Capture delay
	
	// F - Windows command after a file is transferred
	waitSerialSeq ( dccFileXferComm );
	
	if (!dccEndRcvd) {
 	  // Move one Turn 
	  tmcAccelFullTurn(1,0);      	
	  //Wait before continuing
      delay(500);
	  }

	} while ( !dccEndRcvd);
	
	dccEndRcvd = false; //Clear EndRcvd Flag
}
  
  
// ==============================
// DCC Serial Sequence Commands
// ==============================

//Pause until a Serial Command passed as argument is received
void waitSerialSeq ( char waitForCommand ) {
  boolean commAwait = 0;
  char incomingByte = 0;
  
  do {
    incomingByte = rcvSerialComm ();
	if ((char) incomingByte == dccEndComm ) {
	  dccEndRcvd = true;
	  }
	}
	while ( (incomingByte != waitForCommand) && !dccEndRcvd );
  }

  
// Received a command Sequence
// returning a single character as received command
char rcvSerialComm ( void ) {
  char inChar = 0;
  char rcvComm = 0;

  // Wait for #
  do {
    inChar = (char) Serial.read();
	} while (inChar != dccLeadComm);
	// Wait for next Character
	while ( Serial.available () == 0) {
	  };
	// Receive Command Character
    rcvComm = (char) Serial.read();
	// Wait for /
  do {
    inChar = (char) Serial.read();
	} while (inChar != dccLagComm);
  return rcvComm;
  }

 
// ==============================
// TMC 2208
// ==============================

// ============================== 
// Accelerated/Decelerated Full Turn  
// Fuction composed of 800 steps asceleration
// and 800 decelaration (One Turn = 1600)
// Additional Steps in tmcMoreSteps
// (tmcMoreSteps = 1600, will make 1 additional Turn)
// tmcDirection -> 1 Forward, 0 Backward
 #define STEPS 800
 void tmcAccelFullTurn( boolean tmcDirection, int tmcMoreSteps ) { 
   int tmcDELAY[STEPS];
   float angle = 3.141593;
   float accel = .7;
   float c0 = 2000 * sqrt( 2 * angle / accel ) * 0.67703;
   float lastDelay = 0;
   int highSpeed = 150;
   for (int i = 0; i < STEPS; i++) {
    float d = c0;
    if ( i > 0 )
      d = lastDelay - (2 * lastDelay) / (4 * i + 1);
    if ( d < highSpeed )
      d = highSpeed;
    tmcDELAY[i] = d;
    lastDelay = d;
    }
 
  digitalWrite (TMC_EN_PIN, LOW); //Activate driver - Active LOW
  if (tmcDirection) {
       digitalWrite (TMC_DIR_PIN, LOW); //Set Step Direction Forward
	   }
	 else {
	   digitalWrite (TMC_DIR_PIN, HIGH); //Set Step Direction Reverse
	   }
   delayMicroseconds(500);
 
  // Half Turn asceleration    
  // use delays from the array, forward
  for (int i = 0; i < STEPS; i++) {
    tmcOneStep (tmcDELAY[i]);
    }

  // Additional steps if received as argument	
  if (tmcMoreSteps > 0 ) {	
    for (int i = 0; i < tmcMoreSteps; i++) {
	  tmcOneStep(tmcDELAY[STEPS-1]);
      }
    }		
	
  // Half Turn decelaration	
  // use delays from the array, backward
  for (int i = 0; i < STEPS; i++) {
    tmcOneStep(tmcDELAY[STEPS-i-1]);
    }

  delayMicroseconds(300);
  digitalWrite (TMC_DIR_PIN, LOW); //Set Step Direction Forward // Leave it FWD.
  digitalWrite (TMC_EN_PIN, HIGH); //Deactivate driver - Active LOW
  }  
  
  
// ==============================
// Single Step Function  
// ==============================

void tmcOneStep (int tmcPulse) {
  //make steps
  digitalWrite (TMC_STEP_PIN, HIGH);
  delayMicroseconds(50);
  digitalWrite (TMC_STEP_PIN, LOW);
  delayMicroseconds(tmcPulse - 50);
  }  
