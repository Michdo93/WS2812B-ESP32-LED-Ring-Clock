# WS2812B ESP32 LED Ring Clock

Ein Do-It-Yourself (DIY) Uhr-Projekt basierend auf einem **60-LED WS2812B Ring**, gesteuert von einem **ESP32** Mikrocontroller. Die Uhrzeit wird über **NTP (Network Time Protocol)** aus dem Internet bezogen und auf einem **DS3231 Echtzeituhr-Modul (RTC)** gespeichert, sodass die Uhr auch ohne dauerhafte Internetverbindung präzise weiterläuft.

---

## 🛠️ Hardware-Komponenten

| Bauteil | Menge | Beschreibung |
| :--- | :---: | :--- |
| **ESP32 NodeMCU Board** | 1 | Haupt-Mikrocontroller (Wi-Fi & Bluetooth) |
| **WS2812B LED Ring (60 LEDs, 5V)** | 1 | 60 einzeln adressierbare RGB-LEDs |
| **DS3231 / AT24C32 RTC Modul** | 1 | I2C-Echtzeituhr-Modul mit CR2032 Backup-Batterie |
| **Elektrolytkondensator (1000 µF / 25V)** | 1 | Glättungskondensator zum Schutz der LEDs vor Einschaltspitzen |
| **Widerstand 330 Ω** | 1 | Schutzwiderstand für die LED-Datenleitung (330 Ω – 470 Ω) |
| **5V USB Netzteil** | 1 | Empfohlen: Min. 5V / 2A (z. B. Smartphone-Steckernetzteil) |
| **Jumper Wire / Litzendraht** | ca. 10 | Für Steckbrett-Aufbau oder feste Verlötung |

---

## 📐 Schaltplan & Verdrahtung

### Anschluss-Übersicht

```
                    +-----------------------+
                    |    5V / 2A Netzteil   |
                    +-----------+-----------+
                                |
                   +------------+------------+
                   |                         |
               +---v---+                 +---v---+
               |  VIN  |                 |  VCC  |
               | ESP32 |                 |  RTC  |
               +---+---+                 +---+---+
                   |                         |
                   +------------+------------+
                                |
                         +------v------+
                         |  GND (ESP)  |
                         +------+------+
                                |
            +-------------------+-------------------+
            |                                       |
    +-------v-------+                       +-------v-------+
    | Kondensator + |                       | Kondensator - |
    +-------+-------+                       +-------+-------+
            |                                       |
     (5V am LED Ring)                       (GND am LED Ring)
```

![LED Clock](https://github.com/Michdo93/WS2812B-ESP32-LED-Ring-Clock/blob/main/led_clock.jpeg?raw=true)

### Pin-Mapping

| Bauteil | Pin am Bauteil | Verbindet mit | ESP32 Pin / Anmerkung |
| :--- | :--- | :--- | :--- |
| **WS2812B Ring (DIN)** | `5V` / `VCC` (Rot) | $\rightarrow$ | **VIN / 5V** (Zusammen mit Kondensator `+`) |
| **WS2812B Ring (DIN)** | `GND` (Weiß/Schwarz) | $\rightarrow$ | **GND** (Zusammen mit Kondensator `-`) |
| **WS2812B Ring (DIN)** | `DI` / `DIN` (Grün) | $\rightarrow$ | **GPIO 18** *(330 Ω Widerstand dazwischen schalten!)* |
| **RTC Modul** | `VCC` | $\rightarrow$ | **3.3V** (oder 5V) |
| **RTC Modul** | `GND` | $\rightarrow$ | **GND** |
| **RTC Modul** | `SDA` | $\rightarrow$ | **GPIO 21** (I2C Data) |
| **RTC Modul** | `SCL` | $\rightarrow$ | **GPIO 22** (I2C Clock) |

> ⚠️ **WICHTIG (Signalrichtung am LED-Ring):** 
> Der LED-Ring hat zwei Kabelstränge. Benutze unbedingt den Kabelstrang, dessen Lötstellen auf der Platine mit **`IN`**, **`DI`** oder **`DIN`** beschriftet sind. Der Strang mit `OUT`/`DO` bleibt unangeschlossen!

> ⚠️ **WICHTIG (Kondensator-Polarität):**
> Der 1000 µF Elko muss korrekt gepolt werden: Das **lange Bein (+)** kommt an 5V/VIN, das **kurze Bein (-)** an GND. Bei Falschpolung kann der Kondensator beschädigt werden.

---

## 💻 Software & Flashen (Anleitung)

### Methode: Arduino IDE

1. **Arduino IDE installieren:**
   Lade die aktuelle Version der [Arduino IDE](https://www.arduino.cc/en/software) herunter.

2. **ESP32 Board-Support hinzufügen:**
   * Öffne in der Arduino IDE: `Datei` $\rightarrow$ `Voreinstellungen`.
   * Trage bei **Zusätzliche Boardverwalter-URLs** Folgendes ein:
     `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   * Gehe zu `Werkzeuge` $\rightarrow$ `Board` $\rightarrow$ `Boardverwalter`, suche nach **esp32** und klicke auf **Installieren**.

3. **Bibliotheken installieren:**
   Gehe zu `Werkzeuge` $\rightarrow$ `Bibliotheken verwalten` und installiere:
   * **Adafruit NeoPixel** (von Adafruit)
   * **RTClib** (von Adafruit)

4. **Code anpassen:**
   * Öffne die Datei `src/main.cpp`.
   * Passe deine WLAN-Zugangsdaten an:
     ```cpp
     const char* WIFI_SSID     = "DEIN_WLAN_NAME";
     const char* WIFI_PASSWORD = "DEIN_WLAN_PASSWORT";
     ```

5. **Flashen:**
   * Verbinde den ESP32 per USB-Kabel mit dem PC.
   * Wähle unter `Werkzeuge` $\rightarrow$ `Board` dein ESP32-Modell (z. B. **ESP32 Dev Module**).
   * Wähle den passenden **COM-Port** aus.
   * Klicke auf **Hochladen** (Pfeil-Symbol).

---

## 🎨 Funktionsweise der Anzeige

* **Stundenzeiger:** Rot (Rückt alle 12 Minuten um 1 LED weiter)
* **Minutenzeiger:** Grün
* **Sekundenzeiger:** Blau
* **Stundenmarkierungen:** Fünf-Minuten-Raster schwach weiß gedimmt ($0, 5, 10, \dots, 55$).
* **Überlappung:**
  * Stunde + Minute = **Gelb**
  * Stunde + Sekunde = **Magenta**
  * Minute + Sekunde = **Cyan**
