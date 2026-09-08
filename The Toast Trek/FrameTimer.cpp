#include "FrameTimer.h"

void FrameTimer::Init(int fps) {
    QueryPerformanceFrequency(&timerFreq);
    QueryPerformanceCounter(&timeNow);
    QueryPerformanceCounter(&timePrevious);
    // Init fps time info
    requested_FPS = fps;

    // The number of intervals in the given timer, per frame at the requested rate
    intervalsPerFrame = ((float)timerFreq.QuadPart / requested_FPS);
}

int FrameTimer::FramesToUpdate()
{
    framesToUpdate = 0;
    QueryPerformanceCounter(&timeNow);
    intervalsSinceLastUpdate = (float)(timeNow.QuadPart - timePrevious.QuadPart);
    framesToUpdate = (int)(intervalsSinceLastUpdate / intervalsPerFrame);

    if (framesToUpdate != 0)
    {
        QueryPerformanceCounter(&timePrevious);
    }

    return framesToUpdate;
}

