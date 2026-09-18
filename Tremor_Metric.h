#pragma once

#include <stdint.h>
#include <stdbool.h>

// ─── Tremor severity levels (elderly display) ────────────────────
typedef enum {
    TREMOR_CALM = 0,
    TREMOR_MILD,
    TREMOR_MODERATE,
    TREMOR_SEVERE
} TremorLevel_t;

// ─── DSP config ──────────────────────────────────────────────────
#define TREMOR_SAMPLE_RATE_HZ     200.0f   // imuTask runs at 200 Hz
#define TREMOR_WINDOW_SAMPLES     200      // 1 s window @ 200 Hz

// Classic essential-tremor band (4-12 Hz)
#define TREMOR_BAND_LO_HZ         4.0f
#define TREMOR_BAND_HI_HZ         12.0f

// Severity thresholds, dps RMS of band-passed gyro magnitude (tunable)
#define TREMOR_CALM_MAX_DPS       5.0f     //  RMS <  5 dps → CALM
#define TREMOR_MILD_MAX_DPS       12.0f    //  RMS < 12 dps → MILD
#define TREMOR_MODERATE_MAX_DPS   25.0f    //  RMS < 25 dps → MODERATE
                                           //  RMS ≥ 25 dps → SEVERE

void Tremor_Metric_Init(void);

// Feed one gyro sample in dps per axis — call at 200 Hz.
void Tremor_Metric_Feed(float gx_dps, float gy_dps, float gz_dps);

// Non-blocking: true once per completed 1 s window, gives fresh level+RMS.
bool Tremor_Metric_Get(TremorLevel_t *out_level, float *out_rms_dps);

// Display helpers (high contrast for elderly)
const char *Tremor_Level_Text(TremorLevel_t level);    // "CALM"/"MILD"/"MODERATE"/"SEVERE"
uint32_t    Tremor_Level_Color(TremorLevel_t level);   // 0xRRGGBB
