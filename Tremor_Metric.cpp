
#include "Tremor_Metric.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    float b0, b1, b2;
    float a1, a2;
    float x1, x2;
    float y1, y2;
} Biquad_t;


static void Biquad_Bp_Init(Biquad_t *b) {
    float f0 = sqrtf(TREMOR_BAND_LO_HZ * TREMOR_BAND_HI_HZ);
    float Q  = f0 / (TREMOR_BAND_HI_HZ - TREMOR_BAND_LO_HZ);
    float w0 = 2.0f * (float)M_PI * f0 / TREMOR_SAMPLE_RATE_HZ;
    float cs = cosf(w0);
    float sn = sinf(w0);
    float al = sn / (2.0f * Q);
    float a0 = 1.0f + al;
    b->b0 =  al / a0;
    b->b1 =  0.0f;
    b->b2 = -al / a0;
    b->a1 = -2.0f * cs / a0;
    b->a2 = (1.0f - al) / a0;
    b->x1 = b->x2 = 0.0f;
    b->y1 = b->y2 = 0.0f;
}


static float Biquad_Step(Biquad_t *b, float x) {
    float y = b->b0 * x + b->b2 * b->x2 - b->a1 * b->y1 - b->a2 * b->y2;
    b->x2 = b->x1; b->x1 = x;
    b->y2 = b->y1; b->y1 = y;
    return y;
}

static Biquad_t s_bpX, s_bpY, s_bpZ;
static double   s_sumSq = 0.0;
static uint32_t s_count = 0;
static bool     s_windowDone = false;
static TremorLevel_t s_lastLevel = TREMOR_CALM;
static float s_lastRms = 0.0f;


void Tremor_Metric_Init(void) {
    Biquad_Bp_Init(&s_bpX);
    Biquad_Bp_Init(&s_bpY);
    Biquad_Bp_Init(&s_bpZ);
    s_sumSq = 0.0; s_count = 0;
    s_windowDone = false;
    s_lastLevel = TREMOR_CALM;
    s_lastRms = 0.0f;
}


void Tremor_Metric_Feed(float gx_dps, float gy_dps, float gz_dps) {
    float bx = Biquad_Step(&s_bpX, gx_dps);
    float by = Biquad_Step(&s_bpY, gy_dps);
    float bz = Biquad_Step(&s_bpZ, gz_dps);
    float m2 = bx*bx + by*by + bz*bz;
    s_sumSq += (double)m2;
    s_count++;
    if (s_count >= TREMOR_WINDOW_SAMPLES) {
        float rms = sqrtf((float)(s_sumSq / (double)TREMOR_WINDOW_SAMPLES));
        s_lastRms = rms;
        s_lastLevel = TREMOR_CALM;
        if (rms >= TREMOR_MODERATE_MAX_DPS) s_lastLevel = TREMOR_SEVERE;
        else if (rms >= TREMOR_MILD_MAX_DPS) s_lastLevel = TREMOR_MODERATE;
        else if (rms >= TREMOR_CALM_MAX_DPS) s_lastLevel = TREMOR_MILD;
        s_sumSq = 0.0; s_count = 0;
        s_windowDone = true;
    }
}


bool Tremor_Metric_Get(TremorLevel_t *out_level, float *out_rms_dps) {
    if (!s_windowDone) return false;
    s_windowDone = false;
    if (out_level)   *out_level   = s_lastLevel;
    if (out_rms_dps) *out_rms_dps = s_lastRms;
    return true;
}


const char *Tremor_Level_Text(TremorLevel_t level) {
    switch (level) {
        case TREMOR_CALM:     return "CALM";
        case TREMOR_MILD:     return "MILD";
        case TREMOR_MODERATE: return "MODERATE";
        case TREMOR_SEVERE:   return "SEVERE";
        default:              return "CALM";
    }
}


uint32_t Tremor_Level_Color(TremorLevel_t level) {
    switch (level) {
        case TREMOR_CALM:     return 0x00E676;
        case TREMOR_MILD:     return 0xFF5252;
        case TREMOR_MODERATE: return 0xFFA726;
        case TREMOR_SEVERE:   return 0xF44336;
        default:              return 0x00E676;
    }
}
