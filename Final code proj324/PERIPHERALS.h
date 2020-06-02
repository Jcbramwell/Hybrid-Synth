#define Fs 16000              //low sample rate for audio but is max Fs of ATMega328p

#define MIDI_BAUD_RATE 31250  //31.25 (+/- 1%) Kbaud (as stated in The MIDI 1.0 spec pg33)

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------  16 bit timer used to generate interrupt 16000 times per sec  -------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void TIMER1_INIT() 
{
  //Sets both registers to 0 to initialize
  
  TCCR1A = 0;                                                 // Control Reg A put into normal port operation (means counter direction is up from 0)
  TCCR1B = 0;                                                 // Control Reg B put into normal port operation

  //Clear Timer on Compare match (CTC) (Makes timer increment until reaching a certain value then clears, then repeats)
  TCCR1B |= (1 << CS10) | (1 << WGM12);                       //Puts 1 in CS10 no prescaling, puts 1 in WGM12 to select CTC

  //TCNT1 reg will be incremented and compared to OCR1A reg                                                                 
  TCNT1 = 0;                                                  //Counter initialise to 0
  TIMSK1 |= (1 << OCIE1A);                                    //TIMSK1 enables interrupts to trigger when TIMER1_INIT count reaches OCR1A 

  //OCR1A set to ensure correct sample rate
  OCR1A |= (F_CPU / Fs) - 1;                                  //F_CPU is clock rate of ucontroller (16MHz) over sample rate = 1000 and minus 1 (999) as begining at 0
}                                                             //Basically TIMER1_INIT counts from 0 to OCR1A and then resets to 0


//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------  Initialise USART  -----------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void USART_INIT ()
{
  uint16_t UBRR = ( F_CPU / 16 / MIDI_BAUD_RATE ) - 1;                           //UBRR USART Baud Rate Register (ATmega328p datasheet equation pg227)

  //Take the 16 bit UBRR value and put into two 8 bit registers
  UBRR0H = (uint8_t) (UBRR >> 8);
  UBRR0L = (uint8_t) (UBRR);

  //Setup USART to send and receive data
  UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);                          // configuring USART (RXEN = Enable receiver, TXEN = Enable transmitter, RXCIE = receiver interrupt enable, so interrupt everytime receive MIDI data

  //sets size of incoming data to 8bits (UCSZ00 and UCSZ01 = 8 bits)
  UCSR0C |=  (1 << UCSZ01) | (1 << UCSZ00) ;                                     
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------  Writes data to USART data reg  ---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void USART_Transmit (uint8_t data)
{
  while ( !( UCSR0A & (1<<UDRE0)) );        // wait until transfer ready checks to see if USARTY data register has an empty flag written to it
  UDR0 = data;                              // put data in USART data register
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
////-----------------------------------------------------------------------------------  Set up ATMega328p as Master device  ------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void SPIDAC_INIT()
{
  //Set up serial out pins as outputs in Data Direction Register (DDR)
  DDRB |= (1 << PINB5) | (1 << PINB3) | (1 << PINB2);         //Enables pins B5,B3,B2

  PORTB |= (1 << PINB2);                                      //(CS Chip select pin) Sets PINB2 HIGH as not writing to DAC straight away
  
  SPCR = (1 << SPE) | (1 << MSTR);                            //Flag SPE as 1 so that SPI is enabled and flag MSTR as 1 so Master SPI mode is set
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
////----------------------------------------------------------------------------------------  Transmit SPI data  ------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void SPI_transmit(uint16_t data)                              //16 bit data 
{
  PORTB &= ~(1 << PINB2);                                     //From MCP4921 datasheet CS needs to be pulled LOW

  //Two 8 bit data packets needed to send the 16 bits
  uint8_t dataPacket1 = 0b00110000 | (data >> 8);             //1st 8bit packet (bits 15-8): MSB 0 writes to DACA,next 0 sets VREF Input as unbuffered, next 1 sets 1x (VOUT = VREF * D/4096), next 1 enables Output Power Down Control. Other 0s are DAC data bits
  SPDR = dataPacket1;                                         //Sends data by putting it into SPDR (SPI Data Register)
  
  //Loop waits for Transfer to be complete
  while ( !(SPSR & (1 << SPIF)) );                            //Waits for SPSR and SPIF interrupt flag is high indicating transfer is complete
  
  uint8_t dataPacket2 = (data & 0b0000000011111111);          //2nd 8bit packet (bits 7-0) masks first 8 bits of 16 bit data
  SPDR = dataPacket2;                                         //Sends data by putting it into SPDR (SPI Data Register)
  while (!(SPSR & (1 << SPIF)) );                             //Waits for SPSR and SPIF interrupt flag is high indicating transfer is complete
  
  PORTB |= (1 << PINB2);                                      //Finally CS needs to be pulled HIGH
}
