#!/bin/bash
# SeedSigner ESP32-S3 Production Build Script

set -e

echo "=========================================="
echo "SeedSigner ESP32-S3 Production Build"
echo "=========================================="

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check PlatformIO
if ! command -v pio &> /dev/null; then
    echo -e "${RED}Error: PlatformIO not found${NC}"
    echo "Please install: https://platformio.org/install"
    exit 1
fi

# Clean previous builds
echo -e "${YELLOW}Cleaning previous builds...${NC}"
pio run --target clean -e m5stack-cores3 || true

# Run tests first
echo -e "${YELLOW}Running tests...${NC}"
pio test -e native_test || echo "Warning: Tests failed or not available"

# Build production firmware
echo -e "${YELLOW}Building production firmware...${NC}"
pio run -e m5stack-cores3

# Check build output
if [ -f ".pio/build/m5stack-cores3/firmware.bin" ]; then
    echo -e "${GREEN}Build successful!${NC}"
    ls -lh .pio/build/m5stack-cores3/firmware.bin
    
    # Calculate SHA256
    echo -e "${YELLOW}Firmware SHA256:${NC}"
    sha256sum .pio/build/m5stack-cores3/firmware.bin
else
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi

# Create release package
VERSION=$(grep "FIRMWARE_VERSION" src/main.cpp | grep -o '"[^"]*"' | head -1 | tr -d '"')
RELEASE_DIR="releases/${VERSION}"

echo -e "${YELLOW}Creating release package...${NC}"
mkdir -p "${RELEASE_DIR}"
cp .pio/build/m5stack-cores3/firmware.bin "${RELEASE_DIR}/seedsigner-${VERSION}.bin"
cp .pio/build/m5stack-cores3/bootloader.bin "${RELEASE_DIR}/" 2>/dev/null || true
cp .pio/build/m5stack-cores3/partitions.bin "${RELEASE_DIR}/" 2>/dev/null || true

echo -e "${GREEN}Release package created in ${RELEASE_DIR}${NC}"
echo ""
echo "To flash:"
echo "  pio run -e m5stack-cores3 --target upload"
echo ""
echo "Or manually:"
echo "  esptool.py --chip esp32s3 --port /dev/ttyUSB0 write_flash 0x0 ${RELEASE_DIR}/seedsigner-${VERSION}.bin"
