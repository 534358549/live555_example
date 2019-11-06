
#include <stdio.h>
#ifdef WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#endif

#include "H264LiveSource.hh"
 
#ifdef WIN32
#define mSleep(ms)    Sleep(ms)
#else
#define mSleep(ms)    usleep(ms*1000)
#endif
 
 
H264LiveSource::H264LiveSource(UsageEnvironment & env, int (*cb_func)(unsigned char*, unsigned int*)) : 
    FramedSource(env), m_pToken(0), cb_ReadFrame(cb_func)
{
	if(cb_func == NULL) 
    {
		printf("[MEDIA SERVER] call back func is NULL, failed\n");
		return;
    }
}
 
H264LiveSource::~H264LiveSource(void)
{	
	envir().taskScheduler().unscheduleDelayedTask(m_pToken); 
	printf("[MEDIA SERVER] rtsp connection closed\n");
}
 
void H264LiveSource::doGetNextFrame()
{
	// 根据 fps，计算等待时间
	double delay = 1000.0 / (FRAME_PER_SEC * 2);  // ms
	int to_delay = delay * 1000;  // us
 
	m_pToken = envir().taskScheduler().scheduleDelayedTask(to_delay, getNextFrame, this);
}
 
unsigned int H264LiveSource::maxFrameSize() const
{
	return 1024*512;
}
 
void H264LiveSource::getNextFrame(void * ptr)
{
	((H264LiveSource *)ptr)->GetFrameData();
}
 
void H264LiveSource::GetFrameData()
{
	gettimeofday(&fPresentationTime, 0);

    // fill frame data
    if(cb_ReadFrame)
        cb_ReadFrame(fTo, &fFrameSize);

	if (fFrameSize > fMaxSize)
	{
		fNumTruncatedBytes = fFrameSize - fMaxSize;
		fFrameSize = fMaxSize;
	}
	else
	{
		fNumTruncatedBytes = 0;
	}
				 
	afterGetting(this);
}


