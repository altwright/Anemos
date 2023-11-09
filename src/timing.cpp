#include "timing.h"
#include <stdio.h>
#include <stdlib.h>

timespec START_TIME = {};

void initTiming(){
    int err = clock_gettime(TIMING_CLOCK, &START_TIME);
    if (err){
        perror("Failed to initialise START_TIME to CLOCK_MONOTONIC_RAW\n");
        exit(EXIT_FAILURE);
    }
}