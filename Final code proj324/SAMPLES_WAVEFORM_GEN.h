#include <math.h>

#define INITIAL_WAVE_FREQ 100                              // Frequency waveforms will be sampled at. One cycle of each waveform will be stored in the buffers at this frequency in Hz
#define INITIAL_WAVE_TABLE_SIZE (Fs / INITIAL_WAVE_FREQ)   // The size of one waveform in the wave buffer.(16000/100) = 160 bytes in length

// correct floating point increment for each musical frequency (freq / sample rate gives us cycles per sample
// but then we have to multiply by the initial wave table size since 1 cycle is now (sample rate / initial wave frequency) samples
#define MUSIC_C0 INITIAL_WAVE_TABLE_SIZE * (16.35 / Fs)
#define MUSIC_Db0 INITIAL_WAVE_TABLE_SIZE * (17.32 / Fs)
#define MUSIC_D0 INITIAL_WAVE_TABLE_SIZE * (18.35 / Fs)
#define MUSIC_Eb0 INITIAL_WAVE_TABLE_SIZE * (19.45 / Fs)
#define MUSIC_E0 INITIAL_WAVE_TABLE_SIZE * (20.60 / Fs)
#define MUSIC_F0 INITIAL_WAVE_TABLE_SIZE * (21.83 / Fs)
#define MUSIC_Gb0 INITIAL_WAVE_TABLE_SIZE * (23.12 / Fs)
#define MUSIC_G0 INITIAL_WAVE_TABLE_SIZE * (24.50 / Fs)
#define MUSIC_Ab0 INITIAL_WAVE_TABLE_SIZE * (25.96 / Fs)
#define MUSIC_A0 INITIAL_WAVE_TABLE_SIZE * (27.50 / Fs)
#define MUSIC_Bb0 INITIAL_WAVE_TABLE_SIZE * (29.14 / Fs)
#define MUSIC_B0 INITIAL_WAVE_TABLE_SIZE * (30.87 / Fs)
#define MUSIC_C1 INITIAL_WAVE_TABLE_SIZE * (32.70 / Fs)
#define MUSIC_Db1 INITIAL_WAVE_TABLE_SIZE * (34.65 / Fs)
#define MUSIC_D1 INITIAL_WAVE_TABLE_SIZE * (36.71 / Fs)
#define MUSIC_Eb1 INITIAL_WAVE_TABLE_SIZE * (38.89 / Fs)
#define MUSIC_E1 INITIAL_WAVE_TABLE_SIZE * (41.20 / Fs)
#define MUSIC_F1 INITIAL_WAVE_TABLE_SIZE * (43.65 / Fs)
#define MUSIC_Gb1 INITIAL_WAVE_TABLE_SIZE * (46.25 / Fs)
#define MUSIC_G1 INITIAL_WAVE_TABLE_SIZE * (49.00 / Fs)
#define MUSIC_Ab1 INITIAL_WAVE_TABLE_SIZE * (51.91 / Fs)
#define MUSIC_A1 INITIAL_WAVE_TABLE_SIZE * (55.00 / Fs)
#define MUSIC_Bb1 INITIAL_WAVE_TABLE_SIZE * (58.27 / Fs)
#define MUSIC_B1 INITIAL_WAVE_TABLE_SIZE * (61.74 / Fs)
#define MUSIC_C2 INITIAL_WAVE_TABLE_SIZE * (65.41 / Fs)
#define MUSIC_Db2 INITIAL_WAVE_TABLE_SIZE * (69.30 / Fs)
#define MUSIC_D2 INITIAL_WAVE_TABLE_SIZE * (73.42 / Fs)
#define MUSIC_Eb2 INITIAL_WAVE_TABLE_SIZE * (77.78 / Fs)
#define MUSIC_E2 INITIAL_WAVE_TABLE_SIZE * (82.41 / Fs)
#define MUSIC_F2 INITIAL_WAVE_TABLE_SIZE * (87.31 / Fs)
#define MUSIC_Gb2 INITIAL_WAVE_TABLE_SIZE * (92.50 / Fs)
#define MUSIC_G2 INITIAL_WAVE_TABLE_SIZE * (98.00 / Fs)
#define MUSIC_Ab2 INITIAL_WAVE_TABLE_SIZE * (103.83 / Fs)
#define MUSIC_A2 INITIAL_WAVE_TABLE_SIZE * (110.00 / Fs)
#define MUSIC_Bb2 INITIAL_WAVE_TABLE_SIZE * (116.54 / Fs)
#define MUSIC_B2 INITIAL_WAVE_TABLE_SIZE * (123.47 / Fs)
#define MUSIC_C3 INITIAL_WAVE_TABLE_SIZE * (130.81 / Fs)
#define MUSIC_Db3 INITIAL_WAVE_TABLE_SIZE * (138.59 / Fs)
#define MUSIC_D3 INITIAL_WAVE_TABLE_SIZE * (146.83 / Fs)
#define MUSIC_Eb3 INITIAL_WAVE_TABLE_SIZE * (155.56 / Fs)
#define MUSIC_E3 INITIAL_WAVE_TABLE_SIZE * (164.81 / Fs)
#define MUSIC_F3 INITIAL_WAVE_TABLE_SIZE * (174.61 / Fs)
#define MUSIC_Gb3 INITIAL_WAVE_TABLE_SIZE * (185.00 / Fs)
#define MUSIC_G3 INITIAL_WAVE_TABLE_SIZE * (196.00 / Fs)
#define MUSIC_Ab3 INITIAL_WAVE_TABLE_SIZE * (207.65 / Fs)
#define MUSIC_A3 INITIAL_WAVE_TABLE_SIZE * (220.00 / Fs)
#define MUSIC_Bb3 INITIAL_WAVE_TABLE_SIZE * (233.08 / Fs)
#define MUSIC_B3 INITIAL_WAVE_TABLE_SIZE * (246.94 / Fs)
#define MUSIC_C4 INITIAL_WAVE_TABLE_SIZE * (261.63 / Fs)
#define MUSIC_Db4 INITIAL_WAVE_TABLE_SIZE * (277.18 / Fs)
#define MUSIC_D4 INITIAL_WAVE_TABLE_SIZE * (293.66 / Fs)
#define MUSIC_Eb4 INITIAL_WAVE_TABLE_SIZE * (311.13 / Fs)
#define MUSIC_E4 INITIAL_WAVE_TABLE_SIZE * (329.63 / Fs)
#define MUSIC_F4 INITIAL_WAVE_TABLE_SIZE * (349.23 / Fs)
#define MUSIC_Gb4 INITIAL_WAVE_TABLE_SIZE * (369.99 / Fs)
#define MUSIC_G4 INITIAL_WAVE_TABLE_SIZE * (392.00 / Fs)
#define MUSIC_Ab4 INITIAL_WAVE_TABLE_SIZE * (415.30 / Fs)
#define MUSIC_A4 INITIAL_WAVE_TABLE_SIZE * (440.00 / Fs)
#define MUSIC_Bb4 INITIAL_WAVE_TABLE_SIZE * (466.16 / Fs)
#define MUSIC_B4 INITIAL_WAVE_TABLE_SIZE * (493.88 / Fs)
#define MUSIC_C5 INITIAL_WAVE_TABLE_SIZE * (523.25 / Fs)
#define MUSIC_Db5 INITIAL_WAVE_TABLE_SIZE * (554.37 / Fs)
#define MUSIC_D5 INITIAL_WAVE_TABLE_SIZE * (587.33 / Fs)
#define MUSIC_Eb5 INITIAL_WAVE_TABLE_SIZE * (622.25 / Fs)
#define MUSIC_E5 INITIAL_WAVE_TABLE_SIZE * (659.25 / Fs)
#define MUSIC_F5 INITIAL_WAVE_TABLE_SIZE * (698.46 / Fs)
#define MUSIC_Gb5 INITIAL_WAVE_TABLE_SIZE * (739.99 / Fs)
#define MUSIC_G5 INITIAL_WAVE_TABLE_SIZE * (783.99 / Fs)
#define MUSIC_Ab5 INITIAL_WAVE_TABLE_SIZE * (830.61 / Fs)
#define MUSIC_A5 INITIAL_WAVE_TABLE_SIZE * (880.00 / Fs)
#define MUSIC_Bb5 INITIAL_WAVE_TABLE_SIZE * (932.33 / Fs)
#define MUSIC_B5 INITIAL_WAVE_TABLE_SIZE * (987.77 / Fs)
#define MUSIC_C6 INITIAL_WAVE_TABLE_SIZE * (1046.50 / Fs)
#define MUSIC_Db6 INITIAL_WAVE_TABLE_SIZE * (1108.73 / Fs)
#define MUSIC_D6 INITIAL_WAVE_TABLE_SIZE * (1174.66 / Fs)
#define MUSIC_Eb6 INITIAL_WAVE_TABLE_SIZE * (1244.51 / Fs)
#define MUSIC_E6 INITIAL_WAVE_TABLE_SIZE * (1318.51 / Fs)
#define MUSIC_F6 INITIAL_WAVE_TABLE_SIZE * (1396.91 / Fs)
#define MUSIC_Gb6 INITIAL_WAVE_TABLE_SIZE * (1479.98 / Fs)
#define MUSIC_G6 INITIAL_WAVE_TABLE_SIZE * (1567.98 / Fs)
#define MUSIC_Ab6 INITIAL_WAVE_TABLE_SIZE * (1661.22 / Fs)
#define MUSIC_A6 INITIAL_WAVE_TABLE_SIZE * (1760.00 / Fs)
#define MUSIC_Bb6 INITIAL_WAVE_TABLE_SIZE * (1864.66 / Fs)
#define MUSIC_B6 INITIAL_WAVE_TABLE_SIZE * (1975.53 / Fs)
#define MUSIC_C7 INITIAL_WAVE_TABLE_SIZE * (2093.00 / Fs)
#define MUSIC_Db7 INITIAL_WAVE_TABLE_SIZE * (2217.46 / Fs)
#define MUSIC_D7 INITIAL_WAVE_TABLE_SIZE * (2349.32 / Fs)
#define MUSIC_Eb7 INITIAL_WAVE_TABLE_SIZE * (2489.02 / Fs)
#define MUSIC_E7 INITIAL_WAVE_TABLE_SIZE * (2637.02 / Fs)
#define MUSIC_F7 INITIAL_WAVE_TABLE_SIZE * (2793.83 / Fs)
#define MUSIC_Gb7 INITIAL_WAVE_TABLE_SIZE * (2959.96 / Fs)
#define MUSIC_G7 INITIAL_WAVE_TABLE_SIZE * (3135.96 / Fs)
#define MUSIC_Ab7 INITIAL_WAVE_TABLE_SIZE * (3322.44 / Fs)
#define MUSIC_A7 INITIAL_WAVE_TABLE_SIZE * (3520.00 / Fs)
#define MUSIC_Bb7 INITIAL_WAVE_TABLE_SIZE * (3729.31 / Fs)
#define MUSIC_B7 INITIAL_WAVE_TABLE_SIZE * (3951.07 / Fs)
#define MUSIC_C8 INITIAL_WAVE_TABLE_SIZE * (4186.01 / Fs)
#define MUSIC_Db8 INITIAL_WAVE_TABLE_SIZE * (4434.92 / Fs)
#define MUSIC_D8 INITIAL_WAVE_TABLE_SIZE * (4698.63 / Fs)
#define MUSIC_Eb8 INITIAL_WAVE_TABLE_SIZE * (4978.03 / Fs)
#define MUSIC_E8 INITIAL_WAVE_TABLE_SIZE * (5274.04 / Fs)
#define MUSIC_F8 INITIAL_WAVE_TABLE_SIZE * (5587.65 / Fs)
#define MUSIC_Gb8 INITIAL_WAVE_TABLE_SIZE * (5919.91 / Fs)
#define MUSIC_G8 INITIAL_WAVE_TABLE_SIZE * (6271.93 / Fs)
#define MUSIC_Ab8 INITIAL_WAVE_TABLE_SIZE * (6644.88 / Fs)
#define MUSIC_A8 INITIAL_WAVE_TABLE_SIZE * (7040.00 / Fs)
#define MUSIC_Bb8 INITIAL_WAVE_TABLE_SIZE * (7458.62 / Fs)
#define MUSIC_B8 INITIAL_WAVE_TABLE_SIZE * (7902.13 / Fs)

