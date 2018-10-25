/*=====================================================================
 * Using a 74595 to illustrate Shift Register concepts. 
 * 
 * See associated wiki for the wiring schematic.       
 */

// Touchpot uses I2C.
// For the Uno,  A4 is SDA and A5 is SCL.
#include "Wire.h"

/* Touchpot Register definitions
/* see http://danjuliodesigns.com/products/touch_pot/assets/touch_pot_sf_1_4.pdf */
#define TOUCHPOT_VERSION       0   // Read only
#define TOUCHPOT_CUR_POT_VALUE 1   // RW, Current poteniometer value
#define TOUCHPOT_STATUS        2   // Read only, Device Status
#define TOUCHPOT_CONTROL       3   // RW, device configuration
#define TOUCHPOT_USER_LED      4   // RW, user set led value.

int i2cAddr = 8; // Direct access at i2cAddr, indirect registers at i2cAddr+1

// 74595 lines
#define DATA_PIN  A0
#define CLK_PIN   A1
#define LATCH_PIN A2

// We have 8 output LEDs in our current incarnation.
#define OUTPUT_LINES 8

int autolatch = 1;
int touchpot_enabled = 0;

// When we access the touchpot, how is it going to illuminate the LEDs?  Currently
// only "binary" mode is implemented.
typedef enum
{
  TOUCHPOT_BAR,      // Bargraph style.  Illuminate all leds up to touchpot value.
  TOUCHPOT_SLIDER,   // Only illumitate the top LED from the touchpot value.
  TOUCHPOT_BINARY    // Illumiate LEDS that match touchpot value as binary #.
} touchpot_mode_type;

touchpot_mode_type touchpot_mode = TOUCHPOT_BINARY; 

//==============================================================================================
// FUNCTION:  write bit.
//  
// Outputs a bit to the shift register and clocks it in.
//==============================================================================================
void write_bit(int bit)
{
  digitalWrite(DATA_PIN, bit);
  digitalWrite(CLK_PIN, HIGH);
  digitalWrite(CLK_PIN, LOW);
}

//==============================================================================================
// FUNCTION:  latch_data
//
// stores the current shift registers in the latch memorie  
//==============================================================================================
void latch_data( void )
{
  digitalWrite(LATCH_PIN, HIGH);
  digitalWrite(LATCH_PIN, LOW);
}

//==============================================================================================
// FUNCTION:  write_and_latch_byte
//
// Writes an entire byte to the shift register, and then latches the data.  
//==============================================================================================
#define BITS_IN_BYTE 8
void write_and_latch_byte( int data )
{
  int temp_bit;
  int temp_data;
  int i;
  
  data &= 0xFF;
    
  temp_data = data;

  Serial.print("Byte: ");
  Serial.println(data);
  
  for (i = 0; i < BITS_IN_BYTE; i++)
  {
    // we only want the MSB
    temp_bit = temp_data & 0x80;
    temp_bit = temp_bit >> 7;
    write_bit(temp_bit);

    // now shift our byte to get the next bit
    temp_data = temp_data << 1;   
  }

  latch_data();
  
}

//==============================================================================================
// FUNCTION:  print_help
//
// Prints out the serial port a help message, telling what commands do what.
//==============================================================================================
void print_help( void )
{
  Serial.println("Type 0 to shift a 0.  Type 1 to shift a 1.  Both disable the touchpot.");
  Serial.println("s or S enters shift mode (no autolatch, disables touchpot)");
  Serial.println("l or L latches the data.  Disables touchpot.");
  Serial.println("a or A enters autolatch mode (disables touchpot)");
  Serial.println("t or T enables the touchpot");
  Serial.print("Auto-latch is currently ");
  
  if (autolatch) Serial.println("ON");
  else Serial.println("Off");

  Serial.print("Touch pot is currently ");
  if (touchpot_enabled) Serial.println("ENABLED");
  else Serial.println("DISABLED");
}

