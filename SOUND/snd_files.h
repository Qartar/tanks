/*=========================================================
Name	:	snd_files.h
Date	:	04/07/2006
=========================================================*/

#include "../cm_sound.h"

#ifndef __SND_FILES__
#define __SND_FILES__

/*=========================================================
=========================================================*/

typedef struct soundFormat_s
{
	int		format;
	int		channels;
	int		bitwidth;
	int		frequency;
} soundFormat_t;

class cSoundSource : public vObject
{
public:
	static cSoundSource	*createSound (char *szFilename);
	static void			destroySound (cSoundSource *pSound);

	virtual int				getSamples (byte *pOutput, int nSamples, int nOffset, bool bLooping) = 0;
	virtual soundFormat_t	*getFormat () = 0;
	virtual char			*getFilename () = 0;
	virtual float			getLoopPosition (float flPosition) = 0;

private:
	virtual int		Load (char *szFilename) = 0;
	virtual void	Unload () = 0;
};

/*=========================================================
=========================================================*/

#endif // __SND_FILES__