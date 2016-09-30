/*=========================================================
Name	:	snd_device.h
Date	:	04/04/2006
=========================================================*/

#ifndef __SND_DEVICE_H__
#define __SND_DEVICE_H__

#include "../cm_sound.h"

/*=========================================================
=========================================================*/

typedef struct buffer_info_s
{
	int		channels;
	int		bitwidth;
	int		frequency;

	int		read, write;
	int		size;
} buffer_info_t;

typedef enum device_state_s
{
	device_ready,
	device_fail,
	device_abort
} device_state_t;

class cAudioDevice : public vObject
{
public:
	static cAudioDevice	*Create (HWND hWnd);
	static void			Destroy (cAudioDevice *pDevice);

	virtual void		Destroy () = 0;

	virtual device_state_t	getState () = 0;
	virtual buffer_info_t	getBufferInfo () = 0;

	virtual void		writeToBuffer (byte *pAudioData, int nBytes) = 0;
};

#endif //__SND_DEVICE_H__