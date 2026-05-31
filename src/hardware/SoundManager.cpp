#include "SoundManager.h"
#include <driver/i2s.h>
#include <math.h>

#define SOUND_I2S_PORT I2S_NUM_0
#define SOUND_SAMPLE_RATE 16000
#define SOUND_BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT

SoundManager& SoundManager::getInstance() {
    static SoundManager instance;
    return instance;
}

SoundManager::SoundManager() {
    initialized = false;
    enabled = true;
}

bool SoundManager::begin() {
    i2s_config_t i2sConfig = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SOUND_SAMPLE_RATE,
        .bits_per_sample = SOUND_BITS_PER_SAMPLE,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pinConfig = {
        .bck_io_num = BoardConfig::AUDIO_BCLK,
        .ws_io_num = BoardConfig::AUDIO_WCLK,
        .data_out_num = BoardConfig::AUDIO_DOUT,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    esp_err_t result = i2s_driver_install(
        SOUND_I2S_PORT,
        &i2sConfig,
        0,
        nullptr
    );

    if (result != ESP_OK) {
        Serial.println("[SOUND] Failed to install I2S driver");
        initialized = false;
        return false;
    }

    result = i2s_set_pin(
        SOUND_I2S_PORT,
        &pinConfig
    );

    if (result != ESP_OK) {
        Serial.println("[SOUND] Failed to set I2S pins");
        i2s_driver_uninstall(SOUND_I2S_PORT);
        initialized = false;
        return false;
    }

    i2s_zero_dma_buffer(SOUND_I2S_PORT);

    initialized = true;

    Serial.println("[SOUND] I2S audio initialized");

    confirm();

    return true;
}

void SoundManager::enable(bool value) {
    enabled = value;
}

bool SoundManager::isEnabled() const {
    return enabled;
}

bool SoundManager::isReady() const {
    return initialized;
}

void SoundManager::play(SoundPattern pattern) {
    if (!enabled || !initialized) return;

    switch (pattern) {
        case SoundPattern::TAP:
            tap();
            break;

        case SoundPattern::CONFIRM:
            confirm();
            break;

        case SoundPattern::ERROR:
            error();
            break;

        case SoundPattern::GOAL:
            goal();
            break;

        case SoundPattern::YELLOW_CARD:
            yellowCard();
            break;

        case SoundPattern::RED_CARD:
            redCard();
            break;

        case SoundPattern::SUBSTITUTION:
            substitution();
            break;

        case SoundPattern::HALFTIME:
            halftime();
            break;

        case SoundPattern::FULLTIME:
            fulltime();
            break;

        case SoundPattern::WARNING:
            warning();
            break;
    }
}

void SoundManager::tap() {
    playTone(1400, 35, 1800);
}

void SoundManager::confirm() {
    playTone(1200, 70, 2200);
    silence(40);
    playTone(1600, 70, 2200);
}

void SoundManager::error() {
    playTone(300, 180, 2800);
}

void SoundManager::goal() {
    playTone(900, 90, 2400);
    silence(50);
    playTone(1300, 90, 2400);
    silence(50);
    playTone(1700, 120, 2600);
}

void SoundManager::yellowCard() {
    playTone(750, 120, 2200);
}

void SoundManager::redCard() {
    playTone(350, 180, 2800);
    silence(80);
    playTone(350, 180, 2800);
}

void SoundManager::substitution() {
    playTone(1000, 80, 2000);
    silence(40);
    playTone(800, 80, 2000);
}

void SoundManager::halftime() {
    playTone(700, 150, 2400);
    silence(100);
    playTone(700, 150, 2400);
}

void SoundManager::fulltime() {
    playTone(600, 180, 2600);
    silence(100);
    playTone(800, 180, 2600);
    silence(100);
    playTone(1000, 220, 2800);
}

void SoundManager::warning() {
    playTone(500, 120, 2600);
    silence(70);
    playTone(500, 120, 2600);
}

void SoundManager::playTone(
    int frequencyHz,
    int durationMs,
    int volume
) {
    if (!enabled || !initialized) return;
    if (frequencyHz <= 0 || durationMs <= 0) return;

    const int totalSamples =
        (SOUND_SAMPLE_RATE * durationMs) / 1000;

    const float twoPi = 6.28318530718f;

    int16_t sampleBuffer[128];

    int samplesWritten = 0;

    while (samplesWritten < totalSamples) {
        int batchSize = 128;

        if (samplesWritten + batchSize > totalSamples) {
            batchSize = totalSamples - samplesWritten;
        }

        for (int i = 0; i < batchSize; i++) {
            float t =
                (float)(samplesWritten + i) /
                (float)SOUND_SAMPLE_RATE;

            float wave =
                sinf(twoPi * frequencyHz * t);

            sampleBuffer[i] =
                (int16_t)(wave * volume);
        }

        size_t bytesWritten = 0;

        i2s_write(
            SOUND_I2S_PORT,
            sampleBuffer,
            batchSize * sizeof(int16_t),
            &bytesWritten,
            portMAX_DELAY
        );

        samplesWritten += batchSize;
    }

    i2s_zero_dma_buffer(SOUND_I2S_PORT);
}

void SoundManager::silence(int durationMs) {
    if (!initialized || durationMs <= 0) return;

    const int totalSamples =
        (SOUND_SAMPLE_RATE * durationMs) / 1000;

    int16_t zeroBuffer[128] = {0};

    int samplesWritten = 0;

    while (samplesWritten < totalSamples) {
        int batchSize = 128;

        if (samplesWritten + batchSize > totalSamples) {
            batchSize = totalSamples - samplesWritten;
        }

        size_t bytesWritten = 0;

        i2s_write(
            SOUND_I2S_PORT,
            zeroBuffer,
            batchSize * sizeof(int16_t),
            &bytesWritten,
            portMAX_DELAY
        );

        samplesWritten += batchSize;
    }
}