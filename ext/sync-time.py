# Plik: sync_time.py
import ntplib
import datetime
import serial
import serial.tools.list_ports
import sys
import time

# --- Konfiguracja ---
BAUD_RATE = 9600
NTP_SERVER = 'pool.ntp.org'

# Lista znanych identyfikatorów VID:PID dla Arduino i klonów
KNOWN_DEVICES = {
    "2341:0042": "Arduino Mega 2560",
    "2341:0001": "Arduino Uno",
    "2A03:0042": "Arduino Mega 2560", # Nowsze oficjalne
    "1A86:7523": "CH340 (popularny klon)",
}

def find_arduino_port():
    """Automatycznie znajduje port COM, do którego podłączone jest Arduino."""
    ports = serial.tools.list_ports.comports()

    # Przeszukujemy porty, najpierw próbując dopasować po bardziej niezawodnych identyfikatorach VID/PID
    for port in ports:
        # Sprawdzamy, czy port ma VID i PID, zanim spróbujemy ich użyć
        if port.vid is not None and port.pid is not None:
            device_id = f"{port.vid:04X}:{port.pid:04X}"
            if device_id in KNOWN_DEVICES:
                print(f"Znaleziono urządzenie '{KNOWN_DEVICES[device_id]}' na porcie {port.device} (wg VID/PID)")
                return port.device

    # Jeśli nie znaleziono po VID/PID, próbujemy dopasować po opisie (mniej niezawodne, ale pomocne)
    for port in ports:
        if "Arduino" in port.description or "CH340" in port.description:
            print(f"Znaleziono urządzenie z opisem '{port.description}' na porcie {port.device} (wg opisu)")
            return port.device

    # Jeśli nic nie znaleziono, wyświetlamy pomocną listę wszystkich portów
    print("\nBŁĄD: Nie znaleziono podłączonego Arduino.")
    print("Dostępne porty:")
    for port in ports:
        # Bezpieczne formatowanie na wypadek braku VID/PID
        vid_str = f"{port.vid:04X}" if port.vid is not None else "N/A"
        pid_str = f"{port.pid:04X}" if port.pid is not None else "N/A"
        print(f" - {port.device}: {port.description} [VID:PID={vid_str}:{pid_str}]")
    return None


def get_ntp_time():
    """Pobiera czas z serwera NTP i zwraca go jako obiekt datetime w strefie UTC."""
    try:
        client = ntplib.NTPClient()
        response = client.request(NTP_SERVER, version=3)
        # Używamy datetime.fromtimestamp do precyzyjnej konwersji
        # Czas NTP jest w UTC, więc tworzymy obiekt datetime świadomy strefy czasowej
        return datetime.datetime.fromtimestamp(response.tx_time, datetime.timezone.utc)
    except Exception as e:
        print(f"Błąd podczas pobierania czasu NTP: {e}")
        return None

def main():
    """Główna funkcja skryptu."""
    print("Szukam portu Arduino...")
    serial_port = find_arduino_port() 
    if not serial_port:
        sys.exit(1)

    
    print(f"Pobieram aktualny czas z serwera NTP ({NTP_SERVER})...")
    ntp_time = get_ntp_time()
    
    if not ntp_time:
        sys.exit(1)

    # Konwertujemy czas UTC na lokalną strefę czasową komputera
    local_time = ntp_time.astimezone()
    print(f"Pobrany czas (strefa lokalna): {local_time.strftime('%Y-%m-%d %H:%M:%S')}")

    # Formatujemy czas do formatu oczekiwanego przez Arduino
    formatted_time = local_time.strftime('%Y-%m-%d,%H:%M:%S')

    # Tworzymy komendę do wysłania: "TIME:YYYY-MM-DD,HH:MM:SS"
    command = f"TIME:{formatted_time}\n"
    
   # serial_port = 'COM5'
    
    print(f"Próbuję wysłać komendę do Arduino na porcie {serial_port}...")
    print(f"Komenda: {command.strip()}")
    try:
        # Używamy dłuższego timeoutu i dodajemy opóźnienie po otwarciu portu
        with serial.Serial(serial_port, BAUD_RATE, timeout=5) as ser:
            # Otwarcie portu powoduje reset Arduino. Czekamy chwilę, aż się uruchomi.
            print("Port otwarty, czekam na gotowość Arduino...")
            time.sleep(3) # Zwiększono czas oczekiwania do 3 sekund dla pewności

            # KLUCZOWA ZMIANA: Czyszczenie bufora wejściowego, aby pozbyć się starych danych
            # (np. komunikatów startowych wysłanych przez Arduino podczas resetu).
            ser.reset_input_buffer()
            print("Bufor wejściowy wyczyszczony.")

            ser.write(command.encode('utf-8'))
            print("Komenda wysłana pomyślnie.")

            # Odczytujemy wszystkie linie odpowiedzi od Arduino, aż do potwierdzenia lub timeoutu
            print("Oczekiwanie na potwierdzenie od Arduino...")
            confirmation_received = False
            while True:
                try:
                    response_line = ser.readline().decode('utf-8').strip()
                    if response_line:
                        print(f"Arduino: {response_line}")
                        if "OK: Czas zsynchronizowany" in response_line:
                            confirmation_received = True
                            print("Potwierdzenie otrzymane. Kończę.")
                            break
                    elif not confirmation_received:
                        continue # Ignoruj puste linie, jeśli nie ma jeszcze potwierdzenia
                    else: # Jeśli jest pusta linia po potwierdzeniu, zakończ
                        break
                except UnicodeDecodeError:
                    print("Błąd dekodowania. Otrzymano nieprawidłowe dane.")
                    continue

            if not confirmation_received:
                print("Nie otrzymano potwierdzenia od Arduino (timeout).")
    except serial.SerialException as e:
        print(f"BŁĄD: Nie można otworzyć portu szeregowego {serial_port}.")
        print(f"Upewnij się, że wybrano poprawny port i nie jest on używany przez inny program (np. Monitor portu szeregowego w VSCode).")
        print(f"Szczegóły błędu: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
