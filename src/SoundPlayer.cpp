// Plik: SoundPlayer.cpp

#include "SoundPlayer.h"

// --- Definicje melodii ---

// Prosta, trzydźwiękowa melodia startowa
const TuneNote startupSoundTune[] = {
  {NOTE_C5, 100}, {NOTE_REST, 70},
  {NOTE_E5, 100}, {NOTE_REST, 70},
  {NOTE_G5, 150}
};

// Melodia "Wlazł kotek na płotek"
// ZMIANA: Zwiększono czas trwania nut, aby melodia była wolniejsza i wyraźniejsza.
// Poprawna melodia "Wlazł kotek na płotek"
// Zakładamy: 200 = ćwierćnuta, 400 = półnuta

const TuneNote wlazlKotekTune[] = {
  // Wlazł ko-tek 
  {NOTE_C5, 200}, 
  {NOTE_C5, 200}, 
  // na pło-tek
  {NOTE_E5, 200}, 
  {NOTE_E5, 200}, 
  // i mru-ga
  {NOTE_G5, 200}, 
  {NOTE_G5, 200}, 
  {NOTE_E5, 400},  // Dłuższa nuta na "ga"

  // (Można dodać pauzę)
  // {NOTE_REST, 200},

  // Ład-na to
  {NOTE_F5, 200}, 
  {NOTE_F5, 200}, 
  // pio-sen-ka
  {NOTE_E5, 200}, 
  {NOTE_E5, 200}, 
  // nie dłu-ga
  {NOTE_D5, 200}, 
  {NOTE_D5, 200}, 
  {NOTE_C5, 400}   // Dłuższa nuta na "ga" i powrót do C
};


// Motyw z "Archiwum X"
const TuneNote xFilesThemeTune[] = {
  // Główne "gwizdzące" intro
  {NOTE_C4, 400}, // Czwarta oktawa, dla niskiego, mrocznego tonu
  {NOTE_REST, 100}, // Krótka pauza
  {NOTE_A4, 800},  // Piąta oktawa, wyraźny, wysoki ton
  {NOTE_REST, 100}, // Krótka pauza

  {NOTE_C4, 400},
  {NOTE_REST, 100},
  {NOTE_A4, 800},
  {NOTE_REST, 100},

  {NOTE_C4, 400},
  {NOTE_REST, 100},
  {NOTE_A4, 800},
  {NOTE_REST, 100},

  {NOTE_C4, 400},
  {NOTE_REST, 100},
  {NOTE_A4, 800},
  {NOTE_REST, 100},

  // Druga część, która dodaje więcej napięcia (uproszczona)
  // Zazwyczaj to są jakieś arpeggia, ale dla buzzer'a zrobimy to prościej
  {NOTE_D4, 200},
  {NOTE_E4, 200},
  {NOTE_F4, 200},
  {NOTE_E4, 200},
  {NOTE_D4, 400},
  {NOTE_C4, 400},
  {NOTE_REST, 200},

  {NOTE_D4, 200},
  {NOTE_E4, 200},
  {NOTE_F4, 200},
  {NOTE_E4, 200},
  {NOTE_D4, 400},
  {NOTE_C4, 400},
  {NOTE_REST, 200},
};


SoundPlayer::SoundPlayer(ControlPanel& controlPanel) : _controlPanel(&controlPanel) {}

void SoundPlayer::playStartupSound() {
  int tuneSize = sizeof(startupSoundTune) / sizeof(startupSoundTune[0]);
  _playTune(startupSoundTune, tuneSize);
}

void SoundPlayer::playXFilesTheme() {
  int tuneSize = sizeof(xFilesThemeTune) / sizeof(xFilesThemeTune[0]);
  _playTune(xFilesThemeTune, tuneSize);
}

void SoundPlayer::playWlazlKotek() {
  int tuneSize = sizeof(wlazlKotekTune) / sizeof(wlazlKotekTune[0]);
  _playTune(wlazlKotekTune, tuneSize, 400);
}

void SoundPlayer::_playTune(const TuneNote tune[], int tuneSize, int delayTime) {
  if (!_controlPanel) return;

  for (int i = 0; i < tuneSize; i++) {
    if (tune[i].note == NOTE_REST) {
      delay(tune[i].duration);
    } else {
      _controlPanel->beep(tune[i].note, tune[i].duration);
    }
    // ZMIANA: Używamy teraz przekazanego parametru 'delayTime' zamiast
    // sztywnej wartości, aby poprawnie kontrolować tempo.
    delay(delayTime); 
  }
}