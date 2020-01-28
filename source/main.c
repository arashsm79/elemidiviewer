#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "beep.h"
#include "intro.h"
#include "midi.h"



int main()
{
    unsigned int readVariableLengthValue(FILE *midiFile);
    MidiEvent *readTrackEvents(FILE *midiFile, size_t *numberOfEvents, size_t *numberOfNotes);
    void ei_ui_fread(unsigned int *ptr, size_t size, size_t n, FILE *streamPtr);
    Note *composeNotes(MidiEvent *midiEvents, size_t numberOfEvents, size_t numberOfNotes, MidiCurrentStatus *);

    // _+_+_+_+_+( PHASE I )+_+_+_+_+_
    // Play the intro song using the intro.c source file 
    //playIntro();
    
    //open a midi file
    FILE *midiFile = fopen("music.mid", "rb");
    if(midiFile == NULL)
    {
        printf("Erorr Opening File!\n");
        return 1;
    }


    // _+_+_+_+_+( PHASE II )+_+_+_+_+_
    //print the midi file's header
    MidiHeader midiHeader; 
    printMidiHeader(&midiHeader, midiFile);

    //setting the read data to  a midi current status struct
    //the default tempo is 120BPM that translates to 60000000 / 120 = 500000 microseconds per quarter note
    MidiCurrentStatus midiCurrentStatus;
    if(midiHeader.devisionType == 0)
    {
        midiCurrentStatus.ticksPerQuarterNote = midiHeader.ticksPerQuarterNote;
    }else
    {
        midiCurrentStatus.framePerSecond = midiHeader.framePerSecond;
        midiCurrentStatus.ticksPerFrame = midiHeader.ticksPerFrame;

    }
    //tempo is in terms of microseconds per quarter note
    midiCurrentStatus.tempo = 60000000 / 120;

	//microseconds per tick which is tempo / ticksPerQuarterNote 
    midiCurrentStatus.msPerTick = midiCurrentStatus.tempo / midiCurrentStatus.ticksPerQuarterNote;
    

    // _+_+_+_+_+( PHASE III )+_+_+_+_+_
    //initialize the tracks and populate them with events
    MidiTrack *tracks = calloc(midiHeader.tracks, sizeof(MidiTrack));
    NoteSequence *noteSequence = calloc(midiHeader.tracks, sizeof(NoteSequence));

    for(int i = 0; i < midiHeader.tracks; i++)
    {
        char chunkID[4];
        //read 32 bits
        fread(&chunkID, 1, 4, midiFile);

        unsigned int trackSize;
        ei_ui_fread(&trackSize, sizeof(char), 4, midiFile);

        //reads all of the contents of the track
        tracks[i].midiEvents = readTrackEvents(midiFile, &(tracks[i].numberOfEvents), &(noteSequence[i].numberOfNotes));


        //creates an array of notes using note on and note off events
        noteSequence[i].notes = composeNotes(tracks[i].midiEvents, tracks[i].numberOfEvents, noteSequence[i].numberOfNotes, &midiCurrentStatus);
    }
    



    // _+_+_+_+_+( PHASE IV )+_+_+_+_+_
    for(int i = 0; i < midiHeader.tracks; i++)
    {
        playNotes(noteSequence[i].notes, noteSequence[i].numberOfNotes);
    }

    //close the midi file stream
    fclose(midiFile);

}
////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////

