#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "beep.h"
#include <signal.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include "midi.h"
#include "guiconnection.h"
#include "midiparser.h"

//uncomment this and the related function to play a song from intro-song.txt
//#include "intro.h"

//global flags
GuiStatus midiFileFlagStatus = STATUS_CLOSE;
GuiStatus cacheFileFlagStatus = STATUS_CLOSE;
GuiStatus memoryCleanFlagStatus = STATUS_CLEANED;

//global prototypes
void playMidiNotes(Note *notesArray, size_t size, GuiStatus *);
void cleanUpAllocatedSpace(MidiTrack *tracks,unsigned int numberOfTracks ,NoteSequence *noteSequence);

//global variables
MidiTrack *tracks;
NoteSequence *noteSequence;
FILE *midiFile;
MidiHeader midiHeader;
FILE *midiEventCacheFile;
unsigned int trackTotalBytes = 0;

pid_t *pid_parent;
//opens the given midi file and creates an event cache
GuiStatus openMidiFileAndCreateEventCashe(char *filePath, pid_t *ppid)
{
    unsigned int readVariableLengthValue(FILE *midiFile);
    MidiEvent *readTrackEvents(FILE *midiFile, size_t *numberOfEvents, size_t *numberOfNotes, unsigned int);
    void ei_ui_fread(unsigned int *ptr, size_t size, size_t n, FILE *streamPtr);
    Note *composeNotes(MidiEvent *midiEvents, size_t numberOfEvents, size_t numberOfNotes, MidiCurrentStatus *, FILE *);
    void cleanUpAllocatedSpace(MidiTrack *tracks,unsigned int numberOfTracks ,NoteSequence *noteSequence);

    // _+_+_+_+_+( PHASE I )+_+_+_+_+_
    // Play the intro song using the intro.c source file 
    // uncomment the include path
    // playIntro();
    
    pid_parent = ppid;

    
    //open a midi file
    midiFile = fopen(filePath, "rb");
    if(midiFile == NULL)
    {
        kill(*pid_parent, FILENOTFOUND);
    }
    midiFileFlagStatus = STATUS_OPEN;

    //create a new file for writing out the events
    midiEventCacheFile = fopen("cachedEvents.txt", "w");

    if(midiEventCacheFile == NULL)
    {
        kill(*pid_parent, FILENOTFOUND);
    }
    cacheFileFlagStatus = STATUS_OPEN;

    // _+_+_+_+_+( PHASE II )+_+_+_+_+_
    //print the midi file's header
    printMidiHeader(&midiHeader, midiFile, midiEventCacheFile);

    //setting the read data to  a midi current status struct
    //the default tempo is 120BPM that translates to 60000000 / 120 = 500000 microseconds per quarter note
    MidiCurrentStatus midiCurrentStatus;

    //tempo is in terms of microseconds per quarter note
    midiCurrentStatus.tempo = 60000000 / 120;

    midiCurrentStatus.divisionType = midiHeader.devisionType;

    if(midiHeader.devisionType == 0)
    {
        //microseconds per tick which is tempo / ticksPerQuarterNote 
        midiCurrentStatus.msPerTick = midiCurrentStatus.tempo /  midiHeader.ticksPerQuarterNote;
        midiCurrentStatus.ticksPerQuarterNote = midiHeader.ticksPerQuarterNote;

    }else if(midiHeader.devisionType == 1)
    {
        //microseconds per tick which is 1000000 / (tpf * fps)
        midiCurrentStatus.msPerTick = 1000000 /  ( midiHeader.framePerSecond * midiHeader.ticksPerFrame);
        midiCurrentStatus.framePerSecond = midiHeader.framePerSecond;
        midiCurrentStatus.ticksPerFrame = midiHeader.ticksPerFrame;
    }
    
    // _+_+_+_+_+( PHASE III )+_+_+_+_+_
    //initialize the tracks and populate them with events
    tracks = calloc(midiHeader.tracks, sizeof(MidiTrack));
    //there is a note sequence for every track
    noteSequence = calloc(midiHeader.tracks, sizeof(NoteSequence));

    if(tracks == NULL || noteSequence == NULL)
        kill(*pid_parent, GENERALERROR);


    memoryCleanFlagStatus = STATUS_UNCLEAN;

    for(int i = 0; i < midiHeader.tracks; i++)
    {
        char chunkID[5];
        //read 4 bytes
        fread(&chunkID, 1, 4, midiFile);
        chunkID[5] = '\0';
        if(strstr(chunkID, "MTrk") == NULL)
        {
            kill(*pid_parent, CORRUPTMIDI);
        }

        fprintf(midiEventCacheFile, "%-22s -> %4s \n", "Chunk Type", chunkID);

        unsigned int trackSize;
        ei_ui_fread(&trackSize, sizeof(char), 4, midiFile);
        fprintf(midiEventCacheFile, "%-22s -> %u bytes\n", "Track Legnth", trackSize);

        //reads all of the contents of the track
        tracks[i].midiEvents = readTrackEvents(midiFile, &(tracks[i].numberOfEvents), &(noteSequence[i].numberOfNotes), trackSize);


        //creates an array of notes using note on and note off events and parse other events
        noteSequence[i].notes = composeNotes(tracks[i].midiEvents, tracks[i].numberOfEvents, noteSequence[i].numberOfNotes, &midiCurrentStatus, midiEventCacheFile);
    
    }
    
    //closing the event cache file
    fclose(midiEventCacheFile);
    cacheFileFlagStatus = STATUS_CLOSE;
    //close the midi file stream
    fclose(midiFile);
    midiFileFlagStatus = STATUS_CLOSE;


    return STATUS_DONE;
}

