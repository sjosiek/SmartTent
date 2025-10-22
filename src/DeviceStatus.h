#ifndef MODULE_STATUS_H
#define MODULE_STATUS_H

// Ogólny stan "zdrowia" modułu, używany przez logikę programu.
enum class ModuleHealth {
    UNKNOWN,      // Stan nieznany
    INITIALIZING, // W trakcie inicjalizacji
    OK,           // Działa poprawnie
    WARNING,      // Działa, ale z ostrzeżeniami (np. GPS szuka satelitów)
    ERROR,        // Krytyczny błąd, moduł nie działa
    DISABLED      // Celowo wyłączony
};

// Struktura przechowująca kompletny stan jednego modułu.
struct ModuleStatus {
    const char* name;         // Nazwa modułu (np. "GPS")
    ModuleHealth health;      // Stan dla logiki programu
    char statusText[16];      // Tekst statusu do wyświetlenia na LCD
};


#endif // MODULE_STATUS_H
