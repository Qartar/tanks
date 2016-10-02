/*
===============================================================================

Name    :   models.h

Purpose :   static list of models used in game

===============================================================================
*/

static sRect tank_body_rects[] = {
    //  posx    posy    sizex   sizey   gamma
    {   0,      8,      20,     2,      0.5f    },  // left tread
    {   0,      -8,     20,     2,      0.5f    },  // right tread
    {   0,      0,      24,     16,     0.8f    },  // main chassis
    {   -8,     0,      4,      12,     0.6f    },  // whatever
    {   -12,    5,      3,      6,      0.5f    },  // left barrel
    {   -12,    -5,     3,      6,      0.5f    },  // right barrel
    {   10,     0,      2,      12,     0.6f    },
    {   7,      0,      2,      12,     0.6f    }
};
static sRectList tank_body_list = {
    sizeof(tank_body_rects) / sizeof(sRect),
    tank_body_rects
};

static cModel   tank_body_model(&tank_body_list);

static sRect tank_turret_rects[] = {
    //  posx    posy    sizex   sizey   gamma
    {   0,      0,      8,      10,     1.0f    },  // turret
    {   0,      0,      10,     8,      1.0f    },  // turret
    {   13,     0,      16,     2,      1.0f    }   // barrel
};

static sRectList tank_turret_list = {
    sizeof(tank_turret_rects) / sizeof(sRect),
        tank_turret_rects
};

static cModel   tank_turret_model(&tank_turret_list);

/* ========================================================================= */

// BIG

/* ========================================================================= */


static sRect tank_body_rects_big[] = {
    //  posx    posy    sizex   sizey   gamma
    {   0,      16,     40,     4,      0.5f    },  // left tread
    {   0,      -16,    40,     4,      0.5f    },  // right tread
    {   0,      0,      48,     32,     0.8f    },  // main chassis
    {   -16,    0,      8,      24,     0.6f    },  // whatever
    {   -24,    10,     6,      12,     0.5f    },  // left barrel
    {   -24,    -10,    6,      12,     0.5f    },  // right barrel
    {   22,     0,      2,      24,     0.6f    },
    {   18,     0,      2,      24,     0.6f    },
    {   14,     0,      2,      24,     0.6f    }
};
static sRectList tank_body_list_big = {
    sizeof(tank_body_rects_big) / sizeof(sRect),
    tank_body_rects_big
};

static cModel   tank_body_model_big(&tank_body_list_big);

static sRect tank_turret_rects_big[] = {
    //  posx    posy    sizex   sizey   gamma
    {   0,      0,      16,     20,     1.0f    },  // turret
    {   0,      0,      20,     16,     1.0f    },  // turret
    {   0,      0,      18,     18,     1.0f    },  // turret
    {   26,     0,      32,     4,      1.0f    }   // barrel
};

static sRectList tank_turret_list_big = {
    sizeof(tank_turret_rects_big) / sizeof(sRect),
        tank_turret_rects_big
};

static cModel   tank_turret_model_big(&tank_turret_list_big);

/*  ======================================================= */

//  M1 ABRAMS

/*  ======================================================= */

static sRect abrams_body_rects[] = {
    //  posx    posy    sizex   sizey   gamma
    {   -19,    0,      2,      10,     0.4f    },  // middle rear doodad
    {   -19,    7,      2,      2,      0.4f    },  // top rear doodad
    {   -19,    -7,     2,      2,      0.4f    },  // bottom rear doodad
    {   -2,     0,      34,     19,     0.7f    },  // main chassis
    {   14,     0,      2,      8,      0.6f    },  // front divit
    {   12,     0,      2,      8,      0.633f  },  // front divit
    {   10,     0,      2,      8,      0.666f  },  // front divit
    {   -16,    0,      6,      19,     0.8f    },  // back rise
    {   -12,    0,      2,      19,     0.785f  },  // back rise
    {   -10,    0,      2,      19,     0.766f  },  // back rise
    {   -8,     0,      2,      19,     0.725f  }   // back rise
};

static sRectList abrams_body_list = {
    sizeof(abrams_body_rects) / sizeof(sRect),
    abrams_body_rects
};

static cModel   abrams_body_model(&abrams_body_list);

