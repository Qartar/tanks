
//inherit data...?

class tank

	private:
		vector <int> T_position;	//position in 2D space of tank	-need to take values for p1/p2 starting positions?
		vector <int> P_position;	//position in 2D space of bullet	-need to take values for p1/p2 starting positions?
		//vector <int> velocity;		//velocity in 2D space -- calculate from x,y position & velocity
		int direction;				//tank direcion
		int A_velocity;				//object/tank angular velocity
		int Turret_dir;				//turret direction 
	//	int Turret_vel;				//turret angular velocity - same as tank velocity
		int ammo;					//counter for amunition remaining
		int damage;					//counter- damage recieved
		int Bullet_dir				//direciton of projectile
		int Bullet_vel;				//projectile angular velocity
			


	public:
		void DoPhysics();			//runs timestep using physics and user input
		void Draw();				//adds to current frame
		void Update();				//updates from user input
		void Fire();				//fires projectile from tank
		bool Hit();					//tank recieves damage- based on hit/miss...?
//		void Respawn();				//resets tank values to defaults
		void accelerate();			//checks agains max speed and incriments A_velocity
		void decelerate();			//checks agains min speed and decelerates/reverses 

	tank::tank()
	{
			ammo = 25;		
			health= 100;
			damage= 0;
			A_velocity = 0;
			Turret_vel = 0;
	}		//	resets values to standard- health/ammo/damage/veclocities		--locations, randomly generated or set?


/////////////////////////////////////////////////////////////
//	Fire Function
//	checks to see if ammo is sufficient, and assigns a velocity to a bullet--  may not need to render particles- may just register hit then display explosion on enemy (assuming projectile is really fast)
/////////////////////////////////////////////////////////////
		void tank::Fire() {
			
			if ammo > 0;
				ammo--;

		Bullet_vel= Bullet 			//assign velocity to projectile			-necessary to add in tank velocity?
		Bullet_dir=Turret_dir;		//fires bullet in direction turret is pointing
		projectile					//need to create projectile w/ velocity and direction based on turret
		}


/////////////////////////////////////////////////////////////
//	Hit Function
//	checks to see if projectile hits tank
/////////////////////////////////////////////////////////////
		bool tank::Hit() {		//need seperate position for tank and also particle? 
		
			if (T_position[]== P_position[])	//check to see if tank and particle hit- 2D position
		{	damage= damage+20;
			health = health -20;
			return true;		}
		}

		
		
/////////////////////////////////////////////////////////////
//	update function
//	get user input from buffer/queue and act accordingly 
/////////////////////////////////////////////////////////////		
		
		void tank::Update()		//updates from user input...
		{
			accelerate()				//going to have to account for multiple presses of button, or will it sense the button being held down?
			{	if (A_velocity > 30) {	//if tank is at max speed- do not accelerate
					A_velocity = 30;
				}	
				else					//otherwise make tank go faster.. up to 30 apparently
					A_velocity++;
			}

			decelerate()
			{
				if (A_velocity > -10)	//if tank is not at max reverse, slow down
				A_velocity--;
			}





		
/////////////////////////////////////////////////////////////
//	draw function
//	
/////////////////////////////////////////////////////////////


		
/////////////////////////////////////////////////////////////
//	do physics
//
/////////////////////////////////////////////////////////////