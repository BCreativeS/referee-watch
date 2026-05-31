#pragma once

// =====================================================
// BoardConfig.h
// Referee Watch App - Hardware Pin Definitions
// =====================================================
//
// Target: LILYGO T-Watch Ultra (ESP32-S3)
//
// NOTE: These pin numbers must be verified against the
// actual T-Watch Ultra schematic/pinout before flashing.
// The values below are reasonable ESP32-S3 defaults but
// may differ from the production board.
//
// TODO: Confirm all pin assignments from the board
//       datasheet or LILYGO schematic for T-Watch Ultra.
//
// =====================================================

namespace BoardConfig {

    // =================================================
    // SD CARD (SPI bus)
    // =================================================

    constexpr int SD_CS   = 13;   // TODO: verify against T-Watch Ultra schematic
    constexpr int SD_SCK  = 14;   // TODO: verify
    constexpr int SD_MOSI = 15;   // TODO: verify
    constexpr int SD_MISO = 16;   // TODO: verify

    // =================================================
    // I2C BUS
    // Shared by DRV2605 (haptics) and AXP2101 (PMU)
    // =================================================

    constexpr int     I2C_SDA       = 10;      // TODO: verify
    constexpr int     I2C_SCL       = 11;      // TODO: verify
    constexpr uint32_t I2C_FREQUENCY = 400000; // 400 kHz fast mode

    // =================================================
    // I2S AUDIO
    // =================================================

    constexpr int AUDIO_BCLK = 44;   // TODO: verify
    constexpr int AUDIO_WCLK = 45;   // TODO: verify
    constexpr int AUDIO_DOUT = 46;   // TODO: verify

} // namespace BoardConfig
