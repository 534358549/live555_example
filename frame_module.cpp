
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  

#include "frame_module.h"

int frame_queue_init(FrameQueue *fq, int qmax)
{
    int i = 0;

    if(!fq) {
        printf("[%s@%d]Input frame queue is invalid.\n", __FUNCTION__, __LINE__);
        return -1;
    }

    if(fq->fq_flag == 1) {
        printf("[%s@%d]Input frame queue is init already.\n", __FUNCTION__, __LINE__);
        return -1;
    }
    
    if(qmax > FRAME_QUEUE_SIZE) {
        printf("set queue error, max is: %d(you set %d)\n", \
            FRAME_QUEUE_SIZE, qmax);  
        return -1;
    }
    
    fq->vf_max = qmax;
    fq->vf_index = 0; /* waiting for pop index number, read frome here. */
    fq->vf_used = 0;  /* queue used size, shell not more than vf_max. */
    fq->vf_cur = 0;   /* current index for push one frame. */

    for(i = 0; i < fq->vf_max; i++) {
        fq->vf_q[i] = (HI_VFRAME *)malloc(sizeof(HI_VFRAME));
        if(!fq->vf_q) {
            printf("frame queue malloc error at : %d\n", i);
            while(--i)
                free(fq->vf_q[i]);
            return -1;
        }
    }

    /*init a lock*/
    pthread_mutex_init(&fq->lock, NULL);  

    /* set init flag */
    fq->fq_flag = 1;
    
    printf("FrameQueue init success.\n");
    return 0;
}

int frame_queue_push(FrameQueue *fq, HI_VFRAME *vf)
{
    if(!fq) {
        printf("[%s@%d]Input frame queue is invalid.\n", __FUNCTION__, __LINE__);
        return -1;
    }

    if(fq->vf_used >= fq->vf_max) {
        printf("ERR: frame queue full, pop one frame first.\n");
        return -1;
    }

    pthread_mutex_lock(&fq->lock);  

    // push one frame.
    memcpy(fq->vf_q[fq->vf_cur]->vbuff, vf->vbuff, vf->vlen);
    fq->vf_q[fq->vf_cur]->vlen = vf->vlen;

    fq->vf_used++;
    fq->vf_cur = (fq->vf_cur + 1)%(fq->vf_max);

    pthread_mutex_unlock(&fq->lock);  

    return 0;
}

int frame_queue_pop(FrameQueue *fq, HI_VFRAME *vf)
{
    if(!fq) {
        printf("[%s@%d]Input frame queue is invalid.\n", __FUNCTION__, __LINE__);
        return -1;
    }

    if(fq->vf_used <= 0) {
        printf("ERR: frame queue empty, push one frame first.\n");
        return -1;
    } 

    pthread_mutex_lock(&fq->lock);  

    // pop one frame.    
    memcpy(vf->vbuff, fq->vf_q[fq->vf_index]->vbuff, fq->vf_q[fq->vf_index]->vlen);
    vf->vlen = fq->vf_q[fq->vf_index]->vlen;

    fq->vf_used--;
    fq->vf_index = (fq->vf_index + 1)%(fq->vf_max);

    pthread_mutex_unlock(&fq->lock);  

    return 0;
}

int frame_queue_used(FrameQueue *fq)
{
    if(!fq) {
        printf("[%s@%d]Input frame queue is invalid.\n", __FUNCTION__, __LINE__);
        return -1;
    }
    
    return fq->vf_used;
}

int frame_queue_remain(FrameQueue *fq)
{
    if(!fq) {
        printf("[%s@%d]Input frame queue is invalid.\n", __FUNCTION__, __LINE__);
        return -1;
    }
    
    return (fq->vf_max - fq->vf_used);
}

int frame_queue_reset(FrameQueue *fq)
{    
    int i = 0;
    
    if(!fq) {
        printf("[%s@%d]Input frame queue is invalid.\n", __FUNCTION__, __LINE__);
        return -1;
    }

    pthread_mutex_lock(&fq->lock);  

    for(i = 0; i < fq->vf_max; i++) {
        memset(fq->vf_q[i]->vbuff, 0, VFRAME_SIZE);
        fq->vf_q[i]->vlen = 0;
        fq->vf_q[i]->vtype = 0;
    }
    
    fq->vf_cur = 0;
    fq->vf_index = 0;
    fq->vf_used = 0;
    
    pthread_mutex_unlock(&fq->lock);  

    return 0;
}

int frame_queue_exit(FrameQueue *fq)
{
    int i = 0;
    
    if(!fq) {
        printf("[%s@%d]Input frame queue is invalid.\n", __FUNCTION__, __LINE__);
        return -1;
    }

    for(i = 0; i < fq->vf_max; i++) {
        if(fq->vf_q[i])
            free(fq->vf_q[i]);
    }

    fq->vf_cur = 0;
    fq->vf_index = 0;
    fq->vf_max = 0;
    fq->vf_used = 0;

    pthread_mutex_destroy(&fq->lock);

    /* set exit flag */
    fq->fq_flag = 0;

    return 0;
}
