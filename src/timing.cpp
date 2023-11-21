#include "timing.h"
#include <stdio.h>
#include <stdlib.h>

timespec START_TIME = {};

void initTiming()
{
    int err = clock_gettime(TIMING_CLOCK, &START_TIME);
    if (err){
        perror("Failed to initialise START_TIME to CLOCK_MONOTONIC_RAW\n");
        exit(EXIT_FAILURE);
    }
}

void checkGetTime(int err)
{
    if (err){
        perror("Failed to get Current Time\n");
        exit(EXIT_FAILURE);
    }
}

s64 getCurrentTimeNs()
{
    timespec currentTime = {};
    checkGetTime(clock_gettime(TIMING_CLOCK, &currentTime));
    return SEC_TO_NS(currentTime.tv_sec) + currentTime.tv_nsec;
}