#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Input.h"
#include "j1Textures.h"
#include "j1Audio.h"
#include "j1Render.h"
#include "j1Window.h"
#include "j1Map.h"
#include "j1MainMenu.h"
#include "j1Scene.h"
#include "j1Collision.h"
#include "j1FadeToBlack.h"
#include "j1Pathfinding.h"
#include "Player.h"
#include "j1Gui.h"
#include "Image.h"
#include "Button.h"
#include "Label.h"


#ifdef _WIN32	
#include <shellapi.h>
void open_url(const p2SString& url)
{
	ShellExecute(GetActiveWindow(), "open", url.GetString(), NULL, NULL, SW_SHOWNORMAL);
}
#else
void open_url(const p2SString& url) {}
#endif

j1MainMenu::j1MainMenu() : j1Module()
{
	name.create("menu");
}

// Destructor
j1MainMenu::~j1MainMenu()
{}

// Called before render is available

bool j1MainMenu::Awake(pugi::xml_node& conf)
{
	LOG("Loading Scene");
	
	lvl0.create(conf.child("level_0").child_value());
	bool ret = true;

	return ret;
}

// Called before the first frame
bool j1MainMenu::Start()
{
	bool ret = true;

	
	App->map->Load(lvl0.GetString());

	App->render->camera.y = -150;
	App->render->camera.x = -330;

	move_camera = false;
	camera_limit = -2030;
	camera_step_move = 20;

	button_limit = 403;
	
	win_width = App->win->screen_surface->w;
	win_height = App->win->screen_surface->h;

	title1 = App->gui->CreateLabel(win_width/6, 30, "KITTY", false, false, App->gui->screen, RED, 160, "fonts/04B_30__.ttf");
	
	title2 = App->gui->CreateLabel(win_width/6 + 100, 180, "LEE", false, false,App->gui->screen, WHITE, 160, "fonts/04B_30__.ttf");
	press_space = App->gui->CreateLabel(win_width / 3 + 30, win_height - 70, "PRESS SPACE TO START", false, false, App->gui->screen, WHITE, 32, "fonts/Munro.ttf");


	new_game_btn = (Button*)App->gui->CreateButton(win_width / 2 - 80, win_height / 2 - 25 + 250, { 182,148,189,49 }, App->gui->screen, {181,92,191,49 }, { 181,42,190,45 });
	buttons.PushBack(new_game_btn);
	
	new_game_lbl = (Label*)App->gui->CreateLabel(0,0, "PLAY",false,false,new_game_btn, WHITE, 20, "fonts/Munro.ttf");
	new_game_btn->partner = new_game_lbl;
	labels.PushBack(new_game_lbl);


	credits_btn = (Button*)App->gui->CreateButton(win_width / 2 - 80, win_height / 2 - 25 + 250 + 225, { 182,148,189,49 }, App->gui->screen, { 181,92,191,49 }, { 181,42,190,45 });
	buttons.PushBack(credits_btn);
	credits_lbl = (Label*)App->gui->CreateLabel(0, 0, "CREDITS", false, false, credits_btn, WHITE, 20, "fonts/Munro.ttf");
	credits_btn->partner = credits_lbl;
	labels.PushBack(credits_lbl);

	continue_btn = (Button*)App->gui->CreateButton(win_width / 2 - 80, win_height / 2 - 25 + 50 + 275, { 182,148,189,49 }, App->gui->screen, { 181,92,191,49 }, { 181,42,190,45 });
	buttons.PushBack(continue_btn);
	continue_lbl = (Label*)App->gui->CreateLabel(0, 0, "CONTINUE", false, false, continue_btn, WHITE, 20, "fonts/Munro.ttf");
	continue_btn->partner = continue_lbl;
	labels.PushBack(continue_lbl);
	
	settings_btn = (Button*)App->gui->CreateButton(win_width / 2 - 80, win_height / 2 - 25 + 250 + 150, { 182,148,189,49 }, App->gui->screen, { 181,92,191,49 }, { 181,42,190,45 });
	buttons.PushBack(settings_btn);
	settings_btn->AddListener(this);
	settings_lbl = (Label*)App->gui->CreateLabel(0, 0, "SETTINGS", false, false, settings_btn, WHITE, 20, "fonts/Munro.ttf");
	settings_btn->partner = settings_lbl;
	labels.PushBack(settings_lbl);


	quit_game_btn = (Button*)App->gui->CreateButton(10, win_height + 160, { 6,308,49,50 }, App->gui->screen, { 64,309,49,49 }, { 123,312,49,45 });
	buttons.PushBack(quit_game_btn);
	quit_game_btn->AddListener(this);
	github_btn = (Button*)App->gui->CreateButton(win_width - 60, win_height + 100, { 5, 191, 49, 50 }, App->gui->screen, { 62, 191, 51, 49 }, { 122,194,49,45 });
	github_btn->AddListener(this);
	buttons.PushBack(github_btn);
	website_btn = (Button*)App->gui->CreateButton(win_width - 60, win_height + 160, { 6,429,50,49 }, App->gui->screen, { 65,429,49,49 }, { 124,432,49,45 });
	website_btn->AddListener(this);
	buttons.PushBack(website_btn);

	for (int i = 0; i < buttons.Count(); i++)
	{
		buttons[i]->interactable = false;
		buttons[i]->drawable = false;

		if (buttons[i]->partner != nullptr)
			buttons[i]->CenterTextInButton();
		
	}

	for (int i = 0; i < labels.Count(); i++)
	{
		
		labels[i]->drawable = false;
	}



	return ret;
}

