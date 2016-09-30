/*
===============================================================================

Name	:	g_world.h

Purpose	:	World Object

Date	:	10/21/2004

===============================================================================
*/

#define MAX_OBJECTS	16

#include "r_particle.h"

typedef enum eEffects effects_t;

enum eObjectType
{
	object_object,
	object_bullet,
	object_tank
};

class cObject
{
public:
	cObject ();
	~cObject () {}

	virtual void	Draw ();

	virtual void	Touch (cObject *pOther);
	virtual void	Think ();

	cModel	*pModel;
	vec2	vPos, vVel;			// state and rate of change
	float	flAngle, flAVel;	//
	vec4	vColor;

	eObjectType	eType;
};

class cBullet : public cObject
{
public:
	cBullet ();
	~cBullet () {}

	virtual void	Draw ();
	virtual void	Touch (cObject *pOther);
	virtual void	Think ();

	bool	bInGame;
	int		nPlayer;
};

class cTank : public cObject
{
public:
	cTank ();
	~cTank () {}

	virtual void	Draw ();
	virtual void	Touch (cObject *pOther);
	virtual void	Think ();

	void		UpdateKeys( int nKey, bool Down );

	cModel	*pTurret;
	float	flTAngle, flTVel;
	float	flDamage;
	int		nPlayerNum;
	float	flDeadTime;

	bool	m_Keys[8];

	float	flLastFire;
	cBullet	m_Bullet;
};

#define MAX_PARTICLES	1024

class cWorld
{
	friend cParticle;

public:
	cWorld () : pFreeParticles(NULL), pActiveParticles(NULL) {}
	~cWorld () {}

	void	Init ();
	void	Shutdown ();
	void	Reset ();

	void	RunFrame ();
	void	Draw ();

	void	AddObject (cObject *newObject);
	void	DelObject (cObject *oldObject);

	void	AddSmokeEffect (vec2 vPos, vec2 vVel, int nCount);
	void	AddEffect (vec2 vPos, eEffects eType);

private:
	cObject	*m_Objects[MAX_OBJECTS];

	void	MoveObject (cObject *pObject);

	cParticle	*pFreeParticles;
	cParticle	*pActiveParticles;

	cParticle	m_Particles[MAX_PARTICLES];
	cParticle	*AddParticle ();
	void		FreeParticle (cParticle *pParticle);

	void		m_DrawParticles ();
	void		m_ClearParticles ();
};

extern cWorld *g_World;