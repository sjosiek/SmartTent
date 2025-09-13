# SmartTent

Projekt inteligentnego namiotu (lub stacji pogodowej) opartego na platformie Arduino. System monitoruje warunki otoczenia, zarządza energią i pozwala na sterowanie elementami wykonawczymi, takimi jak serwomechanizmy.

## 1. Opis Pinów (Arduino Mega)

Poniższa tabela przedstawia listę pinów wykorzystywanych przez projekt.

| Pin              | Numer | Opis                                                              | Moduł/Urządzenie             |
|------------------|-------|-------------------------------------------------------------------|------------------------------|
| **I2C**          | 20, 21| Magistrala I2C (SDA, SCL)                                         | LCD, BME280, RTC             |
| **SPI**          | 50-53 | Magistrala SPI (MISO, MOSI, SCK, CS)                              | Karta SD                     |
| **Przerwania**   | 2     | Pin alarmu z zegara RTC (SQW) - Przerwanie 0                      | Zegar DS3231 (RTC)           |
|                  | 3     | Czujnik dotykowy do wybudzania systemu - Przerwanie 1             | Czujnik dotykowy TTP223      |
| **Zasilanie**    | 4     | Sterowanie zasilaniem peryferiów (tranzystor/przekaźnik)          | Power Manager                |
| **Czujniki**     | 6     | Linia danych dla czujnika temperatury i wilgotności               | Czujnik DHT11/22             |
| **Wyświetlacze** | 22, 23| Piny CLK i DIO dla 4-cyfrowego wyświetlacza 7-segmentowego        | Wyświetlacz LED TM1637       |
| **Sun Tracker**  | 9     | Serwomechanizm osi poziomej (H)                                   | Sun Tracker                  |
|                  | 10    | Serwomechanizm osi pionowej (V)                                   | Sun Tracker                  |
|                  | A0    | Fotorezystor Dół-Lewo (DL)                                        | Sun Tracker                  |
|                  | A1    | Fotorezystor Góra-Lewo (TL)                                       | Sun Tracker                  |
|                  | A2    | Fotorezystor Góra-Prawo (TR)                                      | Sun Tracker                  |
|                  | A3    | Fotorezystor Dół-Prawo (DR)                                       | Sun Tracker                  |
|                  | A4    | Oś X joysticka do manualnego sterowania                           | Sun Tracker                  |
|                  | A5    | Oś Y joysticka do manualnego sterowania                           | Sun Tracker                  |
|                  | 35    | Przycisk (SW) joysticka                                           | Sun Tracker                  |
| **System**       | 13    | Wbudowana dioda LED (sygnalizacja pracy/błędu)                    | Arduino                      |

## 2. Konfiguracja i Sterowanie

Działaniem systemu można sterować na dwa sposoby: poprzez zmienne w kodzie źródłowym (`main.cpp`) oraz za pomocą pliku konfiguracyjnego `config.txt` na karcie SD.

### 2.1. Konfiguracja w kodzie (`main.cpp`)

Te ustawienia wymagają ponownej kompilacji i wgrania programu.

#### Główny tryb pracy

*   `SLEEP_MODE_ENABLED`
    *   **Opis:** Główny przełącznik trybu oszczędzania energii.
    *   **Wartości:**
        *   `true`: System będzie przechodził w stan uśpienia i wybudzał się cyklicznie lub za pomocą czujnika dotykowego.
        *   `false`: System będzie działał w trybie ciągłym, bez usypiania.

#### Konfiguracja modułu śledzenia słońca (Sun Tracker)

Konfiguracja odbywa się poprzez strukturę `trackerConfig`.

*   `servoVMinAngle` / `servoVMaxAngle`: Minimalny/maksymalny kąt wychylenia dla serwa pionowego.
*   `servoHMinAngle` / `servoHMaxAngle`: Minimalny/maksymalny kąt wychylenia dla serwa poziomego.
*   `performLdrCalibration`: (`true`/`false`) - Czy przeprowadzić automatyczną kalibrację fotorezystorów przy starcie.
*   `performServoCalibration`: (`true`/`false`) - Czy przeprowadzić automatyczną kalibrację serw (przejazd przez pełen zakres ruchu) przy starcie.
*   `performInitialSearch`: (`true`/`false`) - Czy po uruchomieniu system ma aktywnie szukać najjaśniejszego punktu.
*   `useJoystick`: (`true`/`false`) - Włącza/wyłącza manualne sterowanie za pomocą joysticka.
*   `ldrSensorsConnected`: (`true`/`false`) - Informuje system, czy fotorezystory są fizycznie podłączone. Ustaw na `false`, jeśli testujesz tylko ruch serw.
*   `enableServoMovement`: (`true`/`false`) - Globalna blokada ruchu serwomechanizmów.
*   `defaultServoSpeed`: Domyślna prędkość ruchu serw (im wyższa wartość, tym wolniejszy ruch).
*   `defaultTolerance`: Czułość trackera. Określa, jak duża musi być różnica w odczytach z fotorezystorów, aby wywołać ruch serw.
*   `runningUpdateIntervalMs`: Co ile milisekund tracker ma sprawdzać pozycję słońca i korygować ustawienie.

#### Konfiguracja serwomechanizmów

*   `servoConfigs[]`
    *   **Opis:** Tablica definiująca podłączone serwomechanizmy (poza tymi od Sun Trackera).
    *   **Przykład użycia:**
        ```cpp
        const ServoConfig servoConfigs[] = {
          { A8, "Wywietrznik" },
          { A9, "Klapa" }
        };
        ```

### 2.2. Konfiguracja w pliku `config.txt` (karta SD)

Te ustawienia są wczytywane przy starcie systemu i można je zmieniać bez potrzeby ponownej kompilacji kodu. Plik `config.txt` powinien mieć format `klucz=wartość`.

*   `active_mode_minutes`
    *   **Opis:** Czas (w minutach), przez który system pozostaje aktywny po wybudzeniu (w trybie `SLEEP_MODE_ENABLED`).
    *   **Przykład:** `active_mode_minutes=5`

*   `sensor_update_interval_ms`
    *   **Opis:** Interwał (w milisekundach), co jaki czas mają być odczytywane dane z czujników i zapisywane na karcie SD.
    *   **Przykład:** `sensor_update_interval_ms=60000` (co 1 minutę)

*   `led_brightness`
    *   **Opis:** Jasność wyświetlacza 7-segmentowego.
    *   **Wartości:** `0` (wyłączony) do `15` (najjaśniejszy).
    *   **Przykład:** `led_brightness=10`

*   `servo_pos_X`
    *   **Opis:** Docelowa pozycja (w stopniach) dla serwomechanizmu o indeksie `X` (np. `servo_pos_0`, `servo_pos_1`).
    *   **Przykład:** `servo_pos_0=90`

### 2.3. Sterowanie przez port szeregowy (Serial)

System nasłuchuje na komendy wysyłane przez port szeregowy.

*   **Synchronizacja czasu:**
    *   **Komenda:** `SYNC_TIME:YYYY-MM-DDTHH:MM:SS`
    *   **Opis:** Ustawia datę i godzinę w zegarze RTC.
    *   **Przykład:** `SYNC_TIME:2023-10-27T10:30:00`

*   **Ustawienie pozycji serwa:**
    *   **Komenda:** `SERVO:[index]:[pozycja]`
    *   **Opis:** Ustawia serwo o podanym indeksie w tablicy `servoConfigs` na zadaną pozycję.
    *   **Przykład:** `SERVO:0:120` (ustawi pierwsze zdefiniowane serwo na pozycję 120 stopni).

