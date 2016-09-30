/*
===============================================================================

Name	:	models.h

Purpose	:	static list of models used in game

===============================================================================
*/

static sRect tank_body_rects[] = {
	//	posx	posy	sizex	sizey	gamma
	{	0,		8,		20,		2,		0.5f	},	// left tread
	{	0,		-8,		20,		2,		0.5f	},	// right tread
	{	0,		0,		24,		16,		0.8f	},	// main chassis
	{	-8,		0,		4,		12,		0.6f	},	// whatever
	{	-12,	5,		3,		6,		0.5f	},	// left barrel
	{	-12,	-5,		3,		6,		0.5f	},	// right barrel
	{	10,		0,		2,		12,		0.6f	},
	{	7,		0,		2,		12,		0.6f	}
};
static sRectList tank_body_list = {
	sizeof(tank_body_rects) / sizeof(sRect),
	tank_body_rects
};

static cModel	tank_body_model(&tank_body_list);

static sRect tank_turret_rects[] = {
	//	posx	posy	sizex	sizey	gamma
	{	0,		0,		8,		10,		1.0f	},	// turret
	{	0,		0,		10,		8,		1.0f	},	// turret
	{	13,		0,		16,		2,		1.0f	}	// barrel
};

static sRectList tank_turret_list = {
	sizeof(tank_turret_rects) / sizeof(sRect),
		tank_turret_rects
};

static cModel	tank_turret_model(&tank_turret_list);