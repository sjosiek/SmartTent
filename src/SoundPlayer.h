// Plik: SoundPlayer.h

#ifndef SOUND_PLAYER_H
#define SOUND_PLAYER_H

#include "ControlPanel.h"

// Definicje nut dla 4 oktaw (plus pauza)
// Używane w Arduino 'tone()'

#define NOTE_REST    0

// Oktawa 4
#define NOTE_C4    262
#define NOTE_CS4   277  // C#
#define NOTE_D4    294
#define NOTE_DS4   311  // D#
#define NOTE_E4    330
#define NOTE_F4    349
#define NOTE_FS4   370  // F#
#define NOTE_G4    392
#define NOTE_GS4   415  // G#
#define NOTE_A4    440
#define NOTE_AS4   466  // A#
#define NOTE_B4    494

// Oktawa 5
#define NOTE_C5    523
#define NOTE_CS5   554  // C#
#define NOTE_D5    587
#define NOTE_DS5   622  // D#
#define NOTE_E5    659
#define NOTE_F5    698
#define NOTE_FS5   740  // F#
#define NOTE_G5    784
#define NOTE_GS5   831  // G#
#define NOTE_A5    880
#define NOTE_AS5   932  // A#
#define NOTE_B5    988

// Oktawa 6
#define NOTE_C6    1047
#define NOTE_CS6   1109 // C#
#define NOTE_D6    1175
#define NOTE_DS6   1245 // D#
#define NOTE_E6    1319
#define NOTE_F6    1397
#define NOTE_FS6   1480 // F#
#define NOTE_G6    1568
#define NOTE_GS6   1661 // G#
#define NOTE_A6    1760
#define NOTE_AS6   1865 // A#
#define NOTE_B6    1976

// Oktawa 7
#define NOTE_C7    2093
#define NOTE_CS7   2217 // C#
#define NOTE_D7    2349
#define NOTE_DS7   2489 // D#
#define NOTE_E7    2637
#define NOTE_F7    2794
#define NOTE_FS7   2960 // F#
#define NOTE_G7    3136
#define NOTE_GS7   3322 // G#
#define NOTE_A7    3520
#define NOTE_AS7   3729 // A#
#define NOTE_B7    3951


// Struktura do przechowywania nuty i jej czasu trwania
struct TuneNote {
  int note;
  int duration;
};

class SoundPlayer {
public:
  SoundPlayer(ControlPanel& controlPanel);

  void playStartupSound();
  void playXFilesTheme();
  void playWlazlKotek();
  // W przyszłości możesz tu dodać inne melodie, np. playAlarm()

private:
  void _playTune(const TuneNote tune[], int tuneSize, int delayTime = 100);

  ControlPanel* _controlPanel;
};


#endif // SOUND_PLAYER_H