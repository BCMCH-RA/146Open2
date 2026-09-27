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
// Causal pipeline (tremor DSP ported from the W146 sketch), no windowing and
// no lag: per axis, two cascaded RBJ band-pass stages -> magnitude squared
// across the three axes -> EMA on that mean-square -> sqrt() = severity.
// Every 200 Hz sample yields a fresh severity, so the on-screen band tracks the
// signal with a ~0.5 s time constant instead of freezing between 1 s window
// boundaries. The whole pipeline is ~30 flops/sample, negligible next to the
// I2C gyro read in the same task.
#define TREMOR_SAMPLE_RATE_HZ     200.0f   // imuTask runs at 200 Hz

// Classic essential-tremor band (4-12 Hz). The centre is the geometric mean and
// the edges are the CASCADE's half-power points, so both are measured off these
// two defines at init and retuning them here is enough to retune the filter.
// Stage count is fixed at 2 by Cascade_t in the .cpp.
#define TREMOR_BAND_LO_HZ         4.0f
#define TREMOR_BAND_HI_HZ         12.0f

// Each stage's Q and the gain normalisation are solved numerically at init
// rather than baked in as constants: a cascade's -3dB bandwidth is much
// narrower than a single stage's, and the raw cascade is not unity at f0.
// Skipping either correction silently rescales the reported severity.
#define TREMOR_EMA_ALPHA          0.01f    // tau ~= (1/fs)/alpha ~= 0.5 s

// Severity thresholds, dps of the band-passed gyro magnitude (tunable).
// NOTE: these describe a mean-square, so a steady single-axis tremor of
// amplitude A reads about A*0.70 — reach for these numbers with that in mind.
//
// The source sketch this DSP came from calibrated its bands far higher
// (25 / 60 / 100 dps, and called its top band "STRONG" not "SEVERE"). The
// project's own calibration is kept deliberately: only the responsiveness was
// adopted, not the clinical scaling. Measured against the pre-port filter this
// DSP reads within 3% of it across 3-12 Hz, so these thresholds carry over
// unchanged. Edit them here if the source calibration is what you actually want.
#define TREMOR_CALM_MAX_DPS       5.0f     //  sev <  5 dps → CALM
#define TREMOR_MILD_MAX_DPS       12.0f    //  sev < 12 dps → MILD
#define TREMOR_MODERATE_MAX_DPS   25.0f    //  sev < 25 dps → MODERATE
                                           //  sev ≥ 25 dps → SEVERE

void Tremor_Metric_Init(void);

// Feed one gyro sample in dps per axis — call at 200 Hz.
void Tremor_Metric_Feed(float gx_dps, float gy_dps, float gz_dps);

// Non-blocking: true when at least one new IMU sample has been processed since
// the previous call, giving the current level + severity. Safe to poll at any
// rate — it never blocks and never returns a stale-by-a-window value.
bool Tremor_Metric_Get(TremorLevel_t *out_level, float *out_severity_dps);

// Display helpers (high contrast for elderly)
const char *Tremor_Level_Text(TremorLevel_t level);    // "CALM"/"MILD"/"MODERATE"/"SEVERE"
uint32_t    Tremor_Level_Color(TremorLevel_t level);   // 0xRRGGBB
