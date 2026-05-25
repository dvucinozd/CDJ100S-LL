# CDJ-100S MIDI Controller Integration

## Pregled

CDJ-100S Display projekt **već ima integriranu podršku** za MIDI kontroler preko **SPI2 komunikacije**. MIDI kontroler je zasebni hardware koji prevodi CDJ-100S hardverske kontrole (jog, slider, tipke) u komande koje STM32F746 Display board razumije.

---

## Arhitektura Sistema

```
┌──────────────────────┐
│  CDJ-100S Hardware   │  ← Originalni jog wheel, slider, tipke
│  (mehanički input)   │
└──────────┬───────────┘
           │
           │ CDJ-100S proprietary protocol
           ▼
┌──────────────────────┐
│ MIDI Controller      │  ← STM32F103C8T6/CBT6
│ (Translator Board)   │
└──────┬──────┬────────┘
       │      │
       │      ├─────────► USB MIDI (PC/DAW) - opciono
       │
       │ SPI2 (Master)
       ▼
┌──────────────────────┐
│ STM32F746 Display    │  ← Ovaj projekt
│ (Audio Player)       │
└──────────────────────┘
```

---

## SPI2 Komunikacija

### Hardware Setup

**Display Board (STM32F746) - Slave Mode**
- **PI1** → SPI2_SCK (Clock)
- **PB14** → SPI2_MISO (Display → MIDI Controller)
- **PB15** → SPI2_MOSI (MIDI Controller → Display)

**MIDI Controller - Master Mode**
- Generiše clock i inicira transfere
- Šalje CDJ kontrolne komande
- Prima status/feedback od Display boarda

### Konfiguracija ([`spi.c`](file:///d:/AI/CDJ100/Src/spi.c))

```c
hspi2.Instance = SPI2;
hspi2.Init.Mode = SPI_MODE_SLAVE;         // Display je slave!
hspi2.Init.CLKPolarity = SPI_POLARITY_HIGH;
hspi2.Init.CLKPhase = SPI_PHASE_2EDGE;
hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
```

**Interrupt Priority**: `2` (viša od displaya, niža od audio DMA)

---

## SPI Protokol - Struktura Paketa

### Packet Format (4 bajta)

```
┌────────┬─────────┬─────────┬─────────┐
│ Byte 0 │ Byte 1  │ Byte 2  │ Byte 3  │
│ 0x08   │ Command │  Data1  │  Data2  │
└────────┴─────────┴─────────┴─────────┘
```

### RX Buffer Analiza ([`main.c`](file:///d:/AI/CDJ100/Src/main.c))

```c
uint8_t spi_rx[4] = {0};  // Primanje od MIDI kontrolera
uint8_t spi_tx[4] = {0};  // Slanje ka MIDI kontroleru
```

#### Byte 1 - Command Type

| Vrijednost | Značenje |
|------------|----------|
| `0x90` | Button Press Event |
| `0x80` | Button Release Event |
| `0xB0` | Continuous Controller (Pitch/Jog) |

#### Byte 2 - Button/Controller ID

Definisano u header datotekama (primjer iz koda):

| ID | Naziv | Funkcija |
|----|-------|----------|
| `JET` | Jet button | Set loop start |
| `JOG` | Jog wheel press | Track selection mode |
| `ZIP` | Zip button | Set loop end |
| `WAH` | WAH button | Toggle effect / hot cue |
| `HOLD` | Hold button | Toggle hold mode |
| `TIME` | Time button | Time display mode |
| `MASTERTEMPO` | Master Tempo | Enable/disable master tempo |
| `TRACKBACK` | Track << | Previous track / loop adjust |
| `TRACKFORWARD` | Track >> | Next track / loop adjust |
| `SCANBACK` | Scan << | Fast rewind |
| `SCANFORWARD` | Scan >> | Fast forward |
| `PLAY` | Play/Pause | Playback control |
| `CUE` | Cue | Set/go to cue point |

---

## Implementacija - Key Handlers

### Button Press Handler (Asinkroni i Razdvojeni)
U najnovijoj arhitekturi, **`HAL_SPI_TxRxCpltCallback`** je sveden na minimalno dodavanje primljenih paketa u kružni red spremnika kako se ne bi blokirali audio i ostali interrupti. Cjelokupna obrada switch-case naredbi za tipke i pitch fader izvršava se u asinkronoj pozadinskoj funkciji **`ProcessPendingSPIEvents()`** u petlji `while(1)` glavne aplikacije:

