# Ela Midi Viewer

A midi file viewer written in C using gtk.

dependencies:
  -all gtk dependencies
  -pulseaudio
  -alas-dev
  -libasound-dev
  
  the program caches the events in a file called cachedEvents.txt
  note that the application is not able to play multiple notes at the same time.
