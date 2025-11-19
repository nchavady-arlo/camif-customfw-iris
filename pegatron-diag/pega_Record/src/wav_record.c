//For Reference
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define SAMPLE_RATE 44100  // 音訊取樣頻率
#define DURATION 5         // 音訊長度 (秒)
#define AMPLITUDE 32000    // 最大振幅 (16-bit PCM)

typedef struct {
    char riff[4];          // "RIFF"
    uint32_t chunk_size;   // 整個文件大小 - 8
    char wave[4];          // "WAVE"
    char fmt[4];           // "fmt "
    uint32_t fmt_size;     // 16
    uint16_t audio_format; // 音訊格式 (1 表示 PCM)
    uint16_t num_channels; // 通道數 (1: 單聲道, 2: 立體聲)
    uint32_t sample_rate;  // 取樣頻率
    uint32_t byte_rate;    // 每秒資料量 (取樣率 * 通道數 * 位深度 / 8)
    uint16_t block_align;  // 每樣本數據大小 (通道數 * 位深度 / 8)
    uint16_t bits_per_sample; // 每樣本的位深度 (16-bit)
    char data[4];          // "data"
    uint32_t data_size;    // 音訊資料大小 (樣本數 * 位深度 / 8)
} wav_header;

void generate_sine_wave(int16_t* buffer, int num_samples, int sample_rate, int frequency) {
    for (int i = 0; i < num_samples; ++i) {
        buffer[i] = (int16_t)(AMPLITUDE * sin(2 * M_PI * frequency * i / sample_rate));
    }
}

int main() {
    FILE *file;
    wav_header header;
    int num_samples = SAMPLE_RATE * DURATION;   // 計算音訊樣本數
    int16_t *buffer = (int16_t *)malloc(num_samples * sizeof(int16_t));

    if (!buffer) {
        fprintf(stderr, "記憶體分配失敗\n");
        return 1;
    }

    // 生成 440Hz 的正弦波 (A4 音符)
    generate_sine_wave(buffer, num_samples, SAMPLE_RATE, 440);

    // 設置 WAV 文件頭
    memcpy(header.riff, "RIFF", 4);
    header.chunk_size = 36 + num_samples * sizeof(int16_t); // 36是固定的header大小
    memcpy(header.wave, "WAVE", 4);
    memcpy(header.fmt, "fmt ", 4);
    header.fmt_size = 16;
    header.audio_format = 1;      // PCM
    header.num_channels = 1;      // 單聲道
    header.sample_rate = SAMPLE_RATE;
    header.byte_rate = SAMPLE_RATE * header.num_channels * sizeof(int16_t);
    header.block_align = header.num_channels * sizeof(int16_t);
    header.bits_per_sample = 16;
    memcpy(header.data, "data", 4);
    header.data_size = num_samples * sizeof(int16_t);

    // 開啟 WAV 檔案
    file = fopen("output.wav", "wb");
    if (!file) {
        fprintf(stderr, "無法創建文件\n");
        free(buffer);
        return 1;
    }

    // 寫入 WAV 文件頭
    fwrite(&header, sizeof(wav_header), 1, file);

    // 寫入音訊資料
    fwrite(buffer, sizeof(int16_t), num_samples, file);

    // 關閉文件
    fclose(file);

    free(buffer);

    printf("WAV 文件已生成: output.wav\n");
    return 0;
}
