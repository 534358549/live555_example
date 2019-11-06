
#ifndef __FRAME_MODULE_H__
#define __FRAME_MODULE_H__

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define FRAME_QUEUE_SIZE  64   /* do not to big. */
#define VFRAME_SIZE  0x80000   /* 512K, maybe should large.. */

typedef enum _VTYPE {
    VTYPE_H264 = 0,
    VTYPE_H265 = 1,
    VTYPE_JPEG = 2,
    VTYPE_MJPG = 3,
    VTYPE_MPG4 = 4,

    VTYPE_NONE = -1,
}E_VTYPE;

typedef struct _VFRAME {
    unsigned char vbuff[VFRAME_SIZE];
    unsigned int vlen;
    unsigned int vtype; /* type: 0,h264 1,h265, 2,mjpeg*/
}HI_VFRAME;

typedef struct _FrameQueue {
    HI_VFRAME *vf_q[FRAME_QUEUE_SIZE];
    int vf_used;        /**/
    int vf_max;
    int vf_index;
    int vf_cur;
    int fq_flag; /*fq init(1) or not(0).*/
    pthread_mutex_t lock;  
} FrameQueue;

int frame_queue_init(FrameQueue *fq, int qmax);
int frame_queue_push(FrameQueue *fq, HI_VFRAME *vf);
int frame_queue_pop(FrameQueue *fq, HI_VFRAME *vf);
int frame_queue_used(FrameQueue *fq);
int frame_queue_remain(FrameQueue *fq);
int frame_queue_reset(FrameQueue *fq);
int frame_queue_exit(FrameQueue *fq);

#endif

