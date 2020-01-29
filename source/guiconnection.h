#ifndef _GUICONNECTION_H_
#define _GUICONNECTION_H_

#include "guiconnection.h"
//an status enum for communicatic with the gui
typedef enum {

    STATUS_DONE, 
    STATUS_FAILED ,
    STATUS_PROCESSING, 
    STATUS_PLAY, 
    STATUS_PAUSE, 
    STATUS_CLEANED, 
    STATUS_OPEN, 
    STATUS_CLOSE, 
    STATUS_UNCLEAN,
     

} GuiStatus;

typedef enum {

    TYPE2MIDI,
    CORRUPTMIDI,
    ENDOFTRACKERROR,
    FILENOTFOUND,
    GENERALERROR,
    DONEPARSING,
    SIGPAUSE,
    SIGRESUME,
    SIGSETFRAC

} Signal;

#endif
