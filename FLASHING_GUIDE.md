# 📲 Flashing Guide - SeedSigner ESP32-S3

Complete guide for flashing the production firmware to your M5Stack CoreS3.

---

## 🛒 What You Need

### Required
- M5Stack CoreS3 (ESP32-S3)
- USB-C cable (data capable)
- Computer with Windows/Mac/Linux

### Optional
- SD card (for PSBT files)
- NTAG 424 NFC tags (for encrypted backup)

---

## ⚠️ Pre-Flash Security Check

**Before flashing, verify:**

1. **Download from official source**
   ```bash
   git clone https://github.com/silexperience210/seedsigner-esp32s3.git
   cd seedsigner-esp32s3
   ```

2. **Verify firmware hash** (for releases)
   ```bash
   sha256sum -c checksums.txt
   ```

3. **Inspect hardware**
   - No visible tampering on M5Stack
   - Original packaging seals intact
   - No extra components added

---

## 🔧 Method 1: PlatformIO (Recommended)

### Install PlatformIO

**VS Code Extension:**
1. Install VS Code
2. Extensions → Search "PlatformIO IDE"
3. Install and reload

**CLI Only:**
```bash
pip install platformio
```

### Build and Flash

```bash
# Clone repository
git clone https://github.com/silexperience210/seedsigner-esp32s3.git
cd seedsigner-esp32s3

# Open in VS Code with PlatformIO
code .

# Or use CLI
pio run -e m5stack-cores3 --target upload
```

### Verification

After flashing, check serial output:
```
========================================
  SeedSigner-ESP32S3 v0.2.0-production
  Build: Mar  7 2026 22:00:00
========================================

[SECURITY] Initializing security subsystem...
[SECURITY] JTAG disabled
[SECURITY] WiFi stopped
[SECURITY] WiFi deinitialized
[SECURITY] Bluetooth disabled
[SECURITY] Bluetooth deinitialized
[SECURITY] Memory wiped
[SECURITY] Watchdog enabled (30s timeout)
[SECURITY] Air-gap verified
```

---

## 🔧 Method 2: Pre-built Binary (Quick)

### Download Release

1. Go to [Releases](https://github.com/silexperience210/seedsigner-esp32s3/releases)
2. Download `seedsigner-v0.2.0.bin`
3. Verify SHA256 checksum

### Using esptool.py

```bash
# Install esptool
pip install esptool

# Find your port
# Windows: COM3, COM4, etc.
# Linux/Mac: /dev/ttyUSB0, /dev/ttyACM0, etc.

# Flash firmware
esptool.py --chip esp32s3 --port /dev/ttyUSB0 write_flash 0x0 seedsigner-v0.2.0.bin
```

### Using ESP Flash Download Tools (Windows)

1. Download [ESP Flash Download Tool](https://www.espressif.com/en/support/download/other-tools)
2. Select chip: ESP32-S3
3. Load firmware file at address 0x0
4. Set COM port and baud rate (921600)
5. Click "START"

---

## 🔧 Method 3: Arduino IDE

**Not recommended** - PlatformIO has better dependency management.

---

## ✅ Post-Flash Verification

### 1. Boot Test
- Device shows "SeedSigner" splash screen
- Orange logo on black background
- Version v0.2.0-production displayed

### 2. Air-Gap Verification
- No WiFi networks visible
- No Bluetooth devices discoverable
- Serial shows: "[SECURITY] Air-gap verified"

### 3. Self-Test (Debug builds)
```bash
pio run -e m5stack-cores3-debug --target upload
```

Check serial for:
```
=== Test Results ===
Total:  35
Passed: 35
Failed: 0
ALL TESTS PASSED!
```

### 4. Touch Screen Test
- Tap screen - should respond
- Navigate to Settings
- Test all buttons

---

## 🚨 Troubleshooting

### "Failed to connect to ESP32"

**Solution:**
1. Hold BOOT button on CoreS3
2. Press RESET button
3. Release BOOT button
4. Try flashing again

### "Permission denied" on Linux/Mac

```bash
# Add user to dialout group
sudo usermod -a -G dialout $USER
# Log out and back in
```

### Garbled display

**Cause:** Wrong display driver

**Fix:** Ensure you're using M5Stack CoreS3 environment:
```bash
pio run -e m5stack-cores3 --target upload
```

### Tests fail

**Expected:** Some tests may fail on non-debug builds

**To run tests:**
```bash
pio run -e m5stack-cores3-debug --target upload
```

---

## 🔐 Security Verification

After flashing, verify these security features:

### Check Air-Gap
```bash
# Scan for WiFi - should not find "SeedSigner"
# Scan for Bluetooth - should not find device
```

### Check JTAG Disabled
```bash
# Try to connect with OpenOCD (should fail)
openocd -f interface/ftdi/esp32_devkitj_v1.cfg -f target/esp32s3.cfg
```

### Check Self-Tests Pass
Debug build runs tests on boot - verify "ALL TESTS PASSED!"

---

## 📋 First Use Checklist

- [ ] Firmware flashed successfully
- [ ] Boot shows v0.2.0-production
- [ ] Air-gap verified (no WiFi/BT)
- [ ] Touch screen responsive
- [ ] Self-tests pass (debug build)
- [ ] Create test wallet (small amount)
- [ ] Verify backup/restore works
- [ ] Apply tamper-evident seals

---

## 🆘 Recovery

If device is bricked:

1. Enter download mode:
   - Hold BOOT
   - Press RESET
   - Release BOOT

2. Re-flash firmware

3. If still bricked, use:
   ```bash
   esptool.py --chip esp32s3 --port /dev/ttyUSB0 erase_flash
   esptool.py --chip esp32s3 --port /dev/ttyUSB0 write_flash 0x0 firmware.bin
   ```

---

## 📞 Support

- **Issues:** [GitHub Issues](https://github.com/silexperience210/seedsigner-esp32s3/issues)
- **Discussions:** [GitHub Discussions](https://github.com/silexperience210/seedsigner-esp32s3/discussions)

---

**⚠️ Never flash firmware from untrusted sources. Always verify checksums.**
