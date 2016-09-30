/*=========================================================
Name	:	snd_wav_stream.h
Date	:	04/07/2006
=========================================================*/

#include "snd_wav_source.h"

/*=========================================================
=========================================================*/

class cSoundWaveStream : public cSoundWaveSource
{
public:
	virtual int				getSamples (byte *pOutput, int nSamples, int nOffset, bool bLooping);

	virtual int		Load (char *szFilename);
	virtual void	Unload ();

private:
	virtual void	parseData	(riffChunk_t &chunk);

	int				readData (byte *pOutput, int nStart, int nBytes);

	int		m_dataOffset;	// data chunk
	int		m_dataSize;		// in bytes

	riffChunk_t		*m_reader;
};