// radar_flags.cpp
#include "radar_flags.h"

// Global değişken tanımı
RadarFlags_t g_radarFlags = {
    .ssFlag = 0,
    .intgClkFlag = 0,
    .lastSSTime = 0,
    .lastIntgTime = 0
};
