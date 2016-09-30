//	r_particle.h

#ifndef __R_PARTICLE_H__
#define __R_PARTICLE_H__

class cParticle
{
public:
	cParticle () {}
	~cParticle () {}

	void	AddToActive ();
	void	AddToFree ();

	cParticle	*pNext;

	float	flTime;
	float	flSize, flSizeVel;
	vec2	vPos, vVel, vAccel;
	float	flDrag;
	vec4	vColor, vColorVel;

	int		bitFlags;
};

#define PF_INVERT	0x00000001

#endif //__R_PARTICLE_H__