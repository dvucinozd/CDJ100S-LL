# CDJ-100S STM32F7 Display - USB Host Verzija

## Pregled Projekta

Modernizirani audio player za Pioneer CDJ-100S DJ konzolu, baziran na **STM32F746 Discovery** ploči. Reproducira WAV i MP3 datoteke s **Rekordbox** playlistama putem **USB Mass Storage** uređaja.

### Ključne Značajke

- ✅ **USB Host Podrška** - čita audio datoteke direktno s USB stick-a
- 🎵 **Multi-format playback** - WAV (bez problema) i MP3 (Helix dekoder)
- 🎛️ **Hardverska kontrola** - Touchscreen + CDJ-100S jog/slider/tipke via SPI
- 📊 **Hi-res waveform** - 9 nivoa zoom prikaza
- 🎯 **Hot cue bodovi** - prebacivanje s SCAN+WAH tipkama
- 📱 **Rekordbox integracija** - automatsko učitavanje playlisti

---

## Hardware Platforma

### Komponente
- **MCU**: STM32F746NGHx (ARM Cortex-M7, 216MHz)
- **Ploča**: STM32F746G-Discovery
- **Display**: 480x272 RGB888 LCD (LTDC)
- **Audio**: WM8994 codec (SAI2 + I2C3)
- **Storage**: USB OTG Full Speed (12 Mbit/s)

### USB Pinout (PA9-PA12)
```
PA9  - USB_OTG_FS_VBUS (detekcija)
PA10 - USB_OTG_FS_ID   (OTG switching)
PA11 - USB_OTG_FS_DM   (Data Minus)
PA12 - USB_OTG_FS_DP   (Data Plus)
```

---

## Build Instrukcije

### Zahtjevi
- **PlatformIO** (VS Code ekstenzija ili CLI)
- **ARM GCC Toolchain** (automatski instaliran s PlatformIO)
- **Git** (za kloniranje)

### Kompajliranje

```bash
# Kloniraj repozitorij
git clone https://github.com/dvucinozd/CDJ100S-LL.git
cd CDJ100S-LL

# Build s PlatformIO
pio run

# Upload na ploču (preko ST-Link)
pio run -t upload
```

### Alternative: STM32CubeIDE
1. Otvori `.cproject` u STM32CubeIDE
2. Refresh projekt (F5)
3. Build (Ctrl+B)
4. Flash (F11)

---

## Struktura Projekta

```
CDJ100S-LL/
├── Drivers/
│   └── STM32F7xx_HAL_Driver/      # HAL driveri (uključujući HCD/LL USB)
├── Middlewares/
│   ├── ST/
│   │   └── STM32_USB_Host_Library/ # Službena USB Host biblioteka
│   └── Third_Party/
│       ├── FatFs/                  # File system
│       └── Helix/                  # MP3 dekoder
├── Src/
│   ├── main.c                      # Glavna aplikacija
│   ├── usb_host.c                  # USB Host inicijalizacija
│   ├── usbh_conf.c                 # USB HAL konfiguracija
│   ├── usbh_diskio.c               # FatFS disk I/O za USB MSC
│   ├── mp3player.c                 # MP3 reprodukcija
│   └── ...
├── Inc/                            # Header datoteke
├── platformio.ini                  # PlatformIO konfiguracija
└── STM32F746NGHx_FLASH.ld         # Linker script
```

---

## Kako Koristiti

### Priprema USB Stick-a

1. **Formatiranje**: FAT32 ili exFAT
2. **Rekordbox Export**: 
   - Eksportiraj playlist na USB u Rekordboxu
   - Struktura: `/PIONEER/USBANLZ/<folders>/*.mp3`
3. **Spajanje**: Ubaci USB u CN13 konektor na Discovery ploči

### Kontrole

#### Touchscreen
- **Waveform zoom**: Dodirni desno (expand) / lijevo (compress) / centar (reset)
- **Track selekcija**: Klikni na listu

#### CDJ-100S Tipke (preko SPI)
- **Jog wheel**: Scratch i pitch bend
- **Slider**: Tempo kontrola  
- **SCAN+WAH**: Prebacivanje hot cue bodova
- **PLAY/CUE**: Standardne DJ funkcije

---

## Changelog

### 🆕 **Verzija 2.1** (2026-05-26) - Performanse i Stabilnost
- ✅ **SPI Asinkrono Razdvajanje (SPI Decoupling)** - uklonjeno zagušenje zvuka micanjem spora trija FATFS poziva iz interrupta u kružni spremnik
- ✅ **USB Disconnect Freeze Protection** - `Safe_f_read` detektira iskapčanje sticka i sprječava beskonačna petljanja i CPU lockup
- ✅ **MIDI Pitch Fader EMA Smoothing** - integriran EMA filtar ($\alpha = 0.25$) u MIDI podmodul za super-glatko upravljanje bez šuma

### 🆕 **Verzija 2.0** (2026-02-08) - USB Host Integracija
- ✅ Zamijenjena SD kartica s USB Mass Storage podrškom
- ✅ Dodati HAL HCD i LL USB driveri
- ✅ Integrirana službena STM32 USB Host biblioteka
- ✅ Kreiran USB diskio driver za FatFS
- ✅ PlatformIO build podrška
- ✅ Uspješna kompilacija i linkanje

### Verzija 1.1 (2020-05-28)
- Ažuriran hi-res waveform display (9 nivoa zoom-a)
- Promijenjeno ponašanje hot cue-ova (SCAN+WAH switching)
- Ispravljeni minor bugovi

### Verzija 1.0 (Originalno)
- Inicijalno izdanje s SD karticom
- WAV i MP3 playback
- Rekordbox integracija

---

## Poznati Problemi i Ograničenja

### ⚠️ MP3 Decoder
- **Rewind može zapeti** - problem u Helix implementaciji
- **Preporuka**: Koristi WAV datoteke za kritične trackove

### 🐌 USB vs SD Performanse
- USB Full Speed: **12 Mbit/s** (teorijski)
- SD 4-bit: **do 50 MB/s**
- Za audio reprodukciju (**~1.4 Mbps za WAV**) USB je dovoljan

---

## Roadmap - Buduća Poboljšanja

Autor bi cijenio doprinose zajednice:

- [ ] **Popravka Helix dekodera** - riješiti rewind probleme
- [ ] **Master tempo** - timestretch algoritam
- [ ] **Sound FX** - filter, reverb (može zahtijevati DSP chip)
- [ ] **Nove display modove** - spektar, vugraf, itd.
- [ ] **USB Hub support** - više USB uređaja istovremeno

---

## Licenca i Autori

**Original**: [spectran/CDJ-100S-STM32F7-Display](https://github.com/spectran/CDJ-100S-STM32F7-Display)

**USB Fork**: [dvucinozd/CDJ100S-LL](https://github.com/dvucinozd/CDJ100S-LL)

---

## Kontakt i Podrška

Za pitanja, bugove ili feature requestove:
- **GitHub Issues**: [dvucinozd/CDJ100S-LL/issues](https://github.com/dvucinozd/CDJ100S-LL/issues)

---

## Reference

- [STM32F746 Discovery Dokumentacija](https://www.st.com/en/evaluation-tools/32f746gdiscovery.html)
- [Pioneer Rekordbox](https://rekordbox.com/)
- [CDJ-100S MIDI Controller Repo](https://github.com/spectran/CDJ-100S-MIDI-Controller)
- [Helix MP3 Decoder](https://github.com/ultraembedded/libhelix-mp3)
