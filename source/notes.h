#ifndef _NOTE_FREQ_
#define _NOTE_FREQ_

#include <stdbool.h>
#define C 8.18
#define Cs 8.66
#define D 9.18
#define Ds 9.72
#define E 10.30
#define F 10.91
#define Fs 11.56
#define G 12.25
#define Gs 12.98
#define A 13.75
#define As 14.57
#define B 15.43
#define C0 16.35
#define Cs0 17.32
#define D0 18.35
#define Ds0 19.45
#define E0 20.60
#define F0 21.83
#define Fs0 23.12
#define G0 24.50
#define Gs0 25.96
#define A0 27.50
#define As0 29.14
#define B0 30.87
#define C1 32.70
#define Cs1 34.65
#define D1 36.71
#define Ds1 38.89
#define E1 41.20
#define F1 43.65
#define Fs1 46.25
#define G1 49.00
#define Gs1 51.91
#define A1 55.00
#define As1 58.27
#define B1 61.74
#define C2 65.41
#define Cs2 69.30
#define D2 73.42
#define Ds2 77.78
#define E2 82.41
#define F2 87.31
#define Fs2 92.50
#define G2 98.00
#define Gs2 103.83
#define A2 110.00
#define As2 116.54
#define B2 123.47
#define C3 130.81
#define Cs3 138.59
#define D3 146.83
#define Ds3 155.56
#define E3 164.81
#define F3 174.61
#define Fs3 185.00
#define G3 196.00
#define Gs3 207.65
#define A3 220.00
#define As3 233.08
#define B3 246.94
#define C4 261.63
#define Cs4 277.18
#define D4 293.66
#define Ds4 311.13
#define E4 329.63
#define F4 349.23
#define Fs4 369.99
#define G4 392.00
#define Gs4 415.30
#define A4 440.00
#define As4 466.16
#define B4 493.88
#define C5 523.25
#define Cs5 554.37
#define D5 587.33
#define Ds5 622.25
#define E5 659.25
#define F5 698.46
#define Fs5 739.99
#define G5 783.99
#define Gs5 830.61
#define A5 880.00
#define As5 932.33
#define B5 987.77
#define C6 1046.50
#define Cs6 1108.73
#define D6 1174.66
#define Ds6 1244.51
#define E6 1318.51
#define F6 1396.91
#define Fs6 1479.98
#define G6 1567.98
#define Gs6 1661.22
#define A6 1760.00
#define As6 1864.66
#define B6 1975.53
#define C7 2093.00
#define Cs7 2217.46
#define D7 2349.32
#define Ds7 2489.02
#define E7 2637.02
#define F7 2793.83
#define Fs7 2959.96
#define G7 3135.96
#define Gs7 3322.44
#define A7 3520.00
#define As7 3729.31
#define B7 3951.07
#define C8 4186.01
#define Cs8 4434.92
#define D8 4698.63
#define Ds8 4978.03
#define E8 5274.04
#define F8 5587.65
#define Fs8 5919.91
#define G8 6271.93
#define Gs8 6644.88
#define A8 7040.00
#define As8 7458.62
#define B8 7902.13
#define C9 8372.02
#define Cs9	8869.84
#define D9 9397.27
#define Ds9 9956.06
#define E9 10548.08
#define F9 11175.30
#define Fs9 11839.82
#define G9 12543.85
//end of midi tuning range
/*
float midi[127];
int a = 440; // a is 440 hz...
for (int x = 0; x < 127; ++x)
{
   midi[x] = (a / 32) * (2 ^ ((x - 9) / 12));
}
*/
#endif