//==============================================================================================
// FUNCTION:  setup
//==============================================================================================
void setup() 
{
  int i;
  
  Serial.begin(9600);

  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);

  // Make sure all lines are low
  digitalWrite(DATA_PIN, LOW);
  digitalWrite(CLK_PIN, LOW);
  digitalWrite(LATCH_PIN, LOW);
  
  // make sure our shift register and latches are all 0.
  for (i=0; i<OUTPUT_LINES; i++)
  {
    write_bit(0);
  }
  latch_data();

  delay(500);
  
  // now a quick LED test...write a 1 to all registers.
  for (i=0; i<OUTPUT_LINES; i++)
  {
    write_bit(1);
    latch_data();
    delay(500);
  }

  // ...and now put them back to all zeros before starting for real.
  for (i=0; i<OUTPUT_LINES; i++)
  {
    write_bit(0);
  }
  latch_data();

  delay(1000);

  Serial.println("Shift version:  Initialization done.");
  print_help();  
}

//==============================================================================================
// FUNCTION:  loop
//==============================================================================================
void loop() 
{
  char       input;
  int        current_tp_value;
  static int last_tp_value=0;
  
  while (Serial.available())
  {
    input = Serial.read();
    switch (input)
    {
      case '0':
        touchpot_enabled = 0;
        write_bit(0);
        if (autolatch) latch_data();
        Serial.println("Shift 0");
      break;

      case '1':
        touchpot_enabled = 0;
        write_bit(1);
        if (autolatch) latch_data();
        Serial.println("Shift 1");
      break;

      case 'a':
      case 'A':
        autolatch = 1;
        touchpot_enabled = 0;
        Serial.println("Autolatch ON (touchpot disabled)");
      break;

      case 's':
      case 'S':
        autolatch = 0;
        touchpot_enabled = 0;
        Serial.println("Shift mode (autolatch OFF, touchpot disabled)");
      break;
      
      case 'l':
      case 'L':
        touchpot_enabled = 0;
        latch_data();
        Serial.println("Latch!");
      break;

      case 't':
      case 'T':
        touchpot_enabled = 1;
        Serial.println("Touchpot ENABLED");
      break;
      
      case '\n':
        // ignore newlines
      break;

      default:
        Serial.print("Unexpected input: ");
        Serial.println(input);
        print_help();    
    }  // End switch on input character
  }  // End while serial available

  if (touchpot_enabled)
  {
    current_tp_value = read_tp_value();

    Serial.print("Current: ");
    Serial.print(current_tp_value);
    Serial.print(", Last: ");
    Serial.println(last_tp_value);
    
    if (current_tp_value != last_tp_value)
    {
      write_and_latch_byte(current_tp_value);
      last_tp_value = current_tp_value;
      
      Serial.print("Touchpot :");
      Serial.println(current_tp_value);
    }
  }

  delay(50);
}


//==============================================================================================
// FUNCTION:  write_tp_reg
//
//  Uses I2C to write a value to the specified touchpot register. 
//==============================================================================================
void write_tp_reg(uint8_t addr, uint8_t data) 
{
  Wire.beginTransmission(i2cAddr+1);
  Wire.write('W');
  Wire.write(addr);
  Wire.write(data);
  Wire.endTransmission();
}

//==============================================================================================
// FUNCTION:  read_tp_value
//
// Uses I2C to read the curent value of touchpot.  Returns 0  - 255.
//==============================================================================================
uint8_t read_tp_value( void ) 
{
  Wire.requestFrom(i2cAddr, TOUCHPOT_CUR_POT_VALUE);

  if (Wire.available()) 
  {
    return Wire.read();
  } 
  else 
  {
    return 0;
  }
}

//==============================================================================================
// FUNCTION:  read_tp_reg
//
// Uses I2C to read the value of the specified touchpot register.
//==============================================================================================
uint8_t read_tp_reg(uint8_t addr) 
{
  Wire.beginTransmission(i2cAddr+1);
  Wire.write('R');
  Wire.write(addr);
  Wire.endTransmission();

  Wire.requestFrom(i2cAddr+1, 1);

  if (Wire.available()) 
  {
    return Wire.read();
  } 
  else 
  {
    return 0;
  }
}