```c
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
  if (spi_rx[0] != 0 || spi_rx[1] != 0 || spi_rx[2] != 0 || spi_rx[3] != 0) {
    uint8_t next_in = (spi_event_in + 1) % SPI_EVENT_QUEUE_SIZE;
    if (next_in != spi_event_out) {
      spi_event_queue[spi_event_in].rx_bytes[0] = spi_rx[0];
      spi_event_queue[spi_event_in].rx_bytes[1] = spi_rx[1];
      spi_event_queue[spi_event_in].rx_bytes[2] = spi_rx[2];
      spi_event_queue[spi_event_in].rx_bytes[3] = spi_rx[3];
      spi_event_in = next_in;
    }
  }
  for (int i = 0; i < 4; i++)
    spi_rx[i] = 0;
  HAL_SPI_TransmitReceive_IT(&hspi2, spi_tx, spi_rx, 4);
}

void ProcessPendingSPIEvents(void) {
  while (spi_event_out != spi_event_in) {
    uint8_t spi_rx_temp[4];
    spi_rx_temp[0] = spi_event_queue[spi_event_out].rx_bytes[0];
    spi_rx_temp[1] = spi_event_queue[spi_event_out].rx_bytes[1];
    spi_rx_temp[2] = spi_event_queue[spi_event_out].rx_bytes[2];
    spi_rx_temp[3] = spi_event_queue[spi_event_out].rx_bytes[3];
    uint8_t *spi_rx = spi_rx_temp;

    switch (spi_rx[1] & 0xF0) {
      case 0x90: {  // Button Press
        switch (spi_rx[2]) {
          case JET:
            // Set loop start point
            if (display.quantize == 1) {
              display.loopstart = QuantizePosition(0);
            } else {
              display.loopstart = file_pos_wide;
            }
            break;
          // ... [i drugi gumbi]
        }
        break;
      }
      // ... [Pitch i ostale komande]
    }
    spi_event_out = (spi_event_out + 1) % SPI_EVENT_QUEUE_SIZE;
  }
}
```

### Feedback ka MIDI Kontroleru

Display board šalje status nazad preko `spi_tx`:

```c
// Primjer: LED status update
if (!spi_tx[2] & (1 << 0))
  spi_tx[2] |= (1 << 1);   // Set LED bit
else
  spi_tx[2] &= ~(1 << 1);  // Clear LED bit
```

---

## MIDI Controller Hardware

### GitHub Repozitorij

