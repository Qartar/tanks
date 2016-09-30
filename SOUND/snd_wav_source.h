/*=========================================================
Name	:	snd_wav_source.h
Date	:	04/07/2006
=========================================================*/

#include "snd_files.h"

#ifndef __SND_WAV_SOURCE__
#define __SND_WAV_SOURCE__

/*=========================================================
=========================================================*/

#define STREAM_THRESHOLD	0x10000		// 65k

#define CHUNK_FMT		MAKEID('f','m','t',' ')
#define CHUNK_CUE		MAKEID('c','u','e',' ')
#define	CHUNK_DATA		MAKEID('d','a','t','a')

typedef class riffChunk_c : public vObject
{
public:
	riffChunk_c (char *szFilename);
	riffChunk_c (riffChunk_c &Outer);

	void		chunkClose ();
	bool		chunkNext ();

	unsigned int	getName ();
	int				getSize ();

	int				getPos ();
	int				setPos (int pos);

	int		readChunk (byte *pOutput);
	int		readData (byte *pOutput, int nLength);
	int		readInt ();

private:
	void	chunkSet ();

	int		m_start;
	int		m_size;
	int		m_name;
	int		m_pos;

	int		m_read (void *out, int len);

	unsigned int	m_chunkName;
	int				m_chunkSize;
	int				m_chunkStart;

	FILE	*m_riff;
} riffChunk_t;

class cSoundWaveSource : public cSoundSource
{
public:
	virtual int				getSamples (byte *pOutput, int nSamples, int nOffset, bool bLooping) = 0;
	virtual soundFormat_t	*getFormat () { return &m_format; }
	virtual char			*getFilename () { return m_szFilename; }
	virtual float			getLoopPosition (float flPosition);

	virtual int		Load (char *szFilename) = 0;
	virtual void	Unload () = 0;

protected:
	void			parseChunk	(riffChunk_t &chunk);

	virtual void	parseFormat (riffChunk_t &chunk);
	virtual void	parseCue	(riffChunk_t &chunk);
	virtual void	parseData	(riffChunk_t &chunk) = 0;

	soundFormat_t	m_format;
	char			m_szFilename[LONG_STRING];

	int			m_numSamples;
	int			m_loopStart;
};

#endif // __SND_WAV_SOURCE__