// Called each loop iteration
bool j1MainMenu::PreUpdate()
{
	return true;
}

// Called each loop iteration
bool j1MainMenu::Update(float dt)
{
	/*if (App->input->GetKey(SDL_SCANCODE_T) == KEY_DOWN) {
		App->current_lvl = Levels::FIRST_LEVEL;
		App->fade->FadeToBlack();
	}*/
	if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN /*&& !move_camera*/)
	{
		press_space->drawable = false;

	}

	if (move_camera && App->render->camera.x > camera_limit)
		App->render->camera.x -= camera_step_move;


	else if (!press_space->drawable)
	{
		for (int i = 0; i < buttons.Count(); i++)
			buttons[i]->drawable = true;

		for (int i = 0; i < labels.Count(); i++)
			labels[i]->drawable = true;

		if (new_game_btn->GetLocalPosition().y > button_limit && !new_game_btn->interactable)
		{
			for (int i = 0; i < buttons.Count(); i++)
				buttons[i]->SetPos(buttons[i]->GetLocalPosition().x,  buttons[i]->GetLocalPosition().y - camera_step_move);
		}

	
	}

	if (new_game_btn->GetLocalPosition().y <= button_limit)
	{
		for (int i = 0; i < buttons.Count(); i++)
			buttons[i]->interactable = true;
	}

	if (move_camera) {
		
		for (int i = 0; i < buttons.Count(); i++)
		{
			buttons[i]->interactable = false;
			buttons[i]->drawable = false;

		}

		for (int i = 0; i < labels.Count(); i++)
		{
			labels[i]->drawable = false;
		}
	}
	App->map->Draw();
	return true;
}

// Called each loop iteration
bool j1MainMenu::PostUpdate()
{
	bool ret = true;

	return ret;
}

// Called before quitting
bool j1MainMenu::CleanUp()
{
	LOG("Freeing main_menu");
	
	return true;
}

void j1MainMenu::UI_Events(UIElement* element, Mouse_Event action) {

	switch (action) {

	case Mouse_Event::CLICKED:

		if (element == (UIElement*)github_btn)
		{
			open_url("https://github.com/CarlosUPC/Kitty-Lee");
		}

		if (element == (UIElement*)website_btn)
		{
			open_url("https://marcianosmx.com/wp-content/uploads/2011/12/big_gorilla-e1322887919475.jpg");
		}

		if (element == (UIElement*)quit_game_btn)
		{
			App->quit_game = true;
		}

		if (element == (UIElement*)settings_btn)
		{
			move_camera = true;
		}
		break;
	}

}

bool j1MainMenu::Load(pugi::xml_node& load)
{
	bool ret = true;

	

	return ret;
}

bool j1MainMenu::Save(pugi::xml_node& save) const
{
	bool ret = true;


	return ret;
}

