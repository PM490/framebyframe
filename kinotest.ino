/*
  Kinograph Test Sketch
  Shield with 4 potentiometers
*/

const uint16_t MAX_PWM_VALUE = 0xffff;

// analog values
   uint16_t val0 =0;
   uint16_t val1 =0;
   uint16_t val2 =0;
   uint16_t val3 =0;

void setup() {
  init_pwm();
}

void loop() {
  // Reads Analog inputs 0 to 3
  read_pot_led();
  delay(50);
}
   
void read_pot_led(void) {
  val0 = (uint16_t) analogRead(A0);
  val0 = val0 * 4;
  val1 = (uint16_t) analogRead(A1);
  val1 = val1 * 4;
  val2 = (uint16_t) analogRead(A2);
  val2 = val2 * 4;
  val3 = (uint16_t) analogRead(A3);
  val3 = val3 * 4;

  noInterrupts();
  set_PWMexp(2,val3); //Red - Channel 4
  set_PWMexp(6,val2); //Green - Channel 3
  set_PWMexp(7,val1); //Blue - Channel 2
  set_PWMexp(8,val0); //White - Channel 1
  interrupts();
  
}

void init_pwm() {
  noInterrupts();
  // Set Mega Timers for 16 bit
  TCCR3A = 1 << WGM31 | 1 << COM3A1 | 1 << COM3B1 | 1 << COM3C1; // set on top, clear OC on compare match
  TCCR3B = 1 << CS30  | 1 << WGM32 | 1 << WGM33;   // clk/1, mode 14 fast PWM
  TCCR4A = 1 << WGM41 | 1 << COM4A1 | 1 << COM4B1 | 1 << COM4C1; // set on top, clear OC on compare match
  TCCR4B = 1 << CS40  | 1 << WGM42 | 1 << WGM43;   // clk/1, mode 14 fast PWM
  ICR3 = MAX_PWM_VALUE;
  ICR4 = MAX_PWM_VALUE;
  
  // Start PWMs with 50% cycle
  OCR3B = 0x8000; //Pin2
  OCR3C = 0x8000; //Pin3
//  OCR3A = 0x8000; //Pin5 - Output 5 is bad on my Mega, do not use
  OCR4A = 0x8000; //Pin6
  OCR4B = 0x8000; //Pin7
  OCR4C = 0x8000; //Pin8
  interrupts();

  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT); //Use at 50% cycle for scope trigger
//  pinMode(pwm5pin, OUTPUT); // Output 5 is bad on my Mega, do not use
  pinMode(6, OUTPUT); 
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
}

// ============================================
// Set PWM exponential values 0 to 0xfff 12 bit
// ============================================

void set_PWMexp(byte pinNumber, uint16_t pwm) {
  uint16_t pwmexp = 0;
  pwmexp = 0xffff - pow( (double) pwm,1.33337);
 
  switch (pinNumber) {
    case 2 : //pin2 -- Red Channel
      OCR3B = pwmexp;
      break;
    case 3 : //pin3
      OCR3C = pwmexp;
      break;
/*    case 5 : //pin5 // Output 5 is bad on my Mega, do not use
      OCR4A = pwmexp;
      break;
*/
    case 6 : //pin6 -- Green Channel
      OCR4A = pwmexp;
      break;
    case 7 : //pin7 -- Blue Channel
      OCR4B = pwmexp;
      break;
    case 8 : //pin8 -- White Channel
      OCR4C = pwmexp;
      break;  
  }

}