////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////

void playMidiFile(GuiStatus *playStatus, pid_t *ppid)
{ 
    //prototypes
    void cleanUpAllocatedSpace(MidiTrack *tracks,unsigned int numberOfTracks ,NoteSequence *noteSequence);
    // _+_+_+_+_+( PHASE IV )+_+_+_+_+_
    pid_parent = ppid;

    for(int i = 0; i < midiHeader.tracks; i++)
    {
        playMidiNotes(noteSequence[i].notes, noteSequence[i].numberOfNotes, playStatus);
    }


    // _+_+_+_+_+( CLEAN UP )+_+_+_+_+_
    cleanUpAllocatedSpace(tracks,midiHeader.tracks ,noteSequence);
    memoryCleanFlagStatus = STATUS_CLEANED;
    exit(EXIT_SUCCESS);

}
////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////
void on_midiClosed()
{
    // _+_+_+_+_+( CLEAN UP )+_+_+_+_+_
    if(memoryCleanFlagStatus != STATUS_CLEANED)
    { 
        cleanUpAllocatedSpace(tracks,midiHeader.tracks ,noteSequence); 
        memoryCleanFlagStatus = STATUS_CLEANED;
    }

    //closing the event cache file
    if(cacheFileFlagStatus != STATUS_CLOSE)
    { 
        fclose(midiEventCacheFile);
        cacheFileFlagStatus = STATUS_CLOSE;
    }

    //close the midi file stream
    if(midiFileFlagStatus != STATUS_CLOSE)
    { 
        fclose(midiFile);
        midiFileFlagStatus = STATUS_CLOSE;
    }
}
////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////
void cleanUpAllocatedSpace(MidiTrack *tracks,unsigned int numberOfTracks ,NoteSequence *noteSequence)
{
    //clean up tracks and their data
    for(int i = 0; i < numberOfTracks; i++)
    {
        for(int j = 0; j < tracks[i].numberOfEvents; j++)
        { 
            if(tracks[i].midiEvents[j].eventType == META_EVENT)
            {
                free(tracks[i].midiEvents[j].metaEvent.data);

            }else if(tracks[i].midiEvents[j].eventType == SYSEX_EVENT)
            {
                free(tracks[i].midiEvents[j].sysExEvent.data);
            }
        }
        free(tracks[i].midiEvents);
    }

    //cleanup note sequence
    for(int i = 0; i < numberOfTracks; i++)
    {
        free(noteSequence[i].notes);
    }


    free(tracks);
    free(noteSequence);

}
////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////
//this functions matches note of events with their corresponding note on events and alculates their length and returns an array of notes 
Note *composeNotes(MidiEvent *midiEvents, size_t numberOfEvents, size_t numberOfNotes, MidiCurrentStatus *midiCurrentStatus, FILE *midiEventCacheFile)
{
    //prototypes
    float turnMidiNoteNumberToFrequency(unsigned int);
    void findCorrespondingNoteOn(Note *noteArray, size_t noteCount, unsigned int noteNumber, unsigned int playTime, MidiCurrentStatus *);
    void ei_ui_byteArray(unsigned int *ptr, size_t dataSize, unsigned char *data);

    Note *noteArray = calloc(numberOfNotes, sizeof(Note));
    size_t noteCount = 0;

    // the elapsed time since the first event in ticks
    unsigned int playTime;

    short int hasCorruptValue = 0;

    for(int i = 0; i < numberOfEvents && noteCount <= numberOfNotes; i++)
    {
        //ad the current events delta time to total Playtime
        playTime += midiEvents[i].deltaTime;

        //print the current events delta time
        fprintf(midiEventCacheFile, "%-10u",midiEvents[i].deltaTime);



        //control statements for handling midi events
        switch (midiEvents[i].eventType)
        {
        //Meta Event
        case META_EVENT:

            //print the current events channel
            fprintf(midiEventCacheFile, "|%-2u|",midiEvents[i].metaEvent.channelNumber);

            switch (midiEvents[i].metaEvent.specificEventType)
            {
            case SET_TEMPO:
                ;
                //endian independently set the 3 byte data of previously read tempo as the current tempo
                unsigned int newTempo = 0;
                ei_ui_byteArray(&newTempo, midiEvents[i].metaEvent.length, midiEvents[i].metaEvent.data);

                if( newTempo <= 8355711)
                { 
                    midiCurrentStatus->tempo = newTempo;

                    if(midiCurrentStatus->divisionType == 0)
                    { 
                        midiCurrentStatus->msPerTick = midiCurrentStatus->tempo / midiCurrentStatus->ticksPerQuarterNote;

                    }else if(midiCurrentStatus->divisionType == 1)
                    {
                        /* fixed. we cannot calculate msPerTick using tempo, fps, tpf, since:
                            TimeBase = MicroTempo / MicrosPerPPQN
                            SubFramesPerQuarterNote = MicroTempo/(Frames * SubFrames)
                            SubFramesPerPPQN = SubFramesPerQuarterNote / TimeBase
                            MicrosPerPPQN = SubFramesPerPPQN * Frames * SubFrames*/
                    }
                }else
                {
                    hasCorruptValue = 1;
                }
                
                //print the current event
                fprintf(midiEventCacheFile, " %-20s -> %u", "Set Tempo", newTempo);

                break;

            //the rest doesnt effect the midi playback...
            case SEQUENCE_NUMBER:

                if( (midiEvents[i].metaEvent.data[0] >= 0 && midiEvents[i].metaEvent.data[0] < 256) && (midiEvents[i].metaEvent.data[1] >= 0 && midiEvents[i].metaEvent.data[1] < 256))
                {
                    /* nothing to be done, we'll just print the event */

                }else
                {
                    hasCorruptValue = 1;
                }
                //print the current event
                fprintf(midiEventCacheFile, " %-20s -> MSB:%u LSB:%u", "Sequence Number", midiEvents[i].metaEvent.data[0], midiEvents[i].metaEvent.data[1]);
                break;
            case TEXT_EVENT:
                //an indicator that this is a text event
                fprintf(midiEventCacheFile, ">");
 
                //print the current event
                fprintf(midiEventCacheFile, "%-20s", "Text Event");
                fprintf(midiEventCacheFile, " -> text: ");
                for(int j = 0; j < midiEvents[i].metaEvent.length; j++)
                    fprintf(midiEventCacheFile, "%c", midiEvents[i].metaEvent.data[j]);

                break;
            case COPYRIGHT_NOTICE:
                //an indicator that this is a text event
                fprintf(midiEventCacheFile, ">");
 
                //print the current event
                fprintf(midiEventCacheFile, "%-20s", "Copyright Notice");
                fprintf(midiEventCacheFile, " -> text: ");
                for(int j = 0; j < midiEvents[i].metaEvent.length; j++)
                    fprintf(midiEventCacheFile, "%c", midiEvents[i].metaEvent.data[j]);
                break;
            case SEQUENCE_NAME:
                //an indicator that this is a text event
                fprintf(midiEventCacheFile, ">");
 
                //print the current event
                fprintf(midiEventCacheFile, "%-20s", "Sequence Name");
                fprintf(midiEventCacheFile, " -> text: ");
                for(int j = 0; j < midiEvents[i].metaEvent.length; j++)
                    fprintf(midiEventCacheFile, "%c", midiEvents[i].metaEvent.data[j]);
                break;
            case INSTRUMENT_NAME:
                //an indicator that this is a text event
                fprintf(midiEventCacheFile, ">");
 
                //print the current event
                fprintf(midiEventCacheFile, "%-20s", "Instrument Name");
                fprintf(midiEventCacheFile, " -> text: ");
                for(int j = 0; j < midiEvents[i].metaEvent.length; j++)
                    fprintf(midiEventCacheFile, "%c", midiEvents[i].metaEvent.data[j]);
                break;
            case LYRIC:
                //an indicator that this is a text event
                fprintf(midiEventCacheFile, ">");
 
                //print the current event
                fprintf(midiEventCacheFile, "%-20s", "Lyric");
                fprintf(midiEventCacheFile, " -> text: ");
                for(int j = 0; j < midiEvents[i].metaEvent.length; j++)
                    fprintf(midiEventCacheFile, "%c", midiEvents[i].metaEvent.data[j]);
                break;
            case MARKER:
                //an indicator that this is a text event
                fprintf(midiEventCacheFile, ">");
 
                ///print the current event
                fprintf(midiEventCacheFile, "%-20s", "Marker");
                fprintf(midiEventCacheFile, " -> text: ");
                for(int j = 0; j < midiEvents[i].metaEvent.length; j++)
                    fprintf(midiEventCacheFile, "%c", midiEvents[i].metaEvent.data[j]);
                break;
            case CUE_POINT:
                //an indicator that this is a text event
                fprintf(midiEventCacheFile, ">");
 
                //print the current event
                fprintf(midiEventCacheFile, "%-20s", "Cue Point");
                fprintf(midiEventCacheFile, " -> text: ");
                for(int j = 0; j < midiEvents[i].metaEvent.length; j++)
                    fprintf(midiEventCacheFile, "%c", midiEvents[i].metaEvent.data[j]);
                break;
            case PROGRAM_NAME:
                //an indicator that this is a text event
                fprintf(midiEventCacheFile, ">");
 
                ///print the current event
                fprintf(midiEventCacheFile, "%-20s", "Program Name");
                fprintf(midiEventCacheFile, " -> text: ");
                for(int j = 0; j < midiEvents[i].metaEvent.length; j++)
                    fprintf(midiEventCacheFile, "%c", midiEvents[i].metaEvent.data[j]);
                break;
            case DEVICE_NAME:
                //an indicator that this is a text event
                fprintf(midiEventCacheFile, ">");
 
                //print the current event
                fprintf(midiEventCacheFile, "%-20s", "Device Name");
                fprintf(midiEventCacheFile, " -> text: ");
                for(int j = 0; j < midiEvents[i].metaEvent.length; j++)
                    fprintf(midiEventCacheFile, "%c", midiEvents[i].metaEvent.data[j]);
                break;
            case CHANNEL_PREFIX:

                if( midiEvents[i].metaEvent.data[0] >= 0 && midiEvents[i].metaEvent.data[0] < 16 )
                {
                    /* nothing to be done, we'll just print the event */

                }else
                {
                    hasCorruptValue = 1;
                }
                //print the current event
                fprintf(midiEventCacheFile, " %-20s -> Ch:%u", "Channel Prefix", midiEvents[i].metaEvent.data[0]);
                break;
            case MIDI_PORT:

                if( midiEvents[i].metaEvent.data[0] >= 0 && midiEvents[i].metaEvent.data[0] < 128 )
                {
                    /* nothing to be done, we'll just print the event */

                }else
                {
                    hasCorruptValue = 1;
                }
                //print the current event
                fprintf(midiEventCacheFile, " %-20s -> %u", "Midi Port", midiEvents[i].metaEvent.data[0]);
                break;
            case SMPTE_OFFSET:

                if( (midiEvents[i].metaEvent.data[0] >= 0 && midiEvents[i].metaEvent.data[0] < 24) && 
                (midiEvents[i].metaEvent.data[1] >= 0 && midiEvents[i].metaEvent.data[1] < 60) && 
                (midiEvents[i].metaEvent.data[2] >= 0 && midiEvents[i].metaEvent.data[2] < 60) && 
                (midiEvents[i].metaEvent.data[3] >= 0 && midiEvents[i].metaEvent.data[3] < 31) && 
                (midiEvents[i].metaEvent.data[4] >= 0 && midiEvents[i].metaEvent.data[4] < 100) )
                {
                    /* nothing to be done, we'll just print the event */
                }else
                {
                    hasCorruptValue = 1;
                }
                //print the current event
                fprintf(midiEventCacheFile, " %-20s -> H:%u M:%u Sec:%u Fr:%u SubFr:%u", "SMPTE Offset", midiEvents[i].metaEvent.data[0], 
                midiEvents[i].metaEvent.data[1],
                midiEvents[i].metaEvent.data[2],
                midiEvents[i].metaEvent.data[3],
                midiEvents[i].metaEvent.data[4]);
                break;
            case TIME_SIGNATURE:
                ;
                unsigned int denom = 1;
                if( (midiEvents[i].metaEvent.data[0] >= 0 && midiEvents[i].metaEvent.data[0] < 256) && 
                (midiEvents[i].metaEvent.data[1] >= 0 && midiEvents[i].metaEvent.data[1] < 256) && 
                (midiEvents[i].metaEvent.data[2] >= 0 && midiEvents[i].metaEvent.data[2] < 256) && 
                (midiEvents[i].metaEvent.data[3] >= 1 && midiEvents[i].metaEvent.data[3] < 256) )
                {
                    denom = midiEvents[i].metaEvent.data[1];

                }else
                {
                    hasCorruptValue = 1;
                }
                //print the current event
                fprintf(midiEventCacheFile, " %-20s -> Num:%u Den:%u Met:%u 32nds:%u", "Time Signature", midiEvents[i].metaEvent.data[0], 
                (int)pow(2, denom),
                midiEvents[i].metaEvent.data[2],
                midiEvents[i].metaEvent.data[3]);
                break;
            case KEY_SIGNATURE:
                if( (midiEvents[i].metaEvent.data[0] > -8 && midiEvents[i].metaEvent.data[0] < 8) && 
                (midiEvents[i].metaEvent.data[1] >= 0 && midiEvents[i].metaEvent.data[1] <= 1) )
                {
                    /* nothing to be done, we'll just print the event */
                }else
                {
                    hasCorruptValue = 1;
                }
                //print the current event
                fprintf(midiEventCacheFile, " %-20s -> Key:%u Scale:%u", "Key Signature", midiEvents[i].metaEvent.data[0], midiEvents[i].metaEvent.data[1]);                break;
            case SEQUENCER_SPECIFIC:
                //print the current event
                fprintf(midiEventCacheFile, " %-20s -> Len:%u", "Sequence Specific", midiEvents[i].metaEvent.length);
                break;
            default:
                break;
            }
            break;

        //Midi channel event
        case MIDI_CHANNEL_EVENT:
            //print the current events channel
            fprintf(midiEventCacheFile, "|%-2u|",midiEvents[i].channelEvent.channelNumber);
            switch (midiEvents[i].channelEvent.specificEventType)
            {
            case NOTE_ON:

                noteArray[noteCount].frequency = turnMidiNoteNumberToFrequency(midiEvents[i].channelEvent.param1);
                noteArray[noteCount].midiNoteNumber = midiEvents[i].channelEvent.param1;
                noteArray[noteCount].velocity = midiEvents[i].channelEvent.param2;
                noteArray[noteCount].playTime = playTime;
                noteArray[noteCount].delay = (midiEvents[i].deltaTime * midiCurrentStatus->msPerTick) / 1000;
                noteCount++;

                if( (midiEvents[i].channelEvent.param1 >= 0 && midiEvents[i].channelEvent.param1 < 128) && (midiEvents[i].channelEvent.param2 >= 0 && midiEvents[i].channelEvent.param2 < 128))
                {
                    /* nothing to be done, we'll just print the event */

                }else
                {
                    hasCorruptValue = 1;
                    //we'll set the velocity for this corrupted note to 0 so it wont be played
                    noteArray[noteCount].velocity = 0;

                }
                
                //print the current event
                fprintf(midiEventCacheFile, " %-20s -> #%u V:%u", "Note On", midiEvents[i].channelEvent.param1, midiEvents[i].channelEvent.param2);
                
                break;
            case NOTE_OFF:

                if( (midiEvents[i].channelEvent.param1 >= 0 && midiEvents[i].channelEvent.param1 < 128) && (midiEvents[i].channelEvent.param2 >= 0 && midiEvents[i].channelEvent.param2 < 128))
                {
                    /* nothing to be done, we'll just print the event */
                }else
                {
                    hasCorruptValue = 1;
                }

                findCorrespondingNoteOn(noteArray, noteCount, midiEvents[i].channelEvent.param1, playTime, midiCurrentStatus);
                //if the program doesnt find a corresponding note on the length will be 0

                //print the current event
                fprintf(midiEventCacheFile, " %-20s -> #%u V:%u", "Note Off", midiEvents[i].channelEvent.param1, midiEvents[i].channelEvent.param2);
                break;
            case POLY_AFTERTOUCH:

                if( (midiEvents[i].channelEvent.param1 >= 0 && midiEvents[i].channelEvent.param1 < 128) && (midiEvents[i].channelEvent.param2 >= 0 && midiEvents[i].channelEvent.param2 < 128))
                {
                    /* nothing to be done, we'll just print the event */
                }else
                {
                    hasCorruptValue = 1;
                }
                //print the current event
                fprintf(midiEventCacheFile, " %-20s -> #%u val:%u", "Poly Aftertouch", midiEvents[i].channelEvent.param1, midiEvents[i].channelEvent.param2);
                break;
            case CONTROL_CHANGE:
                if( (midiEvents[i].channelEvent.param1 >= 0 && midiEvents[i].channelEvent.param1 < 128) && (midiEvents[i].channelEvent.param2 >= 0 && midiEvents[i].channelEvent.param2 < 128))
                {
                    /* nothing to be done, we'll just print the event */

                }else
                {
                    hasCorruptValue = 1;
                }
                //print the current event
                fprintf(midiEventCacheFile, " %-20s -> #:%u val:%u", "Control Change", midiEvents[i].channelEvent.param1, midiEvents[i].channelEvent.param2);
                break;
            case PROGRAM_CHNG:

                if( midiEvents[i].channelEvent.param1 >= 0 && midiEvents[i].channelEvent.param1 < 128 )
                {
                    /* nothing to be done, we'll just print the event */

                }else
                {
                    hasCorruptValue = 1;
                }
                //print the current event
                fprintf(midiEventCacheFile, " %-20s -> #:%u", "Program Change", midiEvents[i].channelEvent.param1);
                break;
            case CHANNEL_AFTERTOUCH:
                if( midiEvents[i].channelEvent.param1 >= 0 && midiEvents[i].channelEvent.param1 < 128 )
                {
                    /* nothing to be done, we'll just print the event */

                }else
                {
                    hasCorruptValue = 1;
                }
                //print the current event
                fprintf(midiEventCacheFile, " %-20s -> #%u", "Channel Aftertouch", midiEvents[i].channelEvent.param1);
                break;
            case PITCH_BEND:

                if( (midiEvents[i].channelEvent.param1 >= 0 && midiEvents[i].channelEvent.param1 < 128) && (midiEvents[i].channelEvent.param2 >= 0 && midiEvents[i].channelEvent.param2 < 128))
                {
                    /* nothing to be done, we'll just print the event */

                }else
                {
                    hasCorruptValue = 1;
                }
                //print the current event
                fprintf(midiEventCacheFile, " %-20s -> lsb:%u msb:%u", "Pitch Bend", midiEvents[i].channelEvent.param1, midiEvents[i].channelEvent.param2);
                break;
            default:
                break;
            }
            break;

        //System exclusive event
        case SYSEX_EVENT:
            //print the current events channel
            fprintf(midiEventCacheFile, "|%-2u|",midiEvents[i].sysExEvent.channelNumber);

            fprintf(midiEventCacheFile, " %-20s -> Len:%u", "SysEx Event", midiEvents[i].sysExEvent.length);

            break;
        
        default:
            break;
        }

        //if the event contains a corrupt value it wil flag it
        if(hasCorruptValue == 1)
        {
            fprintf(midiEventCacheFile, " ERR");
            hasCorruptValue = 0;
        }
         
        //print a new line
        fprintf(midiEventCacheFile, "\n");
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
            if(noteArray[i].velocity == 0)
                noteArray[i].length = 0;
            else
                noteArray[i].length = ((playTime - noteArray[i].playTime) * midiCurrentStatus->msPerTick) / 1000;
            return;
        }
    }
    return;
}

