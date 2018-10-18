/*=====================================================================
 * Using a 74595 to illustrate Shift Register concepts. 
 * 
 * See associated wiki for the wiring schematic.       
 */

#define DATA_PIN  A0
#define CLK_PIN   A1
#define LATCH_PIN A2

#define OUTPUT_LINES 8

int autolatch = 1;

// outputs a bit to the shift register and clocks it in.
void write_bit(int bit)
{
  digitalWrite(DATA_PIN, bit);
  digitalWrite(CLK_PIN, HIGH);
  digitalWrite(CLK_PIN, LOW);
}

// stores the shift registers in the latches
void latch_data( void )
{
  digitalWrite(LATCH_PIN, HIGH);
  digitalWrite(LATCH_PIN, LOW);
}

/* NOTE:  We're not currenly writing bytes, but I'm leaving this in here for future use */
#define BITS_IN_BYTE 8
void write_and_latch_byte( int data )
{
  int temp_bit;
  int temp_data;
  int i;
  
  data &= 0xFF;
    
  temp_data = data;
  
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

void print_help( void )
{
  Serial.println("Type 0 to shift a 0.  Type 1 to shift a 1.");
  Serial.println("s or S enters shift mode (no autolatch)");
  Serial.println("l or L latches the data");
  Serial.println("a or A enters autolatch mode");
  Serial.print("Auto-latch is currently ");
  if (autolatch) Serial.println("ON");
  else Serial.println("Off");

}

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

  delay(1000);
  
  // now a quick LED test...write a 1 to all registers.
  for (i=0; i<OUTPUT_LINES; i++)
  {
    write_bit(1);
    latch_data();
    delay(1000);
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

void loop() 
{
  char input;
  
  while (Serial.available())
  {
    input = Serial.read();
    switch (input)
    {
      case '0':
        write_bit(0);
        if (autolatch) latch_data();
        Serial.println("Shift 0");
      break;

      case '1':
        write_bit(1);
        if (autolatch) latch_data();
        Serial.println("Shift 1");
      break;

      case 'a':
      case 'A':
        autolatch = 1;
        Serial.println("Autolatch ON");
      break;

      case 's':
      case 'S':
        autolatch = 0;
        Serial.println("Shift mode (autolatch OFF)");
      break;
      
      case 'l':
      case 'L':
        latch_data();
        Serial.println("Latch!");
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
}
