# Upute za Izradu CDJ-100S MIDI Adaptera (Blue Pill)

Ovaj dokument sadrži upute za modifikaciju Pioneer CDJ-100S playera u MIDI kontroler koristeći STM32F103C8T6 (Blue Pill) razvojnu pločicu.

---

## ⚠️ UPOZORENJE

**Radite na vlastitu odgovornost!** Modifikacija zahtijeva lemljenje i rad s elektronikom. Postoji rizik od oštećenja opreme ako se ne pazi na naponske razine (posebno 5V vs 3.3V).

---

## 1. Potrebni Materijal

*   **Pioneer CDJ-100S** (Display board i kućište su ključni)
*   **STM32F103C8T6 (Blue Pill)** razvojna pločica
*   **ST-Link v2** programator
*   **Žice za spajanje** (tanke, fleksibilne)
*   **USB Kabel** (za spajanje na PC - MIDI)
*   **Otpornici** (za voltage divider na pitch slideru - vidi korak 5)

---

## 2. Priprema CDJ-100S

1.  **Rastavljanje**: Otvorite kućište CDJ-100S.
2.  **Uklanjanje viška**:
    *   Uklonite CD mehanizam (drive).
    *   Uklonite Main Board (velika ploča na dnu).
    *   **ZADRŽITE**:
        *   **Display Board** (ploča s ekranom i tipkama).
        *   **Trans Board** (ploča s napajanjem/transformatorom - opcionalno, ako ćete koristiti originalno napajanje, ali preporuka je napajanje preko USB-a za Blue Pill).

---

## 3. Priprema Blue Pill Pločice

1.  **Programiranje**: Prije ugradnje, flashajte firmware na Blue Pill.
    *   Spojite ST-Link (SWDIO, SWCLK, GND, 3.3V).
    *   Koristite PlatformIO (`pio run -t upload`) ili STM32CubeIDE.
2.  **Pin Headers**: Ako planirate direktno lemiti žice, možda je bolje ne lemiti pin headere na Blue Pill radi uštede prostora.

---

## 4. Spajanje (Wiring)

Ovo je najvažniji dio. Spajate signale s **CDJ-100S Display Boarda** na **Blue Pill**.

Koristite `Schematics/Connection_scheme.pdf` kao referencu za točke na Display boardu.

### 🔌 Napajanje

Možete napajati Blue Pill preko USB porta (ako je spojen na PC) ili preko 5V pina ako koristite interno napajanje CDJ-a.

| CDJ Display Board | Blue Pill | Napomena |
| :--- | :--- | :--- |
| **GNDS / GNDD** | **GND** | Zajednička masa (Ground) je OBAVEZNA! |
| **(Trans Board 5V)** | **5V** | **Samo** ako ne napajate preko USB-a |

### 🎹 Signali Tipki (Keyboard Matrix)

Ovi pinovi čitaju pritiske tipki (Play, Cue, Eject, itd.).

| CDJ Signal | Blue Pill Pin | Funkcija |
| :--- | :--- | :--- |
| **KD0** | **PB3** | Keyboard Data 0 |
| **KD1** | **PB4** | Keyboard Data 1 |
| **KD2** | **PB5** | Keyboard Data 2 |
| **S1** | **PB15** | Scan Line 1 |
| **S2** | **PA8** | Scan Line 2 |
| **S3** | **PB12** | Scan Line 3 |
| **S4** | **PB13** | Scan Line 4 |
| **S5** | **PB14** | Scan Line 5 |

### 💿 Jog Wheel (Enkoder)

Spojite direktno s optičkih senzora jog wheel-a.

| CDJ Signal | Blue Pill Pin | Funkcija |
| :--- | :--- | :--- |
| **JOG1** | **PB6** | Encoder Phase A (TIM4_CH1) |
| **JOG2** | **PB7** | Encoder Phase B (TIM4_CH2) |

### 🎚️ Pitch Slider (ADIN) - ⚠️ PAŽNJA!

Originalni CDJ-100S Pitch Slider radi na **5V**. STM32F103 ADC ulaz tolerira **maksimalno 3.3V**.

**MORAŠ MODIFICIRATI NAPAJANJE SLIDERA!**

1.  **Prekini vod** na Display boardu koji dovodi 5V na Pitch Slider.
2.  Spoji taj pin slidera na **3.3V** izlaz s Blue Pill pločice.
    *   *Alternativa*: Koristi otpornički djelitelj napona (Voltage Divider) na signalnoj liniji (npr. 10k + 20k otpornici) da spustiš 5V signal na 3.3V.

| CDJ Signal | Blue Pill Pin | Funkcija |
| :--- | :--- | :--- |
| **ADIN** | **PA0** | Analog Input (0-3.3V MAX!) |
| **CT** | **PA1** | Control Timing (opcionalno) |

> [!TIP]
> **Softversko filtriranje (EMA filtar)**: U najnovijem firmwareu implementiran je napredni **Exponential Moving Average (EMA)** niskopropusni filtar ($\alpha = 0.25$) koji u potpunosti uklanja mikro-šumove i analogno drhtanje (jitter) napona na PA0. Klizanje pitcha je sada izuzetno glatko u Rekordboxu i Traktoru bez potrebe za hardverskim kondenzatorima za filtriranje!

### 💡 LED Feedback

| CDJ Signal | Blue Pill Pin | Funkcija |
| :--- | :--- | :--- |
| **CUE (LED)** | **PB2** | Cue LED |
| **PLAY (LED)** | **PB10** | Play LED |
| **DISC (LED)** | **PB11** | Disc In LED |

---

## 5. Spajanje na Display Board (STM32F746)

Ako koristite ovaj adapter zajedno s našim custom **Display Boardom** (STM32F746), trebate spojiti SPI linije.

| Blue Pill (Master) | STM32F746 (Slave) | Signal | Napomena |
| :--- | :--- | :--- | :--- |
| **PA5** | **PI1** | **SCK** | Clock linija |
| **PA7** | **PB15** | **MOSI** | Master Out Slave In |
| **PA6** | **PB14** | **MISO** | Master In Slave Out |
| **GND** | **GND** | **GND** | Obavezno spojiti mase! |

---

## 6. USB MIDI (PC Povezivanje)

Blue Pill ima USB port.

*   Spojite Blue Pill USB port na PC.
*   PC bi trebao prepoznati uređaj kao "STM32 MIDI" ili "CDJ-100S MIDI".
*   Otvorite VirtualDJ, Traktor ili Rekordbox i mapirajte kontrole.

---

## 7. Montaža u Kućište

1.  Fiksirajte Blue Pill unutar kućišta (plastične vezice, vruće ljepilo ili vijci).
2.  Osigurajte da nema kratkih spojeva s metalnim dijelovima kućišta.
3.  Ako koristite USB kabel, provedite ga kroz otvor stražnjeg panela.

---

## 8. Testiranje

1.  Upalite uređaj.
2.  Pritisnite `PLAY` - LED bi trebao zasvijetliti (ako je Display board spojen i vraća feedback).
3.  Provjerite MIDI input na PC-u (koristite MIDI-OX ili sličan softver).
4.  Pomaknite Pitch Slider - vrijednost bi se trebala glatko mijenjati.
5.  Zavrtite Jog Wheel - trebali biste vidjeti promjene vrijednosti.

---

**Sretno s modifikacijom!**