////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////


MidiEvent *readTrackEvents(FILE *midiFile, size_t *numberOfEvents, size_t *numberOfNotes, unsigned int trackSize)
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

    trackTotalBytes = 0;


    //read delta time
    unsigned int deltaTime = readVariableLengthValue(midiFile);
    midiEvents[trackEventCount].deltaTime = deltaTime;

    unsigned int eventCode;
    unsigned int previousEventCode; //for running status
    //read one byte
    ei_ui_fread(&eventCode, 1, 1, midiFile);
    trackTotalBytes++;



    /*This meta event associates a MIDI channel with following meta events. It's effect is terminated by another MIDI Channel Prefix event or any non- Meta event.
    It is often used before an Instrument Name Event to specify which channel an instrument name represents.*/ 
    unsigned int midiChannelPrefix = 0;


    //read events until end of track
    while(trackTotalBytes <= trackSize)
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
                trackTotalBytes++;


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
                    trackTotalBytes++;
                    
                }
                midiEvents[trackEventCount].metaEvent.length = metaEventLength;


                //break if it reaches the end of track
                if(metaEventType == END_OF_TRACK)
                {
                    deltaTime = readVariableLengthValue(midiFile);
                    if(trackTotalBytes-1 != trackSize)
                    {
                        kill(*pid_parent, ENDOFTRACKERROR);
                    }
                    *numberOfEvents = trackEventCount;
                    *numberOfNotes = noteCount;
                    return midiEvents;
                }


                //read and set data
                midiEvents[trackEventCount].metaEvent.data = calloc(metaEventLength, sizeof(char));
                fread(midiEvents[trackEventCount].metaEvent.data, sizeof(char), metaEventLength, midiFile);

                trackTotalBytes += metaEventLength;




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

                trackTotalBytes += sysExLength;

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

            trackTotalBytes += 1;

            midiEvents[trackEventCount].channelEvent.param1 = param1;

            if(first4BitOfEventCode != PROGRAM_CHNG && first4BitOfEventCode != CHANNEL_AFTERTOUCH)
            { 
                //getting the second parameter 
                unsigned int param2;
                //read one byte
                ei_ui_fread(&param2, 1, 1, midiFile);
                trackTotalBytes += 1;

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

                trackTotalBytes += 1;

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

        midiEvents[trackEventCount].deltaTime = deltaTime;

        // the next event code
        ei_ui_fread(&eventCode, 1, 1, midiFile);
        trackTotalBytes += 1;

    }
    //if the program reaches this part it means that it hasn't found end of track event within the bounds of track length
    kill(*pid_parent, ENDOFTRACKERROR);
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
    trackTotalBytes++;

	final = c;

	if(c & 128) 
    {
		final &= 127;
		do
        {
			ei_ui_fread(&c, sizeof(char), 1, midiFile);
            trackTotalBytes++;

			final = (final << 7) + (c & 127);
		} while(c & 128);
	}
	return final;
}