//this functions matches note of events with their corresponding note on events and alculates their length and returns an array of notes 
Note *composeNotes(MidiEvent *midiEvents, size_t numberOfEvents, size_t numberOfNotes, MidiCurrentStatus *midiCurrentStatus)
{
    //prototypes
    float turnMidiNoteNumberToFrequency(unsigned int);
    void findCorrespondingNoteOn(Note *noteArray, size_t noteCount, unsigned int noteNumber, unsigned int playTime, MidiCurrentStatus *);
    void ei_ui_byteArray(unsigned int *ptr, size_t dataSize, unsigned char *data);

    Note *noteArray = calloc(numberOfNotes, sizeof(Note));
    size_t noteCount = 0;

    // the elapsed time since the first event in ticks
    unsigned int playTime;

    for(int i = 0; i < numberOfEvents && noteCount <= numberOfNotes; i++)
    {
        //ad the current events delta time to total Playtime
        playTime += midiEvents[i].deltaTime;

        //control statements for handling midi events
        switch (midiEvents[i].eventType)
        {
        //Meta Event
        case META_EVENT:
            switch (midiEvents[i].metaEvent.specificEventType)
            {
            case SET_TEMPO:
                ;
                //endian independently set the 3 byte data of previously read tempo as the current tempo
                unsigned int newTempo = 0;

             
                ei_ui_byteArray(&newTempo, midiEvents[i].metaEvent.length, midiEvents[i].metaEvent.data);


                //  newTempo = (midiEvents[i].metaEvent.data[2]<<0) | (midiEvents[i].metaEvent.data[1]<<8) | (midiEvents[i].metaEvent.data[0]<<16);
                midiCurrentStatus->tempo = newTempo;
                midiCurrentStatus->msPerTick = midiCurrentStatus->tempo / midiCurrentStatus->ticksPerQuarterNote; 
                break;
            
            default:
                break;
            }
            break;

        //Midi channel event
        case MIDI_CHANNEL_EVENT:
            switch (midiEvents[i].channelEvent.specificEventType)
            {
            case NOTE_ON:
                noteArray[noteCount].frequency = turnMidiNoteNumberToFrequency(midiEvents[i].channelEvent.param1);
                noteArray[noteCount].midiNoteNumber = midiEvents[i].channelEvent.param1;
                noteArray[noteCount].playTime = playTime;
                noteArray[noteCount].delay = (midiEvents[i].deltaTime * midiCurrentStatus->msPerTick) / 1000;
                noteCount++;
                break;
            case NOTE_OFF:
                findCorrespondingNoteOn(noteArray, noteCount, midiEvents[i].channelEvent.param1, playTime, midiCurrentStatus);
                break;
            case POLY_AFTERTOUCH:
                /* code */
                break;
            case CONTROL_CHANGE:
                /* code */
                break;
            case PROGRAM_CHNG:
                /* code */
                break;
            case CHANNEL_AFTERTOUCH:
                /* code */
                break;
            case PITCH_BEND:
                /* code */
                break;
            default:
                break;
            }
            break;

        //System exclusive event
        case SYSEX_EVENT:
            
            break;
        
        default:
            break;
        }
    }
    return noteArray;
}

////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////

void findCorrespondingNoteOn(Note *noteArray, size_t noteCount, unsigned int noteNumber, unsigned int playTime, MidiCurrentStatus * midiCurrentStatus)
{
    for(int i = noteCount - 1; i >= 0; i--)
    {
        if(noteArray[i].midiNoteNumber == noteNumber)
        {
            noteArray[i].length = ((playTime - noteArray[i].playTime) * midiCurrentStatus->msPerTick) / 1000;
            return;
        }
    }
}

////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////