uint8_t waveBuffer[INITIAL_WAVE_TABLE_SIZE * 4];                            // The waveBuffer holds all 4 waveforms of INITIAL_WAVE_FREQ

//Pointers to each waveform in the buffer
uint8_t* sineWave = &waveBuffer[0];                                         // Pointer to beginning of sine waveform
uint8_t* triangleWave = &waveBuffer[INITIAL_WAVE_TABLE_SIZE];               // Pointer to beginning of triangle waveform
uint8_t* squareWave = &waveBuffer[INITIAL_WAVE_TABLE_SIZE * 2];             // Pointer to beginning of square waveform
uint8_t* sawtoothWave = &waveBuffer[INITIAL_WAVE_TABLE_SIZE * 3];           // Pointer to beginning of sawtooth waveform
uint8_t* wave_pointer = sineWave;                                           // Current pointer, to begin with point at sine


//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------  SINE WAVE  ---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// sine wave formula y(t) = Asin(2piFt + p)  
// A = amp, F = freq, t = time, p = phase

void generateSine ()
{
  const float cyclesPerSample = ((float)INITIAL_WAVE_FREQ / (float)Fs);                                // Convert freq to cycles per sample as Frequency concerned with, instead of cycles per second (Hz) 

  for (int i = 0; i < INITIAL_WAVE_TABLE_SIZE; i++)                                                         // Loop through all of waveform
  {
    sineWave[i] = round ( (255.0/2) * (1.0 + sin (2.0 * M_PI * cyclesPerSample * i + (3* M_PI) / 2)) );     // sinewave = Asin(2 * pi * freq * time + phase), with freq in cyclesPerSample
                                                                                                            // 1.0 makes it people a 2 and low at 0 instead of 1 and -1, however it would make it start at 1 instead of 0
                                                                                                            // To make it start at 0 phase is changed (3pi/2) added so the signal starts at 0 instead of 128, 
                                                                                                            // which would make popping sound when key first pressed
                                                                                                            // multiplying by 255/2 scales sinewave so it can peak at 255 instead of 2
  }                                                                                                         // round function ensures number is integer (<0.5 round down >0.5 up)
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------  TRIANGLE WAVE  -------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void generateTriangle ()
{
  const float incr = 255.0 / (INITIAL_WAVE_TABLE_SIZE / 2.0);       // the amount to increment or decrement for each sample /2 to come back down after reaching top value
  const int decrPoint = ceil (INITIAL_WAVE_TABLE_SIZE / 2.0);       // This defines when to decrement instead of incrementing to achieve triangle waveform (/2 as its halfway through waveform at the peak of triangle wave)
                                                                    // ceiling used as dont want to underflow the integer
  triangleWave[0] = 0;                                              // Sets first value of tri to 0
  
  float tempIncr = 0;                                               // Temporary variable to keep track of the correct floating point value of triangleWave[i]
  for (int i = 1; i < INITIAL_WAVE_TABLE_SIZE; i++)                 // Loop through whole wave
  {
    if (i <= decrPoint)                                             //If before halfway
    {
      tempIncr += incr;                                             //Increment 
      triangleWave[i] = floor (tempIncr);                           //Set value to floating-point value representing the nearest whole number that is less than or equal to the value
    }
    else
    {
      tempIncr -= incr;                                             //decrement 
      triangleWave[i] = floor (tempIncr);                           //Set value to floating-point value representing the nearest whole number that is less than or equal to the value
    }
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------  SQUARE WAVE  ---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void generateSquare ()
{
  const int flipPoint = ceil(INITIAL_WAVE_TABLE_SIZE / 2.0);    // at this point in the samples, output low instead of high to achieve square waveform (Puts output in the centre)
  
  for (int i = 0; i < INITIAL_WAVE_TABLE_SIZE; i++)             //Loop from 0 to initial wavetable size
  {
    if (i < flipPoint)
    {
      squareWave[i] = 255;                                      // Before getting to flip point output 255 (highest amount for unsigned int possible)
    }
    else
    {
      squareWave[i] = 0;                                        // At flip point output low
    }
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------  SAWTOOTH WAVE  -------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void generateSawtooth ()                                  //Sawtooth is basically just a ramp
{
  const float incr = (255.0 / INITIAL_WAVE_TABLE_SIZE);   //At end of iteration will reach 255
  
  for (int i = 0; i < INITIAL_WAVE_TABLE_SIZE; i++)       //Loop from 0 to initial wavetable size
  {
    sawtoothWave[i] = incr * i;                           //Gives ramp characteristic
  }
}
