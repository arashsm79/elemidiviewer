#ifndef __LIB_MIDI_PARSER_HEADER_
#define __LIB_MIDI_PARSER_HEADER_

#include <stdbool.h>

// could have used uint8_t, uint16_t, uint32_t and uint64_t. instead of unsigned byte, ...
//#include <stdint.h>

typedef enum
{
	MIDI_CHANNEL_EVENT,
	META_EVENT = 0xFF,
	SYSEX_EVENT = 0xF0,
} EventType;

typedef enum
{
	/* MIDI META EVENTS */
	SEQUENCE_NUMBER = 		0X00,
	TEXT_EVENT = 			0X01,
	COPYRIGHT_NOTICE = 		0X02,
	SEQUENCE_NAME = 		0X03,
	INSTRUMENT_NAME = 		0X04,
	LYRIC = 				0X05,
	MARKER = 				0X06,
	CUE_POINT = 			0X07,
	PROGRAM_NAME = 			0x08,
	DEVICE_NAME = 			0x09,
	CHANNEL_PREFIX = 		0X20,
	MIDI_PORT = 			0x21, //this is considered obsolete (changed with DEVICE NAME)
	SET_TEMPO =	 			0X51,
	SMPTE_OFFSET = 			0X54,
	TIME_SIGNATURE = 		0X58,
	KEY_SIGNATURE = 		0X59,
	SEQUENCER_SPECIFIC = 	0X7F,	
	END_OF_TRACK = 			0X2F,

	/* MIDI CHANNEL EVENTS */
	NOTE_OFF = 				0X80,
	NOTE_ON = 				0X90,
	POLY_AFTERTOUCH = 		0XA0,
	CONTROL_CHANGE = 		0XB0,
	PROGRAM_CHNG = 			0XC0,
	CHANNEL_AFTERTOUCH = 	0XD0,
	PITCH_BEND = 			0XE0,
} SpecificEventType;


// /* Channel Event Decleration */
	typedef struct
	{
		
		SpecificEventType specificEventType;
		unsigned int channelNumber;
		unsigned int param1;
		unsigned int param2;

	} ChannelEvent;



// /* Meta Event Decleration */
	typedef struct
	{
		SpecificEventType specificEventType;
		unsigned int channelNumber;
		unsigned int length;
		unsigned char *data;
		
	} MetaEvent;



// /* SysEx Event Decleration */
	typedef struct
	{
		unsigned int channelNumber;
		unsigned int length;
		unsigned char *data;
		
	} SysExEvent;


// /* Event Decleration */
	typedef struct
	{
		EventType eventType;
		unsigned int deltaTime;
		
		union {
			ChannelEvent channelEvent;
			MetaEvent metaEvent;
			SysExEvent sysExEvent;
		};

	} MidiEvent;


// /* Track Decleration */
typedef struct
{
	unsigned int trackSize;
	size_t numberOfEvents;
	MidiEvent *midiEvents;
    
} MidiTrack;


// /* Header Decleration */
typedef struct
{
	char chunkType[5];

    unsigned int length;

    unsigned int format;

    unsigned int tracks;

    unsigned int devision;

    unsigned int devisionType;

    unsigned int ticksPerQuarterNote;

    unsigned int framePerSecond;

    unsigned int ticksPerFrame;

} MidiHeader;

typedef struct
{
	//tempo is in terms of microseconds per quarter note
	unsigned int tempo;
	
	//microseconds per tick which is tempo / ticksPerQuarterNote 
	unsigned int msPerTick;

	unsigned int divisionType;

	unsigned int ticksPerQuarterNote;

    unsigned int framePerSecond;

    unsigned int ticksPerFrame;

} MidiCurrentStatus;





// a note structure
typedef struct{
    float frequency;
    int length;
	unsigned int delay;
	unsigned int playTime;
	unsigned int midiNoteNumber;
	unsigned int velocity;

} Note;

typedef struct 
{
	size_t numberOfNotes;
	Note *notes;
} NoteSequence;

void printMidiHeader(MidiHeader *, FILE *, FILE *);
unsigned int readVariableLengthValue(FILE *);

void print_midi_track();
void Midi_player();

#endif
