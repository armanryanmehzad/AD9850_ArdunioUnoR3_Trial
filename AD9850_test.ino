/*
DATE: 07/26/2023
PROGRAMMER: Arman Mehzad
PURPOSE: Initial AD9850 Frequency Test
BOARD: Arduino Uno R3
NOTES: 07/26/2023. I think there is something wrong with the FQ_UD name itself... functionality should not be impaired but the board itself says 'FU_UD'.
REFRENCES: 1. https://github.com/jmharvey1/DDS_AD9850_AntennaAnalyzer/blob/master/Arduino%20Sketch%20Files/Ant_Analyzer/Ant_Analyzer.ino
           2. https://www.analog.com/media/en/technical-documentation/data-sheets/AD9850.pdf
*/

// Defining variables and pin connections, as well as pulse function.
#define W_CLK 8  // W_CLK should be connected to digital port 8.
#define FQ_UD 9  // FQ_UD should be connected to digital port 9.
#define DATA 10  // DATA should be connected to digital port 10.
#define RESET 11  // RESET should be connected to digital port 11.
#define pulseHigh(pin) {digitalWrite(pin, HIGH); digitalWrite(pin, LOW);}  // Function to send a short pulse from one of the digital ports/pins.
double clockfreq = 125000000.0;  // AD9850 clock frequency is 125MHz.
double ttisfreq = 2000.0; // Literature on tTIS stimulation suggests using 2000Hz +/- 10/70Hz. For tACS, [INSERT HERE - 07/26/2023]
double current_freq;

/*
FUNCTION: Byte Transfer Function
LAST EDIT: 07/26/2023
USAGE: Byte transfer function to the AD9850 through the DATA serial line (digital port 10 on the Arduino to D7 on AD9850).
THEORY: This code is going to program the AD9850 with the serial load mode. There are two options: serial or parallel load. Frankly, I don't totally understand
the parallel load or how I'm supposed to achieve it, but I do understand the process by which the serial programming works.

The AD9850 has an 40-bit register that is used to program the frequency control word, which itself is used to determine the output frequency. Serial load mode 
takes [INSERT HERE - 07/26/2023].
*/
void byte_transfer(byte data)
{
  Serial.print(data);
  for (int i=0; i<8; i++, data>>=1)  // This for loop will continue until i<8 is no longer true, and will shift the byte variable to the right by one spot.
  {
    digitalWrite(DATA, data & 0x01);  // This line sets the DATA pin to whatever the least significant bit of data is using the AND operator.
    pulseHigh(W_CLK);  // A rising edge from W_CLK will cause the AD9850 to shift through the 1-bit data on the D7 Data pin. This allows the next bit to be written due to the for loop.
  }
}

/*
FUNCTION: Frequency Transfer & Calculator Function
LAST EDIT: 07/26/2023
USAGE: Caluclates the frequency which is desired for the output signal and uses the byte_transfer function to deliver it to the AD9850.
THEORY: Takes tuning_word for frequency and calculates the output frequency with the formula provided in the AD9850 datasheet. [INSERT HERE - 07/26/2023]

System clock is 125MHz, 2^32 = 4294967295, and we have f_out = (tuning word * reference clock) / 2^32. If we rearrange that formula, we get tuning word = f_out * (2^32 / reference clock).
*/
void send_frequency(double f_out)
{
  unsigned long tuning_word = f_out * (4294967295.0/125000000.0);  // Rearranged formula described above.
  Serial.print(tuning_word);  // Debugging print statement.
  for (int b=0; b<4; b++, tuning_word>>=8)
  {
    byte_transfer(tuning_word & 0xFF);
  }
  byte_transfer(0x000);  // The final control byte for the AD9850 is just all zeros. [I am assuming this has to do with Figure 9 on the datasheet - unsure for now. 07/26/2023]
  pulseHigh(FQ_UD);  // An FQ_UD pulse is necessary to update the frequency on the AD9850.
}

void setup()
{
  pinMode(W_CLK, OUTPUT);
  pinMode(FQ_UD, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(RESET, OUTPUT);

  // To begin programming the AD9850, and to ensure that it is indeed in serial load mode, the following pulses are sent.
  pulseHigh(RESET);
  pulseHigh(W_CLK);
  pulseHigh(FQ_UD);
  Serial.begin(9600);
  current_freq = ttisfreq;
}

void loop()
{
  send_frequency(ttisfreq);
  Serial.print("Loop Beginning!\n");
  delay(10000000);
}
