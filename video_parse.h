#pragma once

#include <stdio.h>

int H264DecLoadAU(unsigned char* pStream, unsigned int iStreamLen, int* pFrameLen);
int H265DecLoadAU(unsigned char* pStream, unsigned int iStreamLen, int* pFrameLen);


