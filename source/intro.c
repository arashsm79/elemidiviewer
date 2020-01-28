#include <stdio.h>
#include <stdlib.h>
#include "intro.h"
#include "beep.h"
#include "notes.h"
#include <string.h>

#define STRING_SIZE 4
#define INCREASE_SIZE 50


//function prototypes
float convertNameToFrequency(char *n);
void playNotes(Note *notesArray, size_t size);

void playIntro(void)
{
    //Open the file containing the intro song
    FILE *fPtr;
    fPtr = fopen("intro-song.txt", "r");

    //Defining strings used to collect garbage values such as indicators (-n, -N, -d)
    char newNoteIndicator[STRING_SIZE];
    char noteNameIndicator[STRING_SIZE];
    char noteLegnthIndicator[STRING_SIZE];

    //Defining note legnth and note name and note frequency
    int noteLegnth;
    char noteName[STRING_SIZE];
    int noteFrequency;

    //getting the first note
    fscanf(fPtr, "%2s%3s%2s%d", noteNameIndicator, noteName, noteLegnthIndicator, &noteLegnth);

    //defining an array of notes using dynamic memory management
    Note *notes;
    //the number of elements at actually has
    int notesCount = 0;
    //the number of elemets our array can store (will increase if the array fills up)
    int notesCountBuffer = 30;
    // initializing the first note
    notes = calloc(notesCountBuffer, sizeof(Note));
    (notes)->frequency = convertNameToFrequency(noteName);
    (notes)->length = noteLegnth;
    notesCount += 1;

    //a dummy pointer to check whether realloc has worked currectly or not
    void *dummyPtr = notes;
    
    while(!feof(fPtr))
    {
        fscanf(fPtr, "%2s%2s%3s%2s%d",newNoteIndicator, noteNameIndicator, noteName, noteLegnthIndicator, &noteLegnth);

        //using realloc to allocate more memory if our array fills up
        if(notesCount == notesCountBuffer)
        {
            notesCountBuffer += INCREASE_SIZE;
            dummyPtr = realloc(notes, (notesCountBuffer) * sizeof(Note));
        }
        if(dummyPtr != NULL)
        {
            notes = dummyPtr;
            (notes + notesCount)->frequency = convertNameToFrequency(noteName);
            (notes + notesCount)->length = noteLegnth;
            
            notesCount += 1;
        }

    }


    // pass the array of notes to playnote function
    playNotes(notes, notesCount);
    
    // freeing up memory and closing the file
    free(notes);
    fclose(fPtr);
    
}

// a simple function to iterate through the array of notes and play them
void playNotes(Note *notesArray, size_t size)
{
    for(int i = 0; i < size; i += 1)
        {
            beep((notesArray + i)->frequency, (notesArray + i)->length + 130);
        }
}


