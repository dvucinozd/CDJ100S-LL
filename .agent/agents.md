# CDJ-100S STM32F7 Display - AI Agent Development Guide

## Pregled Projekta

**Tip**: Embedded audio player za Pioneer CDJ-100S DJ konzolu  
**MCU**: STM32F746NGH6 (ARM Cortex-M7, 216MHz)  
**Ploča**: STM32F746G-Discovery  
**Storage**: USB Mass Storage (zamijenjena SD kartica)  
**Audio Format**: WAV (stabilan), MP3 (Helix dekoder - može zapeti na rewind)  
**Playlist**: Rekordbox export format

---

## Arhitektura - Ključni Slojevi

```
┌─────────────────────────────────────────┐
│  Aplikacija (main.c, mp3player.c, etc.) │
├─────────────────────────────────────────┤
│  FatFS (ff.c) - File System             │
├─────────────────────────────────────────┤
│  USB Disk I/O (usbh_diskio.c)           │
├─────────────────────────────────────────┤
│  USB Host MSC Class (usbh_msc.c)        │
├─────────────────────────────────────────┤
│  USB Host Core (usbh_core.c)            │
├─────────────────────────────────────────┤
│  HAL HCD (stm32f7xx_hal_hcd.c)          │
├─────────────────────────────────────────┤
│  LL USB (stm32f7xx_ll_usb.c)            │
├─────────────────────────────────────────┤
│  Hardware: USB_OTG_FS (PA11/PA12)       │
└─────────────────────────────────────────┘
```

---

## Struktura Direktorija

```
CDJ100S-LL/
├── Drivers/
│   ├── STM32F7xx_HAL_Driver/     # HAL driveri (nikada ne mijenjaj!)
│   └── CMSIS/                    # ARM CMSIS headers (read-only)
│
├── Middlewares/
│   ├── ST/
│   │   └── STM32_USB_Host_Library/  # Službena USB Host lib (klonirana)
│   └── Third_Party/
│       ├── FatFs/                   # File system (ne dirај)
│       └── Helix/                   # MP3 dekoder (buggy rewind!)
│
├── Src/                          # GLAVNI APPLICATION KOD
│   ├── main.c                    # Entry point, file scanning
│   ├── usb_host.c                # USB Host inicijalizacija
│   ├── usbh_conf.c               # USB HAL callbacks
│   ├── usbh_diskio.c             # FatFS↔USB bridge
│   ├── mp3player.c               # MP3 playback (Helix)
│   ├── waveplayer.c              # WAV playback
│   ├── rekordbox.c               # Rekordbox metadata parser
│   ├── display.c                 # LCD rendering
│   └── stm32f7xx_it.c            # Interrupt handlers
│
├── Inc/                          # Headers za Src/
├── FFT/                          # FFT za waveform display
├── startup/                      # Assembly startup kod
├── platformio.ini                # PlatformIO build config
├── STM32F746NGHx_FLASH.ld        # Linker script
└── .agent/
    └── agents.md                 # 👈 Ovaj dokument
```

---

## Kritična Pravila za Development

### ⚠️ NEMOJ DIRATI

