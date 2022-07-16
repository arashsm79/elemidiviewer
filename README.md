# Ela Midi Viewer
> A Simple midi viewer with minimum playing capabilities

A MIDI file viewer written in C using gtk.
This is also a great tool for error-checking MIDI files.

* [Introduction](#Introduction)
* [Screenshots](#Screenshots)
* [Dependencies](#Dependencies)
* [Building](#Building)

# Introduction
Ele Midi Viewer is a simple MIDI parser tool, written in C. The GUI has been written with gtk.
It supports virtually all Midi file events except SysEx events (it only shows the length of sysex events).
Other than just parsing the file, Ele is able to play simple MIDI files that don't have notes playing simultaneously. (some test cases have been added to the source folder, like forelise, believer, the dance of the sugar plum fairy, and ...
All parsed events are stored in a file named cachedEvents.txt. The source code is riddled with comments and is pretty self-explanatory.

MIDI events that are not standard or are out of the standard range, get colored with red and have the word "ERR" at the end of them.

If the program failed to open slave (for playing the file), make sure no other program is using the audio card and if the problem persists check out the link below:

[alsa-unable-to-open-slave-38on](https://dev.to/setevoy/linux-alsa-lib-pcmdmixc1108sndpcmdmixopen-unable-to-open-slave-38on)

# Screenshots
![Alt text](/screenshots/2.png?raw=true "Error checking features")
![Alt text](/screenshots/1.png?raw=true "Main Window")
![Alt text](/screenshots/3.png?raw=true "Progress Bar")
![Alt text](/screenshots/4.png?raw=true "Error checking features")
![Alt text](/screenshots/5.png?raw=true "Error checking features")

# Dependencies
* GTK 3
* pulseaudio
* alsa-utils
* libasound2-dev
  
# Building
  Compile the project using the following command:
  ```bash
  gcc -o ElaMidiViewer.o main.c midiparser.c beep.o sintable.o intro.c -lasound -lm `pkg-config --cflags --libs gtk+-3.0` -export-dynamic
  ```