✅ **DOSTUPAN**: [dvucinozd/CDJ100S-LL-MIDI](https://github.com/dvucinozd/CDJ100S-LL-MIDI)

**Autor**: Ruslan Terentiev (spectran)

### Sadržaj Repozitorija

#### 📁 Datoteke i Projekti

| Datoteka/Folder | Opis |
|----------------|------|
| **STM32CubeIDE projekt** | Kompletna firmware za STM32F103C8T6 |
| **Schematics/** | Električne sheme (.jpg, .sch format) |
| **Connection_scheme.pdf** | Shema povezivanja sa CDJ-100S Display boardom |
| **Sprint Layout (.lay)** | PCB layout u Sprint Layout formatu |
| **VirtualDJ configs (.xml)** | Primjeri konfiguracija za VirtualDJ |
| **Instructions.pdf** | Detaljna uputstva za montažu |

#### 🎥 Video Demo
[YouTube - CDJ-100S MIDI Controller](https://www.youtube.com/watch?v=wtE2o-FcW-4)

### Montaža - Koraci (iz README.md)

1. **Demontaža CDJ-100S**
   - Ukloni CD drive i Main board (neće se koristiti)

2. **Montaža Custom Boarda**
   - Uklopiti u kućište CDJ-100S
   - Preporučeno: USB jack na zadnjoj ploči

3. **Napajanje**
   - Spoji **GNDD** i **V+5V** sa Trans boarda na Custom board
   - Spoji **GNDD** i **GNDS** na Display boardu

4. **Signal Connections** (prema Connection_scheme.pdf)
   - **V+5V, GNDD** - napajanje
   - **JOG1-2** - jog wheel signali
   - **S1-S5** - switch signali
   - **KD0-2** - keyboard signali
   - **CUE, PLAY, DISC** - LED signali
   - **ADIN** - analog input (pitch slider)
   - **CT** - control signal

5. **⚠️ VAŽNO: Pitch Slider Voltage**
   - **Prekini** vezu V+5V → pitch slider na Display boardu
   - **Spoji** pitch slider +5V → **+3.3V** Custom boarda
   - Razlog: STM32F103 ADC radi 0-3.3V (ne 0-5V!)

6. **Programiranje**
   - Program STM32F103C8T6 preko **SWD**
   - Restart CDJ-100S

7. **✅ Gotovo!**
   - CDJ-100S je sada MIDI kontroler!

---

## Kompatibilnost s USB Host Verzijom

### ✅ USB Host Integracija NE Utiče na MIDI

SPI2 i USB_OTG_FS koriste **različite pinove**:

**SPI2 Pinovi** (MIDI Controller):
- PI1, PB14, PB15

**USB_OTG_FS Pinovi** (USB Audio):
- PA9, PA10, PA11, PA12

**Zaključak**: MIDI kontroler i USB Host **mogu raditi istovremeno** bez problema!

---

## DIY MIDI Controller - Opcije

Ako nemaš gotov MIDI kontroler, imaš nekoliko opcija:

### Opcija 1: Arduino/Blue Pill SPI Bridge

```c
// Pseudo-code za Blue Pill (STM32F103)
void loop() {
  // Čitaj CDJ-100S hardver (GPIO/ADC)
  uint8_t button = readCDJButton();
  uint16_t pitch = readPitchSlider();
  
  // Formiraj SPI paket
  uint8_t tx[4];
  tx[0] = 0x00;
  tx[1] = 0x90;  // Button press
  tx[2] = button;
  tx[3] = 0x00;
  
  // Pošalji na Display board
  SPIMaster.transfer(tx, 4);
}
```

### Opcija 2: Direktno Povezivanje (bez MIDI)

Ako ne trebaš USB MIDI funkcionalnost, možeš spojiti CDJ-100S hardver **direktno na STM32F746 GPIO pinove** i čitati ih u aplikaciji.

### Opcija 3: Koristi Touchscreen Ekskluzi vno

Trenutni kod već podržava touchscreen kontrolu - možeš koristiti projekt **bez fizičkih CDJ kontrola**.

---

## Testing & Debugging

### SPI Monitoring

Dodaj debug output u SPI callback:

```c
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
  // Debug print
  printf("SPI RX: %02X %02X %02X %02X\n", 
         spi_rx[0], spi_rx[1], spi_rx[2], spi_rx[3]);
  
  // ... normalna logika
}
```

### Loopback Test

Kratko spoji MOSI i MISO (PB15 ↔ PB14) za self-test:

```c
// U main.c
HAL_SPI_TransmitReceive_IT(&hspi2, spi_tx, spi_rx, 4);

// Provjeri da spi_rx == spi_tx
if (memcmp(spi_rx, spi_tx, 4) == 0) {
  printf("SPI Loopback OK\n");
}
```

---

## Sljedeći Koraci

1. **Ako imaš MIDI Controller Hardware**:
   - Spoji SPI2 pinove (PI1, PB14, PB15)
   - Upload firmware na MIDI kontroler
   - Testiraj button responses

2. **Ako nemaš MIDI Controller**:
   - Koristi touchscreen kontrolu
   - Ili implementiraj DIY bridge (Arduino/Blue Pill)

3. **Za USB MIDI Funkcionalnost**:
   - MIDI kontroler može slati USB MIDI ka PC-u paralelno
   - Koristi Rekordbox/DAW integraciju

---

## Reference

- [SPI Configuration](file:///d:/AI/CDJ100/Src/spi.c)
- [SPI Interrupt Handler](file:///d:/AI/CDJ100/Src/stm32f7xx_it.c#L607)
- [Main Initialization](file:///d:/AI/CDJ100/Src/main.c#L247)
- [Original CDJ-100S Display Repo](https://github.com/spectran/CDJ-100S-STM32F7-Display)

---

**Status**: ✅ MIDI kontroler **kompatibilan** s USB Host verzijom projekta!
