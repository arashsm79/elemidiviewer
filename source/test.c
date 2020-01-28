#include <math.h>

float turnMidiNoteNumberToFrequency(unsigned int noteNumber)
{
    int a = 440; 
    
    return (a / 32) * (pow(2, (noteNumber - 9)) / 12);
    
}
int main()
{
    float x = turnMidiNoteNumberToFrequency(120);
    int g = 2;
}