1. **HAL Driveri** (`Drivers/STM32F7xx_HAL_Driver/`)
   - Ovo je službeni ST kod - **nikada ne modificiraj**
   - Ako fali driver, preuzmi sa [GitHub](https://github.com/STMicroelectronics/stm32f7xx_hal_driver)

2. **USB Host Library** (`Middlewares/ST/STM32_USB_Host_Library/`)
   - Klonirano sa GitHub-a - **read-only**
   - Ako treba update: `git pull` u tom folderu

3. **FatFS** (`Middlewares/Third_Party/FatFs/`)
   - Chan's FatFS - stabilan, ne dirај osim ako nužno

4. **Linker Script** (`STM32F746NGHx_FLASH.ld`)
   - Ako ne znaš što radiš, **NEMOJ** modificirati
   - Flash: 1MB @ 0x08000000, RAM: 320KB @ 0x20000000

### ✅ SLOBODNO MIJENJAJ

1. **Application Kod** (`Src/*.c`)
   - Ovo je custom logika - mijenjaj koliko hoćeš
   
2. **Display Logic** (`Src/display.c`)
   - UI rendering, provjeri `menu_mode` state machine

3. **Audio Players** (`mp3player.c`, `waveplayer.c`)
   - **Poznati bug**: MP3 rewind može zapeti (Helix problem)

4. **USB Disk I/O** (`usbh_diskio.c`)
   - Ako dodaješ caching/optimizaciju

---

## Build Workflow

### PlatformIO (PREPORUČENO)

```bash
# Clean
pio run -t clean

# Compile
pio run

# Upload
pio run -t upload

# Serial monitor (ako treba debug)
pio device monitor
```

**Konfig**: [`platformio.ini`](file:///d:/AI/CDJ100/platformio.ini)

### STM32CubeIDE (Alternative)

1. Otvori: File → Import → Existing Projects → `d:\AI\CDJ100`
2. Build: Ctrl+B
3. Flash: F11

**Konfig**: [`.cproject`](file:///d:/AI/CDJ100/.cproject)

---

## Debugging Tips

### USB Ne Detektira Stick

**Simptomi**: `Appli_state` ostaje na `APPLICATION_IDLE`

**Check:**
```c
// U usb_host.c, dodaj debug output
void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id) {
  printf("USB Event: %d\n", id);  // 0=disconnect, 1=connect, 2=class_active
}
```

**Uzroci:**
- VBUS nije spojen (PA9)
- USB stick ne podržava Full Speed
- Loš USB kabel

### FatFS Mount Fails

**Simptomi**: `f_mount()` vraća != FR_OK

**Check:**
```c
// U main.c
FRESULT res = f_mount(&USBFatFs, USBPath, 0);
if (res != FR_OK) {
  // FR_DISK_ERR (1) = diskio problem
  // FR_NOT_READY (3) = USB nije ready
  // FR_NO_FILESYSTEM (13) = nije FAT32/exFAT
}
```

**Rješenja:**
1. Formatiraj USB kao FAT32 (`format E: /FS:FAT32`)
2. Provjeri `USBH_MSC_UnitIsReady()` u `usbh_diskio.c`

### MP3 Player Zaglavi Na Rewind

**Simptomi**: `mp3player.c:Mp3Rewind()` infinite loop

**Uzrok**: Bug u Helix dekoderu - pogrešan file seek

**Workaround**:
```c
// U mp3player.c, ograniči broj seek pokušaja
int retries = 0;
while (readPtr != fileStart && retries++ < 100) {
  // ... seek logic
}
```

**Dugoročno**: Zamijeni Helix sa libmad ili minimp3

---

## Česti Problemi i Rješenja

### Problem: "undefined reference to `Reset_Handler`"

**Uzrok**: Startup file nije uključen u build

**Rješenje**:
```ini
# platformio.ini
build_src_filter = 
  +<startup/*.s>
```

### Problem: "VFP register arguments mismatch"

**Uzrok**: Neki source fileovi kompajlirani bez FPU flagova

**Rješenje**:
```ini
# platformio.ini
build_flags = 
  -mfpu=fpv5-sp-d16
  -mfloat-abi=hard
```

### Problem: "multiple definition of syscalls"

**Uzrok**: `syscalls.c` kompajliran dvaput (root + Src/)

**Rješenje**:
```ini
# platformio.ini
build_src_filter = 
  -<syscalls.c>  # Isključi root verziju
```

---

## Riješeni Arhitektonski Problemi i Nadogradnje

### 1. SPI Asinkrono Razdvajanje (SPI Interrupt Decoupling)
* **Problem**: Originalni kod je izvršavao spore FATFS operacije (`f_lseek`, `f_read`, itd.) unutar SPI interrupt callbacka (`HAL_SPI_TxRxCpltCallback`). To je uzrokovalo zastajkivanje zvuka (audio dropouts) jer su interrupti bili zagušeni.
* **Rješenje**:
  - SPI interrupt je sveden na minimum (ispod 1 μs) - on samo dodaje 4 bajta u kružni spremnik (`spi_event_queue` veličine 16) i odmah reaktivira SPI DMA transfer.
  - Funkcija `ProcessPendingSPIEvents()` čita spremnik asinkrono u glavnoj petlji (`main.c`) i izvršava svu originalnu switch-case logiku.
  - Korišteno je lokalno zasjenjivanje `uint8_t *spi_rx` u obradi kako bi se očuvao originalni kod bez modifikacije mapiranja tipki.

### 2. Sigurnost pri Odspajanju USB Medija (USB Disconnect Safety)
* **Problem**: Ako se USB stick odspoji tijekom reprodukcije ili učitavanja Rekordbox baze, FATFS funkcija `f_read` počne vraćati grešku (`FR_DISK_ERR` / `FR_NOT_READY`), što bi izazvalo beskonačnu petlju `while(f_read() != FR_OK);` i potpuno zaledilo mikrokontroler.
* **Rješenje**:
  - Implementirana je pomoćna funkcija `Safe_f_read` koja broji uzastopne greške čitanja.
  - Ako greška potraje duže od 5 pokušaja, funkcija pretpostavlja da je medij odspojen, postavlja pročitane bajtove na 0 i vraća `FR_OK`. To prekida beskonačnu petlju i dopušta sigurnu propagaciju greške, sprječavajući CPU lockup.
  - Funkcija je prozirno mapirana preko makroa `#define f_read` u `rekordbox.c` i `stm32746g_discovery_audio.c`.

### 3. EMA Filtar za Pitch Slider (MIDI)
* **Problem**: Ripple napona i analogne smetnje uzrokovali su jitter (drhtanje) vrijednosti pitch slidera na Blue Pillu, zagušujući USB MIDI kanal.
* **Rješenje**:
  - Implementiran je Exponential Moving Average (EMA) niskopropusni filtar ($\alpha = 0.25$) na očitanju ADC-a prije snaps/deadbands logike:
    `ema_pitch = (0.25f * local_pitch) + (0.75f * ema_pitch);`
  - Rezultat je savršeno zaglađeno klizanje pitcha bez trzanja i jittera u Traktoru/Rekordboxu.

---

## Roadmap - Prioriteti za Dalji Razvoj

### 🔴 Kritično
1. **Fix MP3 Rewind Bug**
   - File: `mp3player.c:Mp3Rewind()`
   - Problem: Helix decoder seek ne radi pravilno
   - Rješenje: Implementirati buffered seeking

2. **USB Hot-Swap Support**
   - Trenutno: USB mora biti spojen pri boot-u
   - Potrebno: Runtime detekcija i mount

### 🟡 Važno
3. **Master Tempo**
   - Timestretch algoritam za pitch-independent tempo
   - Preporuka: Koristi SoundTouch library

4. **Caching Layer**
   - File: `usbh_diskio.c`
   - Keširanje često čitanih sektora za bržu navigaciju

### 🟢 Nice-to-Have
5. **USB Hub Podrška**
   - Omogući više USB uređaja istovremeno

6. **Sound FX**
   - Filter, echo, reverb (može zahtijevati DSP chip)

---

## Tehničke Specifikacije

### Memory Layout
```
Flash:  0x08000000 - 0x080FFFFF (1MB)
RAM:    0x20000000 - 0x2004FFFF (320KB)
SDRAM:  0xC0000000 - 0xC07FFFFF (8MB, za frame buffer)
```

### USB Pinout
```
PA9  - USB_OTG_FS_VBUS (VBUS detection)
PA10 - USB_OTG_FS_ID   (OTG role detection)
PA11 - USB_OTG_FS_DM   (Data Minus)
PA12 - USB_OTG_FS_DP   (Data Plus)
```

### Clock Tree
```
HSE:        25 MHz (external crystal)
PLL:        432 MHz (SYSCLK source)
SYSCLK:     216 MHz (max Cortex-M7)
AHB:        216 MHz
APB1:       54 MHz (TIM, I2C, UART)
APB2:       108 MHz (SPI, SAI, ADC)
USB_CLK:    48 MHz (from PLLSAIP)
```

### Interrupts (Prioriteti)
```
OTG_FS:     Priority 5 (USB)
DMA2:       Priority 0 (Audio - najviši!)
SDMMC1:     (Disabled - više ne koristimo)
LTDC:       Priority 3 (Display)
```

---

## Testing Checklist

Prije commit-a, provjeri:

- [ ] `pio run` kompajlira bez grešaka
- [ ] `pio run` kompajlira bez **warnings** (osim Helix `-Wunused-*`)
- [ ] Binary size < 1MB (Flash limit)
- [ ] USB stick detection (<5s boot time)
- [ ] WAV playback radi
- [ ] MP3 playback radi (test rewind!)
- [ ] Touchscreen responsive
- [ ] Waveform display ažuriran
- [ ] Nema memory leak-ova (run 10+ minuta)

---

## Git Workflow

```bash
# Nova feature
git checkout -b feature/master-tempo
# ... work ...
git add -A
git commit -m "Add master tempo support with SoundTouch

- Integrated SoundTouch library
- Added tempo slider in UI
- Tested with WAV and MP3 files"
git push origin feature/master-tempo
# ... create PR ...

# Bugfix
git checkout -b fix/mp3-rewind
# ... work ...
git commit -m "Fix MP3 rewind infinite loop

- Limited seek retries to 100
- Added error handling
- Closes #42"
git push origin fix/mp3-rewind
```

---

## Reference - Externe Resurse

### Službena Dokumentacija
- [STM32F746 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0385-stm32f7-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)
- [USB Host Library UM1734](https://www.st.com/resource/en/user_manual/um1734-stmicroelectronics.pdf)
- [FatFS Documentation](http://elm-chan.org/fsw/ff/00index_e.html)

### GitHub Repozitoriji
- [HAL Driver](https://github.com/STMicroelectronics/stm32f7xx_hal_driver)
- [USB Host Library](https://github.com/STMicroelectronics/stm32_mw_usb_host)
- [Original CDJ-100S](https://github.com/spectran/CDJ-100S-STM32F7-Display)

### Community
- [STM32 Forum](https://community.st.com/forums)
- [Rekordbox Forum](https://forums.pioneerdj.com/hc/en-us/community/topics/200369707-rekordbox)

---

## Kontakt i維護

**Original Author**: spectran  
**USB Fork Maintainer**: dvucinozd

**Issues**: [GitHub Issues](https://github.com/dvucinozd/CDJ100S-LL/issues)

