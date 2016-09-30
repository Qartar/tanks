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

/* ========================================================================= */

// BIG

/* ========================================================================= */


static sRect tank_body_rects_big[] = {
	//	posx	posy	sizex	sizey	gamma
	{	0,		16,		40,		4,		0.5f	},	// left tread
	{	0,		-16,	40,		4,		0.5f	},	// right tread
	{	0,		0,		48,		32,		0.8f	},	// main chassis
	{	-16,	0,		8,		24,		0.6f	},	// whatever
	{	-24,	10,		6,		12,		0.5f	},	// left barrel
	{	-24,	-10,	6,		12,		0.5f	},	// right barrel
	{	22,		0,		2,		24,		0.6f	},
	{	18,		0,		2,		24,		0.6f	},
	{	14,		0,		2,		24,		0.6f	}
};
static sRectList tank_body_list_big = {
	sizeof(tank_body_rects_big) / sizeof(sRect),
	tank_body_rects_big
};

static cModel	tank_body_model_big(&tank_body_list_big);

static sRect tank_turret_rects_big[] = {
	//	posx	posy	sizex	sizey	gamma
	{	0,		0,		16,		20,		1.0f	},	// turret
	{	0,		0,		20,		16,		1.0f	},	// turret
	{	0,		0,		18,		18,		1.0f	},	// turret
	{	26,		0,		32,		4,		1.0f	}	// barrel
};

static sRectList tank_turret_list_big = {
	sizeof(tank_turret_rects_big) / sizeof(sRect),
		tank_turret_rects_big
};

static cModel	tank_turret_model_big(&tank_turret_list_big);