// converting the given note name to its frequency using the header "notes.h"
//could've used "reflections" but since the program must be pure C (not c++), did this :D
float convertNameToFrequency(char *n)
{
    if(strstr(n, "0") != NULL)
    { 
        if(strcmp(n, "C0") == 0)
            return C0;
        else if(strcmp(n, "C#0") == 0)
            return Cs0;
        else if(strcmp(n, "D0") == 0)
            return D0;
        else if(strcmp(n, "D#0") == 0)
            return Ds0;
        else if(strcmp(n, "E0") == 0)
            return E0;
        else if(strcmp(n, "F0") == 0)
            return F0;
        else if(strcmp(n, "F#0") == 0)
            return Fs0;
        else if(strcmp(n, "G0") == 0)
            return G0;
        else if(strcmp(n, "G#0") == 0)
            return Gs0;
        else if(strcmp(n, "A0") == 0)
            return A0;
        else if(strcmp(n, "A#0") == 0)
            return As0;
        else if(strcmp(n, "B0") == 0)
            return B0;
    }else if(strstr(n, "1") != NULL)
    {
        if(strcmp(n, "C1") == 0)
            return C1;
        else if(strcmp(n, "C#1") == 0)
            return Cs1;
        else if(strcmp(n, "D1") == 0)
            return D1;
        else if(strcmp(n, "D#1") == 0)
            return Ds1;
        else if(strcmp(n, "E1") == 0)
            return E1;
        else if(strcmp(n, "F1") == 0)
            return F1;
        else if(strcmp(n, "F#1") == 0)
            return Fs1;
        else if(strcmp(n, "G1") == 0)
            return G1;
        else if(strcmp(n, "G#1") == 0)
            return Gs1;
        else if(strcmp(n, "A1") == 0)
            return A1;
        else if(strcmp(n, "A#1") == 0)
            return As1;
        else if(strcmp(n, "B1") == 0)
            return B1;

    }else if(strstr(n, "2") != NULL)
    {
        if(strcmp(n, "C2") == 0)
            return C2;
        else if(strcmp(n, "C#2") == 0)
            return Cs2;
        else if(strcmp(n, "D2") == 0)
            return D2;
        else if(strcmp(n, "D#2") == 0)
            return Ds2;
        else if(strcmp(n, "E2") == 0)
            return E2;
        else if(strcmp(n, "F2") == 0)
            return F2;
        else if(strcmp(n, "F#2") == 0)
            return Fs2;
        else if(strcmp(n, "G2") == 0)
            return G2;
        else if(strcmp(n, "G#2") == 0)
            return Gs2;
        else if(strcmp(n, "A2") == 0)
            return A2;
        else if(strcmp(n, "A#2") == 0)
            return As2;
        else if(strcmp(n, "B2") == 0)
            return B2;
    }else if(strstr(n, "3") != NULL)
    {
        if(strcmp(n, "C3") == 0)
            return C3;
        else if(strcmp(n, "C#3") == 0)
            return Cs3;
        else if(strcmp(n, "D3") == 0)
            return D3;
        else if(strcmp(n, "D#3") == 0)
            return Ds3;
        else if(strcmp(n, "E3") == 0)
            return E3;
        else if(strcmp(n, "F3") == 0)
            return F3;
        else if(strcmp(n, "F#3") == 0)
            return Fs3;
        else if(strcmp(n, "G3") == 0)
            return G3;
        else if(strcmp(n, "G#3") == 0)
            return Gs3;
        else if(strcmp(n, "A3") == 0)
            return A3;
        else if(strcmp(n, "A#3") == 0)
            return As3;
        else if(strcmp(n, "B3") == 0)
            return B3;
    }else if(strstr(n, "4") != NULL)
    {
        if(strcmp(n, "C4") == 0)
            return C4;
        else if(strcmp(n, "C#4") == 0)
            return Cs4;
        else if(strcmp(n, "D4") == 0)
            return D4;
        else if(strcmp(n, "D#4") == 0)
            return Ds4;
        else if(strcmp(n, "E4") == 0)
            return E4;
        else if(strcmp(n, "F4") == 0)
            return F4;
        else if(strcmp(n, "F#4") == 0)
            return Fs4;
        else if(strcmp(n, "G4") == 0)
            return G4;
        else if(strcmp(n, "G#4") == 0)
            return Gs4;
        else if(strcmp(n, "A4") == 0)
            return A4;
        else if(strcmp(n, "A#4") == 0)
            return As4;
        else if(strcmp(n, "B4") == 0)
            return B4;   
    }else if(strstr(n, "5") != NULL)
    {
        if(strcmp(n, "C5") == 0)
            return C5;
        else if(strcmp(n, "C#5") == 0)
            return Cs5;
        else if(strcmp(n, "D5") == 0)
            return D5;
        else if(strcmp(n, "D#5") == 0)
            return Ds5;
        else if(strcmp(n, "E5") == 0)
            return E5;
        else if(strcmp(n, "F5") == 0)
            return F5;
        else if(strcmp(n, "F#5") == 0)
            return Fs5;
        else if(strcmp(n, "G5") == 0)
            return G5;
        else if(strcmp(n, "G#5") == 0)
            return Gs5;
        else if(strcmp(n, "A5") == 0)
            return A5;
        else if(strcmp(n, "A#5") == 0)
            return As5;
        else if(strcmp(n, "B5") == 0)
            return B5;   
    }else if(strstr(n, "6") != NULL)
    {
        if(strcmp(n, "C6") == 0)
            return C6;
        else if(strcmp(n, "C#6") == 0)
            return Cs6;
        else if(strcmp(n, "D6") == 0)
            return D6;
        else if(strcmp(n, "D#6") == 0)
            return Ds6;
        else if(strcmp(n, "E6") == 0)
            return E6;
        else if(strcmp(n, "F6") == 0)
            return F6;
        else if(strcmp(n, "F#6") == 0)
            return Fs6;
        else if(strcmp(n, "G6") == 0)
            return G6;
        else if(strcmp(n, "G#6") == 0)
            return Gs6;
        else if(strcmp(n, "A6") == 0)
            return A6;
        else if(strcmp(n, "A#6") == 0)
            return As6;
        else if(strcmp(n, "B6") == 0)
            return B6; 
    }else if(strstr(n, "7") != NULL)
    {
        if(strcmp(n, "C7") == 0)
            return C7;
        else if(strcmp(n, "C#7") == 0)
            return Cs7;
        else if(strcmp(n, "D7") == 0)
            return D7;
        else if(strcmp(n, "D#7") == 0)
            return Ds7;
        else if(strcmp(n, "E7") == 0)
            return E7;
        else if(strcmp(n, "F7") == 0)
            return F7;
        else if(strcmp(n, "F#7") == 0)
            return Fs7;
        else if(strcmp(n, "G7") == 0)
            return G7;
        else if(strcmp(n, "G#7") == 0)
            return Gs7;
        else if(strcmp(n, "A7") == 0)
            return A7;
        else if(strcmp(n, "A#7") == 0)
            return As7;
        else if(strcmp(n, "B7") == 0)
            return B7;
    }else if(strstr(n, "8") != NULL)
    {
        if(strcmp(n, "C8") == 0)
            return C8;
        else if(strcmp(n, "C#8") == 0)
            return Cs8;
        else if(strcmp(n, "D8") == 0)
            return D8;
        else if(strcmp(n, "D#8") == 0)
            return Ds8;
        else if(strcmp(n, "E8") == 0)
            return E8;
        else if(strcmp(n, "F8") == 0)
            return F8;
        else if(strcmp(n, "F#8") == 0)
            return Fs8;
        else if(strcmp(n, "G8") == 0)
            return G8;
        else if(strcmp(n, "G#8") == 0)
            return Gs8;
        else if(strcmp(n, "A8") == 0)
            return A8;
        else if(strcmp(n, "A#8") == 0)
            return As8;
        else if(strcmp(n, "B8") == 0)
            return B8; 
    }
        
    
}