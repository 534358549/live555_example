
#include <stdio.h>
#include "video_parse.h"

#if 0
int H264DecLoadAU(unsigned char* pStream, unsigned int iStreamLen, unsigned int* pFrameLen)
{
	unsigned int i;
	unsigned int state = 0xffffffff;
	unsigned int bFrameStartFound = 0;
	unsigned int bSliceStartFound = 0;

	*pFrameLen = 0;
	if (NULL == pStream || iStreamLen <= 4)
	{
		return -1;
	}

	for (i = 0; i < iStreamLen; i++)
	{
		if ((state == 0x1) && ((pStream[i] & 0x1f) >= 0x1 || (pStream[i] & 0x1f) <= 0x8))
		{
			if (1 == bFrameStartFound || bSliceStartFound == 1)
			{
				if ((pStream[i + 1] >> 7) == 1 || (pStream[i + 1] >> 6) & 0x1 == 1)
				{
					*pFrameLen = i - 4;
					return 0;
				}
			}
			else
			{
				bSliceStartFound = 1;
				//bFrameStartFound = HI_TRUE; 
			}
		}

		/*find a sps, pps, sei*/
		if ((state == 0x1) && ((pStream[i] & 0x1f) == 0x7 || 
            (pStream[i] & 0x1f) == 0x8 || (pStream[i] & 0x1f) == 0x6))
		{
			if (1 == bSliceStartFound)
			{
				bSliceStartFound = 1;
			}
			else if (1 == bFrameStartFound)
			{
				*pFrameLen = i - 4;
				return 0;
			}
			else
			{
				bFrameStartFound = 1;
			}
		}
		state = (state << 8) | pStream[i];
	}

	*pFrameLen = i;
	return -1;
}
#endif
unsigned int CheckFirstMbAddr(unsigned char *p)
{
	unsigned int code;
	unsigned int ZeroCount = 0, i;

	code = (*p++) << 24;
	code |= (*p++) << 16;
	code |= (*p++) << 8;
	code |= *p++;

	for (i = 0; i < 32; i++)
	{
		if ((code & 0x80000000))
		{
			break;
		}
		else
		{
			ZeroCount++;
			code = code << 1;
		}
	}

	code = code << ZeroCount;

	return ((code >> (31 - ZeroCount)) - 1);
}

int H264DecLoadAU(unsigned char* pStream, unsigned int iStreamLen, int* pFrameLen)
{
	unsigned int i;
	unsigned int FirstMbAddr;
	unsigned char* p;
	unsigned int state = 0xffffffff;
	unsigned int FrameStartFound = 0;
	unsigned int PrevFirstMBAddr = 0;

	if (NULL == pStream || iStreamLen <= 4)
	{
		return -1;
	}

	for (i = 0; i < iStreamLen; i++)
	{
		/*find a I-slice or a P-slice*/
		if (((state & 0xFFFFFFFF) == 0x1) && ((pStream[i]&0x1F) == 0x01 || (pStream[i] & 0x1F) == 0x05))
		{
			p = &pStream[i];
			FirstMbAddr = CheckFirstMbAddr(p);
			if (1 == FrameStartFound)
			{
				if (FirstMbAddr <= PrevFirstMBAddr)
				{
					*pFrameLen = i - 4;
					PrevFirstMBAddr = FirstMbAddr;
					state = 0xffffffff;
					return 0;
				}
				else
				{
					PrevFirstMBAddr = FirstMbAddr;
				}
			}
			else
			{
				PrevFirstMBAddr = FirstMbAddr;
				FrameStartFound = 1;
			}
		}

		/*find a sps, pps or au_delimiter*/
		if (((state & 0xFFFFFFFF) == 0x1) &&
			((pStream[i] & 0x1F) == 0x07 || (pStream[i] & 0x1F) == 0x108 || (pStream[i] & 0x1F) == 0x109))
		{
			if (1 == FrameStartFound)
			{
				*pFrameLen = i - 4;
				PrevFirstMBAddr = 0;
				state = 0xffffffff;
				return 0;
			}
			else
			{
				FrameStartFound = 1;
				PrevFirstMBAddr = 0;
			}
		}
		state = (state << 8) | pStream[i];
		if (0x80000 - 1 <= i)  /* max AU size may not large than 0x80000 */
		{
			*pFrameLen = i - 3;
			return 0;
		}
	}
	*pFrameLen = i;

	return -1;
}

int H265DecLoadAU(unsigned char* pStream, unsigned int iStreamLen, int* pFrameLen)
{
	unsigned int i;
	unsigned int state = 0xffffffff;
	unsigned int bFrameStartFound = 0;
	unsigned int bSliceStartFound = 0;

	*pFrameLen = 0;
	if (NULL == pStream || iStreamLen <= 4)
	{
		return -1;
	}

	for (i = 0; i < iStreamLen; i++)
	{
		if ((state & 0xFFFFFF7E) >= 0x100 &&
			(state & 0xFFFFFF7E) <= 0x13E)
		{
			if (1 == bFrameStartFound || bSliceStartFound == 1)
			{
				if ((pStream[i + 1] >> 7) == 1)
				{
					*pFrameLen = i - 4;
					return 0;
				}
			}
			else
			{
				bSliceStartFound = 1;
				//bFrameStartFound = HI_TRUE; 
			}
		}

		/*find a vps, sps, pps*/
		if ((state & 0xFFFFFF7E) == 0x140 ||
			(state & 0xFFFFFF7E) == 0x142 ||
			(state & 0xFFFFFF7E) == 0x144)
		{
			if (1 == bSliceStartFound)
			{
				bSliceStartFound = 1;
			}
			else if (1 == bFrameStartFound)
			{
				*pFrameLen = i - 4;
				return 0;
			}
			else
			{
				bFrameStartFound = 1;
			}
		}
		state = (state << 8) | pStream[i];
	}

	*pFrameLen = i;
	return -1;
}
