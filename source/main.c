#include <stdio.h>
#include <stdlib.h>
#include "beep.h"
#include "intro.h"
#include "midi.h"



int main()
{
    unsigned int readVariableLengthValue(FILE *midiFile);
    void readTrackEvents(FILE *midiFile, MidiEvent *midiEvents);
    void ei_ui_fread(unsigned int *ptr, size_t size, size_t n, FILE *streamPtr);

    // _+_+_+_+_+( PHASE I )+_+_+_+_+_
    // Play the intro song using the intro.c source file 
    //playIntro();
    
    //open a midi file
    FILE *midiFile = fopen("music.mid", "rb");


    // _+_+_+_+_+( PHASE II )+_+_+_+_+_
    //print the midi file's header
    MidiHeader midiHeader; 
    printMidiHeader(&midiHeader, midiFile);

    // _+_+_+_+_+( PHASE III )+_+_+_+_+_
    //initialize the tracks and populate them with events
    MidiTrack *tracks = calloc(midiHeader.tracks, sizeof(MidiTrack));
    for(int i = 0; i < midiHeader.tracks; i++)
    {
        char chunkID[4];
        //read 32 bits
        fread(&chunkID, 1, 4, midiFile);

        unsigned int trackSize;
        ei_ui_fread(&trackSize, sizeof(char), 4, midiFile);

        tracks[i].midiEvents = calloc(500, sizeof(MidiEvent));
        readTrackEvents(midiFile, tracks[i].midiEvents);
    }




    // _+_+_+_+_+( PHASE IV )+_+_+_+_+_


    //close the midi file stream
    fclose(midiFile);

}

////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////

void readTrackEvents(FILE *midiFile, MidiEvent *midiEvents)
{
    //prototypes
    unsigned int readVariableLengthValue(FILE *midiFile);
    void ei_ui_fread(unsigned int *ptr, size_t size, size_t n, FILE *streamPtr);

    size_t trackEventCount = 0;

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
                    break;
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
            previousEventCode = eventCode;
            //this event is a midichannel event
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

            //getting the second parameter 
            unsigned int param2;
            //read one byte
            ei_ui_fread(&param2, 1, 1, midiFile);

            //setting the params
            midiEvents[trackEventCount].channelEvent.param1 = param1;
            midiEvents[trackEventCount].channelEvent.param2 = param2;

            

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

            //getting the second parameter 
            unsigned int param2;
            //read one byte
            ei_ui_fread(&param2, 1, 1, midiFile);

            //setting the params
            midiEvents[trackEventCount].channelEvent.param1 = param1;
            midiEvents[trackEventCount].channelEvent.param2 = param2;

            
        }
        

        //increment the counter for midiEvents array
        trackEventCount++;
        printf("%d\n", deltaTime);

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