static sRect abrams_turret_rects[] = {
    //  posx    posy    sizex   sizey   gamma
    {   -16,    0,      2,      8,      0.8f    },  // rear radar array
    {   -5,     0,      22,     12,     1.0f    },  // horizontal plane
    {   -3,     0,      14,     14,     1.0f    },  // central plane
    {   0,      0,      12,     8,      1.0f    },  // forward wide plane
    {   0,      0,      14,     6,      1.0f    },  // forward narrow plane
    {   13,     0,      14,     2,      1.0f    },  // turret barrel
    {   -12,    3,      6,      4,      0.95f   },  // back bottom doodad
    {   -12,    -3,     6,      4,      0.95f   },  // back top doodad
    {   0,      2,      3,      5,      0.9f    },  // cockpit 1 - vert
    {   0,      2,      5,      3,      0.9f,   }   // cockpit 1 - horiz
};

static sRectList abrams_turret_list = {
    sizeof(abrams_turret_rects) / sizeof(sRect),
        abrams_turret_rects
};

static cModel   abrams_turret_model(&abrams_turret_list);

/*
===============================================================================

Name    :   T-80

===============================================================================
*/

static sRect t80_body_rects[] = {
    //  posx    posy    sizex   sizey   gamma
    {   0,      8,      24,     2,      0.5f    },  // left tread
    {   0,      -8,     24,     2,      0.5f    },  // right tread
    {   0,      0,      26,     16,     0.8f    },  // main chassis
    {   -9,     0,      4,      12,     0.65f   },  // whatever
    {   -13,    4,      3,      6,      0.6f    },  // left barrel
    {   -13,    -4,     3,      6,      0.6f    },  // right barrel
    {   12,     0,      2,      8,      0.6f    },
    {   10,     0,      2,      8,      0.666f  },
    {   8,      0,      2,      10,     0.725f  },
    {   6,      0,      2,      10,     0.775f  }
};
static sRectList t80_body_list = {
    sizeof(t80_body_rects) / sizeof(sRect),
    t80_body_rects
};

static cModel   t80_body_model(&t80_body_list);

static sRect t80_turret_rects[] = {
    //  posx    posy    sizex   sizey   gamma
    {   0,      0,      6,      12,     1.0f    },  // dark turret
    {   0,      0,      10,     10,     1.0f    },  // dark turret
    {   0,      0,      12,     6,      1.0f    },  // dark turret
    {   0,      0,      4,      10,     1.0f    },  // light turret
    {   0,      0,      8,      8,      1.0f    },  // light turret
    {   0,      0,      10,     4,      1.0f    },  // light turret
    {   -7,     0,      2,      12,     1.0f    },  // radar array
    {   13,     0,      16,     2,      1.0f    }   // barrel
};

static sRectList t80_turret_list = {
    sizeof(t80_turret_rects) / sizeof(sRect),
        t80_turret_rects
};

static cModel   t80_turret_model(&t80_turret_list);

/*
===============================================================================

Name    :   BLACK EAGLE

===============================================================================
*/

static sRect blkegl_body_rects[] = {
    //  posx    posy    sizex   sizey   gamma
    {   2,      8,      24,     2,      0.4f    },  // left tread
    {   2,      -8,     24,     2,      0.4f    },  // right tread
    {   2,      0,      26,     16,     0.7f    },  // main chassis
    {   14,     0,      2,      8,      0.5f    },
    {   12,     0,      2,      8,      0.566f  },
    {   10,     0,      2,      10,     0.625f  },
    {   8,      0,      2,      10,     0.675f  }
};
static sRectList blkegl_body_list = {
    sizeof(blkegl_body_rects) / sizeof(sRect),
    blkegl_body_rects
};

static cModel   blkegl_body_model(&blkegl_body_list);

static sRect blkegl_turret_rects[] = {
    //  posx    posy    sizex   sizey   gamma
    {   -3,     0,      16,     12,     1.0f    },  // turret
    {   0,      0,      6,      14,     0.95f   },  // turret
    {   2,      0,      6,      12,     0.95f   },  // turret
    {   6,      0,      2,      10,     0.95f   },  // turret
    {   7,      0,      2,      6,      0.95f   },  // turret
    {   13,     0,      16,     2,      0.95f   }   // barrel
};

static sRectList blkegl_turret_list = {
    sizeof(blkegl_turret_rects) / sizeof(sRect),
        blkegl_turret_rects
};

static cModel   blkegl_turret_model(&blkegl_turret_list);
