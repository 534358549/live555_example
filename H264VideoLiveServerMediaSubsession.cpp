
#include "H264VideoLiveServerMediaSubsession.hh"
 
H264VideoLiveServerMediaSubsession::H264VideoLiveServerMediaSubsession(UsageEnvironment & env, FramedSource * source, int (*cb_func)(unsigned char*, unsigned int*)) : OnDemandServerMediaSubsession(env, True)
{
	m_pSource = source;
	m_pSDPLine = 0;
    cb_ReadFrame = cb_func;
}
 
H264VideoLiveServerMediaSubsession::~H264VideoLiveServerMediaSubsession(void)
{
	if (m_pSDPLine)
	{
		free(m_pSDPLine);
	}
}
 
H264VideoLiveServerMediaSubsession * H264VideoLiveServerMediaSubsession::createNew(UsageEnvironment & env, FramedSource * source, int (*cb_func)(unsigned char*, unsigned int*))
{
	return new H264VideoLiveServerMediaSubsession(env, source, cb_func);
}
 
FramedSource * H264VideoLiveServerMediaSubsession::createNewStreamSource(unsigned clientSessionId, unsigned & estBitrate)
{
	return H264VideoStreamFramer::createNew(envir(), new H264LiveSource(envir(), cb_ReadFrame));
}
 
RTPSink * H264VideoLiveServerMediaSubsession::createNewRTPSink(Groupsock * rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource * inputSource)
{
	return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}
 
char const * H264VideoLiveServerMediaSubsession::getAuxSDPLine(RTPSink * rtpSink, FramedSource * inputSource)
{
	if (m_pSDPLine)
	{
		return m_pSDPLine;
	}
 
	m_pDummyRTPSink = rtpSink;
 
	//mp_dummy_rtpsink->startPlaying(*source, afterPlayingDummy, this);
	m_pDummyRTPSink->startPlaying(*inputSource, 0, 0);
 
	chkForAuxSDPLine(this);
 
	m_done = 0;
 
	envir().taskScheduler().doEventLoop(&m_done);
 
	m_pSDPLine = strdup(m_pDummyRTPSink->auxSDPLine());
 
	m_pDummyRTPSink->stopPlaying();
 
	return m_pSDPLine;
}
 
void H264VideoLiveServerMediaSubsession::afterPlayingDummy(void * ptr)
{
	H264VideoLiveServerMediaSubsession * This = (H264VideoLiveServerMediaSubsession *)ptr;
 
	This->m_done = 0xff;
}
 
void H264VideoLiveServerMediaSubsession::chkForAuxSDPLine(void * ptr)
{
	H264VideoLiveServerMediaSubsession * This = (H264VideoLiveServerMediaSubsession *)ptr;
 
	This->chkForAuxSDPLine1();
}
 
void H264VideoLiveServerMediaSubsession::chkForAuxSDPLine1()
{
	if (m_pDummyRTPSink->auxSDPLine())
	{
		m_done = 0xff;
	}
	else
	{
		double delay = 1000.0 / (FRAME_PER_SEC);  // ms
		int to_delay = delay * 1000;  // us
 
		nextTask() = envir().taskScheduler().scheduleDelayedTask(to_delay, chkForAuxSDPLine, this);
	}
}

