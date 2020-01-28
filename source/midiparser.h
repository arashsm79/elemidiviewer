#ifndef _MIDIPARSER_H_
#define _MIDIPARSER_H_

#include "midiparser.h"

GuiStatus openMidiFileAndCreateEventCashe(char *filePath);
GuiStatus playMidiFile(GuiStatus *);
void on_midiClosed();


#endif