MidiEvent *readTrackEvents(FILE *midiFile, size_t *numberOfEvents, size_t *numberOfNotes)
{
    //prototypes
    unsigned int readVariableLengthValue(FILE *midiFile);
    void ei_ui_fread(unsigned int *ptr, size_t size, size_t n, FILE *streamPtr);

    //a counter for the number of note-on events which dont have 0 velocity
    size_t noteCount = 0;

    size_t trackEventCount = 0;
    //we'll dynamically increase the buffer if it gets filled up
    size_t trackEventCountBuffer = 100;
    MidiEvent *midiEvents = calloc(trackEventCountBuffer, sizeof(MidiEvent));



    //read delta time
    unsigned int deltaTime = readVariableLengthValue(midiFile);
    midiEvents[trackEventCount].deltaTime = deltaTime;

    unsigned int eventCode;
    unsigned int previousEventCode; //for running status
    //read one byte
    ei_ui_fread(&eventCode, 1, 1, midiFile);


    /*This meta event associates a MIDI channel with following meta events. It's effect is terminated by another MIDI Channel Prefix event or any non- Meta event.
    It is often used before an Instrument Name Event to specify which channel an instrument name represents.*/ 
    unsigned int midiChannelPrefix = 0;


    //read events until end of track
    while(1)
    {
        //get the first 4 bit of event code from right (mask = 11110000)
        unsigned int first4BitOfEventCode = eventCode & (0xF0);
        
        if(eventCode >= 0xF0 && eventCode <= 0xFF)
        {   
            previousEventCode = eventCode;
            //this event is a metaevent or sysex


            if(eventCode == 0xFF)
            {
                //this is a meta event
                midiEvents[trackEventCount].eventType = META_EVENT;


                //read the type of metaevent
                unsigned int metaEventType;
                //read one byte
                ei_ui_fread(&metaEventType, 1, 1, midiFile);

                //setting the event type value
                midiEvents[trackEventCount].metaEvent.specificEventType = metaEventType;



                //get the length
                unsigned int metaEventLength;
                if(metaEventType == SEQUENCER_SPECIFIC)
                {
                    metaEventLength = readVariableLengthValue(midiFile);

                }else
                {
                    //read one byte
                    ei_ui_fread(&metaEventLength, 1, 1, midiFile);
                }
                midiEvents[trackEventCount].metaEvent.length = metaEventLength;


                //break if it reaches the end of track
                if(metaEventType == END_OF_TRACK)
                {
                    *numberOfEvents = trackEventCount;
                    *numberOfNotes = noteCount;
                    return midiEvents;
                }


                //read and set data
                midiEvents[trackEventCount].metaEvent.data = calloc(metaEventLength, sizeof(char));
                fread(midiEvents[trackEventCount].metaEvent.data, sizeof(char), metaEventLength, midiFile);



                //if it was a channel prefix set the channel
                if (metaEventType == CHANNEL_PREFIX)
                {
                    midiChannelPrefix = midiEvents[trackEventCount].metaEvent.data[0];
                }


                //set the channel of curent event
                midiEvents[trackEventCount].metaEvent.channelNumber = midiChannelPrefix;
                
                
                
            }else
            {
                //this is a sysex event
                midiEvents[trackEventCount].eventType = SYSEX_EVENT;

                //set the length
                unsigned int sysExLength = readVariableLengthValue(midiFile);
                midiEvents[trackEventCount].sysExEvent.length = sysExLength;

                //read and set the data
                midiEvents[trackEventCount].sysExEvent.data = calloc(sysExLength, sizeof(char));
                fread(midiEvents[trackEventCount].sysExEvent.data, sizeof(char), sysExLength, midiFile);

                //set the channel of curent event
                midiEvents[trackEventCount].sysExEvent.channelNumber = midiChannelPrefix;    
            }
            

        }else if(eventCode >= 0x80 && eventCode <= 0xEF)
        {
            //this event is a midichannel event
            previousEventCode = eventCode;

            midiChannelPrefix = 0;
            /*CHANNEL_PREFIX is used to associate any subsequent SysEx and Meta events with a particular MIDI channel, 
            and will remain in effect until the next MIDI Channel Prefix Meta event or the next MIDI event.*/

            // Delta Time 	Event Type Value 	MIDI Channel 	Parameter 1 	Parameter 2
            midiEvents[trackEventCount].eventType = MIDI_CHANNEL_EVENT;

            //setting the event type value
            midiEvents[trackEventCount].channelEvent.specificEventType = first4BitOfEventCode;
            
            //this is the channel number (mask = 00001111)
            unsigned int second4BitOfEventCode = eventCode & (0x0F);
            midiEvents[trackEventCount].channelEvent.channelNumber = second4BitOfEventCode;

            //getting the first parameter 
            unsigned int param1;
            //read one byte
            ei_ui_fread(&param1, 1, 1, midiFile);
            midiEvents[trackEventCount].channelEvent.param1 = param1;

            if(first4BitOfEventCode != PROGRAM_CHNG && first4BitOfEventCode != CHANNEL_AFTERTOUCH)
            { 
                //getting the second parameter 
                unsigned int param2;
                //read one byte
                ei_ui_fread(&param2, 1, 1, midiFile);
                midiEvents[trackEventCount].channelEvent.param2 = param2;

                //if the velocity of a noteon event was zero, turn in into a note off
                if(first4BitOfEventCode == NOTE_ON && param2 == 0)
                {
                    midiEvents[trackEventCount].channelEvent.specificEventType = NOTE_OFF;
                }else if(first4BitOfEventCode == NOTE_ON && param2 != 0)
                {
                    noteCount++;
                }
            }

            

        }else
        {
            //this is a running status
            /* The MIDI spec allows for a MIDI message to be sent without its Status byte
            as long as the previous, transmitted message had the same Status. */

            midiEvents[trackEventCount].eventType = MIDI_CHANNEL_EVENT;

            //get the first 4 bit of previous event code from right (mask = 11110000)
            first4BitOfEventCode = previousEventCode & (0xF0);

            //setting the event type value
            midiEvents[trackEventCount].channelEvent.specificEventType = first4BitOfEventCode;
            
            //this is the channel number (mask = 00001111)
            unsigned int second4BitOfEventCode = previousEventCode & (0x0F);
            midiEvents[trackEventCount].channelEvent.channelNumber = second4BitOfEventCode;

            //since this is a running status the current eventcode we just read is actually a parameter
            unsigned int param1 = eventCode;
            midiEvents[trackEventCount].channelEvent.param1 = param1;

            if(first4BitOfEventCode != PROGRAM_CHNG && first4BitOfEventCode != CHANNEL_AFTERTOUCH)
            { 
                //getting the second parameter 
                unsigned int param2;
                //read one byte
                ei_ui_fread(&param2, 1, 1, midiFile);
                midiEvents[trackEventCount].channelEvent.param2 = param2;

                //if the velocity of a noteon event was zero, turn in into a note off
                if(first4BitOfEventCode == NOTE_ON && param2 == 0)
                {
                    midiEvents[trackEventCount].channelEvent.specificEventType = NOTE_OFF;

                }else if(first4BitOfEventCode == NOTE_ON && param2 != 0)
                {
                    noteCount++;
                }
            }


            

            
        }
        

        //increment the counter for midiEvents array
        trackEventCount++;

        //check the count and buffer, if it is realloc it
        if(trackEventCount == trackEventCountBuffer)
        {
            trackEventCountBuffer *=3;
            MidiEvent *dummyptr;
            dummyptr = realloc(midiEvents, (trackEventCountBuffer) * sizeof(MidiEvent));
            if(dummyptr == NULL)
            {
                printf("An erorr has occured");
                return dummyptr;
            }else
            {
                midiEvents = dummyptr;
            }
            

        }
        //read next delta time
        deltaTime = readVariableLengthValue(midiFile);
        //fread(&deltaTime, 1, 1, midiFile);
        midiEvents[trackEventCount].deltaTime = deltaTime;

        // the next event code
        ei_ui_fread(&eventCode, 1, 1, midiFile); 
    }
}
////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////