////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////


void printMidiHeader(MidiHeader *midiHeader, FILE *midiFile, FILE *midiEventCacheFile)
{
    //principle of least privilege
    void readHeaderChunk(MidiHeader *, FILE *);

    readHeaderChunk(midiHeader, midiFile);

    fprintf(midiEventCacheFile, "%-22s -> %s \n", "Chunk Type", midiHeader->chunkType);
    fprintf(midiEventCacheFile, "%-22s -> %u bytes\n", "Header Legnth", midiHeader->length);
    fprintf(midiEventCacheFile, "%-22s -> %u \n", "Format", midiHeader->format);
    fprintf(midiEventCacheFile, "%-22s -> %u \n", "Tracks", midiHeader->tracks);
    if(midiHeader->devisionType == 1)
    {
        fprintf(midiEventCacheFile, "%-22s -> %u", "Frame Per Second", midiHeader->framePerSecond);
        fprintf(midiEventCacheFile, "%-22s -> %u", "Ticks Per Frame", midiHeader->ticksPerFrame);

    }else if(midiHeader->devisionType == 0)
    {
        fprintf(midiEventCacheFile, "%-22s -> %u\n\n\n", "Ticks Per Quarter Note", midiHeader->ticksPerQuarterNote);
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
    midiHeader->chunkType[5] = '\0';
    if(strstr(midiHeader->chunkType, "MThd") == NULL)
    {
        kill(*pid_parent, CORRUPTMIDI);
    }
    
    
    //read 32 bits
    ei_ui_fread(&(midiHeader->length), 4, 1, fptr);

    if(midiHeader->length != 6)
    {
        kill(*pid_parent, CORRUPTMIDI);
        
    }

    //read 16 bits
    ei_ui_fread(&(midiHeader->format), 1, 2, fptr);
    if(midiHeader->format < 0 || midiHeader->format > 2)
    {
        kill(*pid_parent, CORRUPTMIDI);
    }
    //read 16 bits
    ei_ui_fread(&(midiHeader->tracks), 1, 2, fptr);
    if(midiHeader->tracks < 1 || midiHeader->tracks > 65535)
    {
        kill(*pid_parent, CORRUPTMIDI);
    }
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
        kill(*pid_parent, GENERALERROR);
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

//// a simple function to iterate through the array of midi notes and play them
void playMidiNotes(Note *notesArray, size_t size, GuiStatus *playStatus)
{
    void delay(int milliseconds);
    int i = 0;
    while(i < size)
    { 
        if(*playStatus == STATUS_PLAY)
        { 
            delay((notesArray + i)->delay);
            beep((notesArray + i)->frequency, (notesArray + i)->length + 130);
            i+=1;
        }
        
    }
}

////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////

//delay function
void delay(int milliseconds)
{
    long pause;
    clock_t now,then;

    pause = milliseconds*(CLOCKS_PER_SEC/1000);
    now = then = clock();
    while( (now-then) < pause )
        now = clock();
}


////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////