#include <math.h>
#include "j1App.h"
#include "j1FadeToBlack.h"
#include "j1Render.h"
#include "j1Map.h"
#include "j1Window.h"
#include "j1Scene.h"
#include "j1Player.h"
#include "j1Audio.h"
#include "j1Input.h"
#include "SDL/include/SDL_render.h"
#include "SDL/include/SDL_timer.h"
#include "p2Log.h"


j1FadeToBlack::j1FadeToBlack()
{
	screen = { 0, 0, 1920, 1080 };
}

j1FadeToBlack::~j1FadeToBlack()
{}

bool j1FadeToBlack::Start()
{
	LOG("Preparing Fade Screen");
	SDL_SetRenderDrawBlendMode(App->render->renderer, SDL_BLENDMODE_BLEND);
	return true;
}

bool j1FadeToBlack::Update(float dt)
{
	if (current_step == fade_step::none)
		return true;

	Uint32 now = SDL_GetTicks() - start_time;
	float normalized = MIN(1.0f, (float)now / (float)total_time);

	switch (current_step)
	{
	case fade_step::fade_to_black:
	{
		if (now >= total_time)
		{
			
			//App->player->Start();
			if (level1) {
				//Switch to level 2
				SwitchingLevel(App->scene->lvl2.GetString());
				App->player->position = App->map->GetInitialPosition();
				App->scene->stg = LEVEL_2;
				App->render->camera.x = 0;
				App->render->camera.y = 0;
			}
			else if (level2) {
				//Switch to level 1
				SwitchingLevel(App->scene->lvl1.GetString());
				App->player->position = App->map->GetInitialPosition();
				App->scene->stg = LEVEL_1;
				App->render->camera.x = 0;
				App->render->camera.y = 0;
				
			}
			

			/*total_time += total_time;
			start_time = SDL_GetTicks();
			current_step = fade_step::fade_from_black;*/
		
			fading = false;
			current_step = fade_step::fade_from_black;
		}
	} break;

	case fade_step::fade_from_black:
	{
		normalized = 1.0f - normalized;

		if (now >= total_time)
		{
			current_step = fade_step::none;
		}


	} break;
	}

	// Render the black square with alpha on the screen
	SDL_SetRenderDrawColor(App->render->renderer, 0, 0, 0, (Uint8)(normalized * 255.0f));
	SDL_RenderFillRect(App->render->renderer, &screen);
	return true;
}

bool j1FadeToBlack::CleanUp()
{
	return true;
}

bool j1FadeToBlack::FadeToBlack(float time)
{
	bool ret = false;
	//App->player->CleanUp();
	level1 = true;

	if (current_step == fade_step::none)
	{
	
		current_step = fade_step::fade_to_black;
		start_time = SDL_GetTicks();
		total_time = (uint)(time * 0.5f * 1000.0f);
		fading = true;
		ret = true;
	}

	return ret;
	
}

bool j1FadeToBlack::SwitchingLevel(const char* tmx_map)
{
	bool ret = true;
	App->collider->EraseMapCollider();
	App->map->CleanUp();
	App->map->Load(tmx_map);
	App->map->AddCollidersMap();
	return ret;
}

bool j1FadeToBlack::IsFading()
{
	return current_step != fade_step::none;
}