// reads a variable length value from the file
unsigned int readVariableLengthValue(FILE *midiFile)
{
    void ei_ui_fread(unsigned int *ptr, size_t size, size_t n, FILE *streamPtr);
	unsigned int final;
	int c;

	ei_ui_fread(&c, sizeof(char), 1, midiFile);

	final = c;

	if(c & 128) 
    {
		final &= 127;
		do
        {
			ei_ui_fread(&c, sizeof(char), 1, midiFile);
			final = (final << 7) + (c & 127);
		} while(c & 128);
	}
	return final;
}


////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////


void printMidiHeader(MidiHeader *midiHeader, FILE *midiFile)
{
    //principle of least privilege
    void readHeaderChunk(MidiHeader *, FILE *);

    readHeaderChunk(midiHeader, midiFile);

    printf("************\n** Header **\n************\n");
    printf("* Chunk Type: %s \n", midiHeader->chunkType);
    printf("* Header Legnth: %u bytes\n", midiHeader->length);
    printf("* Format: %u \n", midiHeader->format);
    printf("* Tracks: %u \n", midiHeader->tracks);
    if(midiHeader->devisionType == 1)
    {
        printf("* Frame Per Second: %u", midiHeader->framePerSecond);
        printf("* Ticks Per Frame: %u", midiHeader->ticksPerFrame);

    }else if(midiHeader->devisionType == 0)
    {
        printf("* Ticks Per Quarter Note: %u\n\n\n", midiHeader->ticksPerQuarterNote);
    }

}


