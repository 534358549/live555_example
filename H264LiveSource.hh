
#ifndef __H264LiveSource_H__
#define __H264LiveSource_H__
 
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"
#include "FramedSource.hh"
 
#define FRAME_PER_SEC 60
 
class H264LiveSource : public FramedSource
{
public:
	H264LiveSource(UsageEnvironment & env, int (*cb_func)(unsigned char*, unsigned int*));
	~H264LiveSource(void);
 
public:
	virtual void doGetNextFrame();
	virtual unsigned int maxFrameSize() const;
 
	static void getNextFrame(void * ptr);
	void GetFrameData();
 
private:
	void *m_pToken;
	int (*cb_ReadFrame)(unsigned char *pbuff, unsigned int *len);
};
 
#endif
