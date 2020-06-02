#include "PERIPHERALS.h"
#include "SAMPLES_WAVEFORM_GEN.h"
#include "USARTISR_MIDI.h"

#define GATE_OUT_PIN PINB4                                  // For envelope generator and LED PINB4 used to control gate output
#define SELECT_WAVE_PIN PINB1                               // PINB1 used as input from button to toggle through waveforms
#define INCREMENT_TABLE_SIZE 50                             // Increment table used to increment through table at a given frequency without needing floating point numbers (the larger the size, the more accurate the frequency)

typedef struct voice                                        // this struct represents a synthesizer voice
{
  volatile uint16_t table_position = 0;                     // table_position determines where in the waveform table the voice is currently (volatile as will be written to timer 1 interrupt)
  uint8_t note = 0;                                         // note records the current midi note value
  uint8_t amplitude_val = 0;                                // amplitude_val determines the amplitude value to apply to the note (should be 0 to 16)
  uint8_t increment_table[INCREMENT_TABLE_SIZE];            // the increment table values are used to increment through the table at a given frequency without requiring floating point numbers
  volatile uint8_t increment_table_pos = 0;                 // a value used to increment through the increment_table (volatile as will be written to timer 1 interrupt)
  uint8_t previous_notes[10];                               // these are the previous note values in case of a key being released while other keys are still pressed, the Voice will play a previous note value until all keys are released
  uint8_t notes_pressed = 0;                                // the number of previous notes still being held
  uint8_t increment_table_proc = 0;                         // 1 if the increment_table needs to be reprocessed, 0 if it doesn't
} Voice;

Voice osc;                                                  // the voice that will act as a digital oscillator


void processMidiEvent (const MidiEvent* const midiEvent);   // takes in a MidiEvent and uses its data to correctly update the Voice data structure
void process_increment_table();                             // processes the increment_table values for the Voice

uint8_t currentWaveIsSelected = 0;                          // Boolean value used so when button pressed, synth doesnt change through all waveforms
uint8_t currentWaveLocation = 0;                            // determines which wave is selected sine=0, tri=1, square=2, saw=3

