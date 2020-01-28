#ifndef _MIDIPARSER_H_
#define _MIDIPARSER_H_

#include "midiparser.h"

GuiStatus openMidiFileAndCreateEventCashe(char *filePath, pid_t *);
void playMidiFile(GuiStatus *, pid_t *);
void on_midiClosed();


#endif