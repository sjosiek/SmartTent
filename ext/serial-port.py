import sys
import serial
import serial.tools.list_ports

# --- SŁOWNIK ZNANYCH URZĄDZEŃ (VID:PID) ---
# Możesz tu dodać więcej identyfikatorów dla swoich urządzeń
KNOWN_DEVICES = {
    # Oryginalne Arduino
    "2341:0043": "Arduino Uno",
    "2341:0042": "Arduino Mega",
    "2341:0001": "Arduino Uno (Rev3)",
    "2A03:0043": "Arduino Uno (Nowsze)",
    # Klony i inne popularne układy
    "1A86:7523": "CH340 (Klon Arduino)",
    "10C4:EA60": "CP210x (np. ESP32)",
}

# --- KONFIGURACJA ---
# Upewnij się, że prędkość jest taka sama jak w Twoim kodzie Arduino (np. Serial.begin(9600))
BAUDRATE = 9600
# --------------------

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
    print("\nBŁĄD: Nie znaleziono podłączonego Arduino lub innego znanego urządzenia.")
    print("Dostępne porty:")
    if not ports:
        print(" - Brak dostępnych portów szeregowych.")
    else:
        for port in ports:
            # Bezpieczne formatowanie na wypadek braku VID/PID
            vid_str = f"{port.vid:04X}" if port.vid is not None else "N/A"
            pid_str = f"{port.pid:04X}" if port.pid is not None else "N/A"
            print(f" - {port.device}: {port.description} [VID:PID={vid_str}:{pid_str}]")
    return None

# --- GŁÓWNA CZĘŚĆ SKRYPTU ---
if __name__ == "__main__":
    # Krok 1: Automatycznie znajdź port
    port_name = find_arduino_port()

    # Krok 2: Jeśli port nie został znaleziony, zakończ program
    if not port_name:
        sys.exit(1)

    # Krok 3: Spróbuj połączyć się i odczytywać dane
    ser = None  # Inicjalizujemy zmienną ser
    try:
        # WAŻNE: dtr=False zapobiega restartowaniu się Arduino po połączeniu!
        # Najpierw tworzymy obiekt bez argumentu dtr
        ser = serial.Serial(port_name, BAUDRATE, timeout=1) 
        # A następnie ustawiamy właściwość dtr na False
        ser.dtr = False
        print(f"Nasłuchiwanie portu {port_name}... Naciśnij Ctrl+C, aby zakończyć.")

        while True:
            line = ser.readline()
            if line:
                print(line.decode('utf-8', errors='ignore').strip())

    except serial.SerialException as e:
        print(f"BŁĄD: Nie można otworzyć portu {port_name}. Może jest używany przez inny program?")
        print(e)
        sys.exit(1)
    except KeyboardInterrupt:
        print("\nZamykanie programu.")
    finally:
        if ser and ser.is_open:
            ser.close()
            print("Port szeregowy został zamknięty.")