////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////


//reads the content of the given midiheader
void readHeaderChunk(MidiHeader *midiHeader, FILE *fptr)
{
    //principle of least privilege
    void ei_ui_fread(unsigned int *ptr, size_t size, size_t n, FILE *streamPtr);

    //read 32 bits
    fread(&(midiHeader->chunkType), 1, 4, fptr);
   
    //read 32 bits
    ei_ui_fread(&(midiHeader->length), 4, 1, fptr);

    //read 16 bits
    ei_ui_fread(&(midiHeader->format), 1, 2, fptr);

    //read 16 bits
    ei_ui_fread(&(midiHeader->tracks), 1, 2, fptr);

    //read 16 bits
    ei_ui_fread(&(midiHeader->devision), 1, 2, fptr);


    //reading the first bit of devision with a mask (to specify devision type)
    unsigned int mask = 1 << 15;
    midiHeader->devisionType = midiHeader->devision & mask;
    midiHeader->devisionType = midiHeader->devisionType >> 15;

    if(midiHeader->devisionType == 0)
    {

        midiHeader->ticksPerQuarterNote = midiHeader->devision;

    }else if(midiHeader->devisionType == 1)
    {
        
        //with masks and using bitwise shift operator we can extract the wanted data from devision
        //00000000 00000000 0------- ^^^^^^^^
        // - are fps
        // ^ are tpf

        unsigned int maskFPS = 0x7F00;
        // fps mask: 00000000 00000000 01111111 00000000
        midiHeader->framePerSecond = (midiHeader->devision & maskFPS) >> 8;

        
        unsigned int maskTPF = 0x00FF;
        //tpf mask: 00000000 00000000 00000000 11111111
        midiHeader->ticksPerFrame = midiHeader->devision & maskTPF;
    }

    // a controll statement for header chunks larger than 6 bytes
    if(midiHeader->length > 6)
    {
        printf("* Warning: This is not a standard Midi File!");
        int numberOfBytesOfAdditionalInformationInHeader = midiHeader->length - 6;
        int additionalInformationInHeader;
        fread(&additionalInformationInHeader, 1, numberOfBytesOfAdditionalInformationInHeader, fptr);
    }
}


////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////



//Endian-independent fread function to read unsigned int
void ei_ui_fread(unsigned int *ptr, size_t size, size_t n, FILE *streamPtr)
{
    size_t dataSize = size * n;
    unsigned char data[dataSize];

    fread(data, size, n, streamPtr);
    *ptr = 0;

    for(int i = 0, j = (dataSize - 1) * 8; i < dataSize && j >= 0; i+=1, j-=8)
    {
        *ptr |= data[i]<<j;
    }

}


////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////


//Endian-independent function to read unsigned int from a byte array
void ei_ui_byteArray(unsigned int *ptr, size_t dataSize, char unsigned *data)
{
    *ptr = 0;

    for(int i = 0, j = (dataSize - 1) * 8; i < dataSize && j >= 0; i+=1, j-=8)
    {
        *ptr |= data[i]<<j;
    }
    unsigned int new;
    new = (data[2]<<0) | (data[1]<<8) | (data[0]<<16);

}


float turnMidiNoteNumberToFrequency(unsigned int noteNumber)
{
    //frequency of A4
    int a = 440; 

    return (a / 32) * pow(2, ((noteNumber - 9) / 12.0));
    
}

////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////



