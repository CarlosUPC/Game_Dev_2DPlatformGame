#ifndef __COIN_H__
#define __COIN_H__

#include "j1Entity.h"

class Coin :public j1Entity {
public:
	Coin(int x, int y);
	~Coin();

	bool Start();
	bool Update(float dt);
	bool CleanUp();
	
	void PushBack();

};
#endif // !__COIN_H__
