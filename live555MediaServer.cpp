

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "H264VideoLiveServerMediaSubsession.hh"
#include "frame_module.h"
#include "video_parse.h"

UsageEnvironment* env;

// To make the second and subsequent client for each stream reuse the same
// input stream as the first client (rather than playing the file from the
// start for each client), change the following "False" to "True":
Boolean reuseFirstSource = False;

// To stream *only* MPEG-1 or 2 video "I" frames
// (e.g., to reduce network bandwidth),
// change the following "False" to "True":
Boolean iFramesOnly = False;

FrameQueue g_stFrmQ;

static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
			   char const* streamName, char const* inputFileName); // fwd

static char newDemuxWatchVariable;

#define FRAME_READ_LEN 1024

void *read_frame_thread_func(void *args)
{
    int s32Ret = 0;
    int bufPos = 0;
    int s32ReadLen;
    unsigned char pu8Buf[512*1024] = {0};
    unsigned char frameBuf[512*1024] = {0};
    unsigned int frameLen = 0;
    int frameCnt = 0;
    FILE *fpStrm = NULL;
    HI_VFRAME vf;

    fpStrm = fopen("test.264", "rb");
    if(!fpStrm) {
        printf("file open failed.\n");
        return NULL;
    }

    while(1) {
        if(feof(fpStrm)) {
            fseek(fpStrm, SEEK_SET, 0);
            usleep(1);
        }
        
		s32Ret = fread(pu8Buf + bufPos, 1, FRAME_READ_LEN, fpStrm);
		if(s32Ret <= 0) 
		{
			bufPos = 0;
			continue;
		} 
 
		bufPos += FRAME_READ_LEN;

		/*in case ptr overflow.*/
		if(bufPos > 500*1024) {
			printf("Error, can not get one frame, continue.\n");
			bufPos = 0;
			continue;
		}

		s32Ret = H264DecLoadAU(pu8Buf, bufPos, &s32ReadLen);
		if(s32Ret < 0) {
			/*frame not end.*/
			continue;
		} else {                
			/*get one frame now, copy it*/
			memcpy(frameBuf, pu8Buf, s32ReadLen);
			frameLen = s32ReadLen;

			/*deal remaind data.*/
			bufPos -= s32ReadLen;				
			memcpy(pu8Buf, pu8Buf + s32ReadLen, bufPos);

			frameCnt++;
		}

        /******************************************************/
        /* here we shall to deal this frame. */
        //printf("[frm index: %08d] %02x %02x %02x %02x %02x [frm len:%d]\n", frameCnt, 
            //frameBuf[0], frameBuf[1], frameBuf[2], frameBuf[3], frameBuf[4], frameLen);
        if(frame_queue_remain(&g_stFrmQ) <= 0) {
            printf("WARN: frame full, reset it.\n");
            frame_queue_reset(&g_stFrmQ);
        }
        memcpy(vf.vbuff, frameBuf, frameLen);
        vf.vlen = frameLen;
        frame_queue_push(&g_stFrmQ, &vf);

        usleep(40000);
    }

    printf("file read end , exit.\n");
    sleep(3);
    
    return NULL;
}

int cb_readframe(unsigned char *pbuff, unsigned int *len) 
{
    HI_VFRAME vf;
    if(frame_queue_used(&g_stFrmQ)<= 0 ) {
        *len = 0;
    } else {
        frame_queue_pop(&g_stFrmQ, &vf);
        memcpy(pbuff, vf.vbuff, vf.vlen);
        *len = vf.vlen; 
    }

    printf("%s \n", __FUNCTION__);
    
    return 0;
}

int main(int argc, char** argv) {
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  UserAuthenticationDatabase* authDB = NULL;
#ifdef ACCESS_CONTROL
  // To implement client access control to the RTSP server, do the following:
  authDB = new UserAuthenticationDatabase;
  authDB->addUserRecord("username1", "password1"); // replace these with real strings
  // Repeat the above with each <username>, <password> that you wish to allow
  // access to the server.
#endif

  // Create the RTSP server:
  RTSPServer* rtspServer = RTSPServer::createNew(*env, 8554, authDB);
  if (rtspServer == NULL) {
    *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
    exit(1);
  }

  char const* descriptionString
    = "Session streamed by \"testOnDemandRTSPServer\"";

  // Set up each of the possible streams that can be served by the
  // RTSP server.  Each such stream is implemented using a
  // "ServerMediaSession" object, plus one or more
  // "ServerMediaSubsession" objects for each audio/video substream.

  // A H.264 video elementary stream:
  {
    char const* streamName = "h264ESVideoTest";
    char const* inputFileName = "test.264";
   	H264LiveSource * H264LSrc;
   
    frame_queue_init(&g_stFrmQ, 20);
    
    ServerMediaSession* sms
      = ServerMediaSession::createNew(*env, streamName, streamName,
				      descriptionString);
    sms->addSubsession(H264VideoLiveServerMediaSubsession
		       ::createNew(*env, H264LSrc, cb_readframe));
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, inputFileName);
  }

  // Also, attempt to create a HTTP server for RTSP-over-HTTP tunneling.
  // Try first with the default HTTP port (80), and then with the alternative HTTP
  // port numbers (8000 and 8080).

  if (rtspServer->setUpTunnelingOverHTTP(80) || rtspServer->setUpTunnelingOverHTTP(8000) || rtspServer->setUpTunnelingOverHTTP(8080)) {
    *env << "\n(We use port " << rtspServer->httpServerPortNum() << " for optional RTSP-over-HTTP tunneling.)\n";
  } else {
    *env << "\n(RTSP-over-HTTP tunneling is not available.)\n";
  }

  pthread_t pth_rd;
  pthread_create(&pth_rd, NULL, read_frame_thread_func, NULL);

  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}

static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
			   char const* streamName, char const* inputFileName) {
  char* url = rtspServer->rtspURL(sms);
  UsageEnvironment& env = rtspServer->envir();
  env << "\n\"" << streamName << "\" stream, from the file \""
      << inputFileName << "\"\n";
  env << "Play this stream using the URL \"" << url << "\"\n";
  delete[] url;
}