//------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------SETUP-------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup () 
{
  cli();  // disable interrupts

  DDRB |= (1 << GATE_OUT_PIN);            // FOR envelope generator gate output and LED
  PORTB &= ~(1 << GATE_OUT_PIN);          // FOR envelope generator gate output/LED pin low 
  DDRB &= ~(1 << SELECT_WAVE_PIN);        // wave selection pin is input for button press

  // peripheral setup
  TIMER1_INIT();
  USART_INIT();
  SPIDAC_INIT();

  // setup waveform buffers
  generateSine();             //Call sine function
  generateTriangle();         //Call Triangle function
  generateSquare();           //Call square function
  generateSawtooth();         //Call saw function
  
  sei();  // enable interrupts
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------MAIN LOOP---------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------

void loop () 
{
  while (midiReadIndex != midiWriteIndex) // while there are MidiEvents in the MIDI event buffer, process them
  {
    processMidiEvent(&midiEventBuffer[midiReadIndex]);
    midiReadIndex = (midiReadIndex + 1) % MIDI_EVENT_BUFFER_SIZE;
  }

  if (osc.increment_table_proc)     // if increment_table needs processing (this is done after the processMidiEvent loop so that if a bunch of notes are pressed at once, we don't waste processing on all of them since their increment_tables won't be used)
  {
    process_increment_table();
  }
  
  if (PINB & (1 << SELECT_WAVE_PIN) && !currentWaveIsSelected)   // if wave selection button pressed and hasn't stayed high since previous loop
  {
    currentWaveLocation = (currentWaveLocation + 1) % 4;            // Update current wave location % to wrap around the 4 waveforms
    switch (currentWaveLocation)                                    // Sets current wave pointer to the right waveform
    {
      case 0  :                                                     // If in position 0 sine
        wave_pointer = sineWave;
        break;
      case 1  :                                                     // If in position 1 triangle
        wave_pointer = triangleWave;                                 
        break;
      case 2  :                                                     // If in position 2 square
        wave_pointer = squareWave;                                   
        break;
      case 3  :                                                     // If in position 3 saw
        wave_pointer = sawtoothWave;                                 
        break;
      default :                                                     // Default to sine
        wave_pointer = sineWave;
    }
    currentWaveIsSelected = 1;                                                // As a wave has been selected set to 1
  }
  else if ( !(PINB & (1 << SELECT_WAVE_PIN)) && currentWaveIsSelected )       // if the wave selection button was released
  {
    currentWaveIsSelected = 0;                                                // A new waveform hasnt been selected
  }
}

void processMidiEvent (const MidiEvent* const midiEvent)                      //Deals with MIDI ON Events and MIDI OFF events
{
 //-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 //-----------------------------------------------------------------------------In the case of MIDI OFF message-----------------------------------------------------------------
 //-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  if ( (midiEvent->statusByte >> 4) == MIDI_NOTE_OFF || midiEvent->dataByte[1] == 0)      // Note OFF could have note off status byte or a MIDI ON event with velocity of 0
  {
    osc.notes_pressed -= 1;                                                               // Decrement the number of notes pressed. A key is released so number of notes pressed at same time is -= 1
    
    if (osc.notes_pressed == 0)                                                           // If no notes pressed
    {
      PORTB &= ~(1 << GATE_OUT_PIN);                                                      // FOR envelope generaton in analogue section. If no notes pressed gate output low 
    } else                                                                                // If notes are still being pressed so previous note still plays
    {
      osc.note = osc.previous_notes[osc.notes_pressed];                                   // Set current note to the previous note in the buffer
      osc.increment_table_proc = 1;                                                       // As using a previous note increment table needs reprocessing
    }
  }
 //-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 //-----------------------------------------------------------------------------In the case of MIDI ON message-----------------------------------------------------------------
 //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------- 
  else  // else not a note on message 
  {
    osc.note = midiEvent->dataByte[0];                                                      // set the midi note value of the Voice struct to the current midi note being processed
    osc.amplitude_val = round (midiEvent->dataByte[1] / 8);                                 // Gets amplitude value from the velocity data byte then /8 which as max of databyte is 128 max amplitude will be 16 so 12 bit data wont be overflowed which will be sent to DAC 
                                                                                            // which keeps within clipping range (ex. 255 * 16 = 4080 < 4095)
    osc.notes_pressed += 1;                                                                 // increase number of notes pressed by 1 so the number of notes pressed simultaneously is += 1
    osc.previous_notes[osc.notes_pressed] = midiEvent->dataByte[0];                         // Add MIDI note databyte to the previous notes array. Put this value into the previous note buffer to be processed if a key after this is released but this key is still pressed
    osc.increment_table_proc = 1;                                                           // Processing new note, increment_table needs reprocessing

    PORTB |= (1 << GATE_OUT_PIN);                                                           // FOR envelope generaton in analogue section. Sets gate output high as note is pressed. gate is low when a note is released
  }
}

void process_increment_table()                  //Integer value corresponds to a MIDI note e.g Note C0 = 0. MIDI.h contains all defines for each note
{
  float runningIncr = 0.0;                      // the running total for incrementing, to be rounded off and placed 
  float incr = 0.0;                             // the floating point value to increment by to achieve desired frequency
  
  switch (osc.note)                             // Takes current MIDI note and compares with every MIDI define to match the incr variable to the music note define
  {
    case MIDI_C0  :
      incr = MUSIC_C0;
      break;
    case MIDI_Db0  :
      incr = MUSIC_Db0;
      break;
    case MIDI_D0  :
      incr = MUSIC_D0;
      break;
    case MIDI_Eb0  :
      incr = MUSIC_Eb0;
      break;
    case MIDI_E0  :
      incr = MUSIC_E0;
      break;
    case MIDI_F0  :
      incr = MUSIC_F0;
      break;
    case MIDI_Gb0  :
      incr = MUSIC_Gb0;
      break;
    case MIDI_G0  :
      incr = MUSIC_G0;
      break;
    case MIDI_Ab0  :
      incr = MUSIC_Ab0;
      break;
    case MIDI_A0  :
      incr = MUSIC_A0;
      break;
    case MIDI_Bb0  :
      incr = MUSIC_Bb0;
      break;
    case MIDI_B0  :
      incr = MUSIC_B0;
      break;
    case MIDI_C1  :
      incr = MUSIC_C1;
      break;
    case MIDI_Db1  :
      incr = MUSIC_Db1;
      break;
    case MIDI_D1  :
      incr = MUSIC_D1;
      break;
    case MIDI_Eb1  :
      incr = MUSIC_Eb1;
      break;
    case MIDI_E1  :
      incr = MUSIC_E1;
      break;
    case MIDI_F1  :
      incr = MUSIC_F1;
      break;
    case MIDI_Gb1  :
      incr = MUSIC_Gb1;
      break;
    case MIDI_G1  :
      incr = MUSIC_G1;
      break;
    case MIDI_Ab1  :
      incr = MUSIC_Ab1;
      break;
    case MIDI_A1  :
      incr = MUSIC_A1;
      break;
    case MIDI_Bb1  :
      incr = MUSIC_Bb1;
      break;
    case MIDI_B1  :
      incr = MUSIC_B1;
      break;
    case MIDI_C2  :
      incr = MUSIC_C2;
      break;
    case MIDI_Db2  :
      incr = MUSIC_Db2;
      break;
    case MIDI_D2  :
      incr = MUSIC_D2;
      break;
    case MIDI_Eb2  :
      incr = MUSIC_Eb2;
      break;
    case MIDI_E2  :
      incr = MUSIC_E2;
      break;
    case MIDI_F2  :
      incr = MUSIC_F2;
      break;
    case MIDI_Gb2  :
      incr = MUSIC_Gb2;
      break;
    case MIDI_G2  :
      incr = MUSIC_G2;
      break;
    case MIDI_Ab2  :
      incr = MUSIC_Ab2;
      break;
    case MIDI_A2  :
      incr = MUSIC_A2;
      break;
    case MIDI_Bb2  :
      incr = MUSIC_Bb2;
      break;
    case MIDI_B2  :
      incr = MUSIC_B2;
      break;
    case MIDI_C3  :
      incr = MUSIC_C3;
      break;
    case MIDI_Db3  :
      incr = MUSIC_Db3;
      break;
    case MIDI_D3  :
      incr = MUSIC_D3;
      break;
    case MIDI_Eb3  :
      incr = MUSIC_Eb3;
      break;
    case MIDI_E3  :
      incr = MUSIC_E3;
      break;
    case MIDI_F3  :
      incr = MUSIC_F3;
      break;
    case MIDI_Gb3  :
      incr = MUSIC_Gb3;
      break;
    case MIDI_G3  :
      incr = MUSIC_G3;
      break;
    case MIDI_Ab3  :
      incr = MUSIC_Ab3;
      break;
    case MIDI_A3  :
      incr = MUSIC_A3;
      break;
    case MIDI_Bb3  :
      incr = MUSIC_Bb3;
      break;
    case MIDI_B3  :
      incr = MUSIC_B3;
      break;
    case MIDI_C4  :
      incr = MUSIC_C4;
      break;
    case MIDI_Db4  :
      incr = MUSIC_Db4;
      break;
    case MIDI_D4  :
      incr = MUSIC_D4;
      break;
    case MIDI_Eb4  :
      incr = MUSIC_Eb4;
      break;
    case MIDI_E4  :
      incr = MUSIC_E4;
      break;
    case MIDI_F4  :
      incr = MUSIC_F4;
      break;
    case MIDI_Gb4  :
      incr = MUSIC_Gb4;
      break;
    case MIDI_G4  :
      incr = MUSIC_G4;
      break;
    case MIDI_Ab4  :
      incr = MUSIC_Ab4;
      break;
    case MIDI_A4  :
      incr = MUSIC_A4;
      break;
    case MIDI_Bb4  :
      incr = MUSIC_Bb4;
      break;
    case MIDI_B4  :
      incr = MUSIC_B4;
      break;
    case MIDI_C5  :
      incr = MUSIC_C5;
      break;
    case MIDI_Db5  :
      incr = MUSIC_Db5;
      break;
    case MIDI_D5  :
      incr = MUSIC_D5;
      break;
    case MIDI_Eb5  :
      incr = MUSIC_Eb5;
      break;
    case MIDI_E5  :
      incr = MUSIC_E5;
      break;
    case MIDI_F5  :
      incr = MUSIC_F5;
      break;
    case MIDI_Gb5  :
      incr = MUSIC_Gb5;
      break;
    case MIDI_G5  :
      incr = MUSIC_G5;
      break;
    case MIDI_Ab5  :
      incr = MUSIC_Ab5;
      break;
    case MIDI_A5  :
      incr = MUSIC_A5;
      break;
    case MIDI_Bb5  :
      incr = MUSIC_Bb5;
      break;
    case MIDI_B5  :
      incr = MUSIC_B5;
      break;
    case MIDI_C6  :
      incr = MUSIC_C6;
      break;
    case MIDI_Db6  :
      incr = MUSIC_Db6;
      break;
    case MIDI_D6  :
      incr = MUSIC_D6;
      break;
    case MIDI_Eb6  :
      incr = MUSIC_Eb6;
      break;
    case MIDI_E6  :
      incr = MUSIC_E6;
      break;
    case MIDI_F6  :
      incr = MUSIC_F6;
      break;
    case MIDI_Gb6  :
      incr = MUSIC_Gb6;
      break;
    case MIDI_G6  :
      incr = MUSIC_G6;
      break;
    case MIDI_Ab6  :
      incr = MUSIC_Ab6;
      break;
    case MIDI_A6  :
      incr = MUSIC_A6;
      break;
    case MIDI_Bb6  :
      incr = MUSIC_Bb6;
      break;
    case MIDI_B6  :
      incr = MUSIC_B6;
      break;
    case MIDI_C7  :
      incr = MUSIC_C7;
      break;
    case MIDI_Db7  :
      incr = MUSIC_Db7;
      break;
    case MIDI_D7  :
      incr = MUSIC_D7;
      break;
    case MIDI_Eb7  :
      incr = MUSIC_Eb7;
      break;
    case MIDI_E7  :
      incr = MUSIC_E7;
      break;
    case MIDI_F7  :
      incr = MUSIC_F7;
      break;
    case MIDI_Gb7  :
      incr = MUSIC_Gb7;
      break;
    case MIDI_G7  :
      incr = MUSIC_G7;
      break;
    case MIDI_Ab7  :
      incr = MUSIC_Ab7;
      break;
    case MIDI_A7  :
      incr = MUSIC_A7;
      break;
    case MIDI_Bb7  :
      incr = MUSIC_Bb7;
      break;
    case MIDI_B7  :
      incr = MUSIC_B7;
      break;
    case MIDI_C8  :
      incr = MUSIC_C8;
      break;
    case MIDI_Db8  :
      incr = MUSIC_Db8;
      break;
    case MIDI_D8  :
      incr = MUSIC_D8;
      break;
    case MIDI_Eb8  :
      incr = MUSIC_Eb8;
      break;
    case MIDI_E8  :
      incr = MUSIC_E8;
      break;
    case MIDI_F8  :
      incr = MUSIC_F8;
      break;
    case MIDI_Gb8  :
      incr = MUSIC_Gb8;
      break;
    case MIDI_G8  :
      incr = MUSIC_G8;
      break;
    case MIDI_Ab8  :
      incr = MUSIC_Ab8;
      break;
    case MIDI_A8  :
      incr = MUSIC_A8;
      break;
    case MIDI_Bb8  :
      incr = MUSIC_Bb8;
      break;
    case MIDI_B8  :
      incr = MUSIC_B8;
      break;
    default :
      incr = MUSIC_C0;
  }

  for (int i = 0; i < INCREMENT_TABLE_SIZE; i++)
  {
    runningIncr += incr;                                                          // tracks the real floating point value of the incr
    osc.increment_table[i] = round (runningIncr) - round (runningIncr - incr);    // Get integer difference of rounded running total and last
  }

  osc.increment_table_proc = 0;                                                   //Set needs processing flag to 0 as it was just used
}

ISR (TIMER1_COMPA_vect)
{
  SPI_transmit ( wave_pointer[osc.table_position] * osc.amplitude_val );                                                     // output current wave at table position multiplied by amplitude value
  osc.table_position = ( osc.table_position + osc.increment_table[osc.increment_table_pos] ) % INITIAL_WAVE_TABLE_SIZE;      // update table position by adding value from the increment table % so it wraps around wave table
  osc.increment_table_pos = (osc.increment_table_pos + 1) % INCREMENT_TABLE_SIZE;                                            // add 1 to update the position of increment table % so it wraps around increment table
}
