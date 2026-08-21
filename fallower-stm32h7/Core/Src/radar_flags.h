// radar_flags.h
#ifndef RADAR_FLAGS_H
#define RADAR_FLAGS_H

#include <stdint.h>

typedef struct {
    volatile uint8_t ssFlag;          // SS sinyali durumu
    volatile uint8_t intgClkFlag;     // INTG_CLK sinyali durumu
    volatile uint32_t lastSSTime;     // Son SS zamanı
    volatile uint32_t lastIntgTime;   // Son INTG_CLK zamanı
} RadarFlags_t;

#define SS_LOW_EVENT     (1UL << 0)    // SS düşme olayı
#define SS_HIGH_EVENT    (1UL << 1)    // SS yükselme olayı
#define INTG_CLK_EVENT   (1UL << 2)    // INTG_CLK olayı

extern RadarFlags_t g_radarFlags;

#endif // RADAR_FLAGS_H
