
#include "j1App.h"
#include "j1Entity.h"
#include "j1Collision.h"
#include "j1Render.h"
#include "j1Textures.h"

#include "SDL/include/SDL_timer.h"
#include "j1Audio.h"
#include "p2Defs.h"
#include "p2Log.h"
#include "j1Map.h"
#include "j1Scene.h"
#include "p2List.h"
#include "j1Input.h"
#include "j1FadeToBlack.h"

#include <cmath>


j1Entity::j1Entity(Types type) : type(type) 
{}

j1Entity::~j1Entity()
{
	if (collider.collider != nullptr)
		collider.collider->to_delete = true;
}

const Collider* j1Entity::GetCollider() const
{
	return collider.collider;
}

void j1Entity::OnCollision(Collider* collider)
{
}

bool j1Entity::Update(float dt) {
	Move(dt);

	Draw(dt);

	return true;
}

void j1Entity::Draw(float dt) {
	App->render->Blit(data.tileset.texture, position.x, position.y);
}

bool j1Entity::CleanUp()
{
	return false;
}

void j1Entity::DeleteAnimation()
{
	//deleting entity animation data already loaded in its corresponding animation variables
	for (uint i = 0; i < data.num_animations; ++i) {		//this block of code delete animation data loaded of xml,
		if (data.animations[i].frames != nullptr) {			//is in PushBack() because when load all animation in its
			delete[] data.animations[i].frames;				//corresponding variables, that data is useless
			data.animations[i].frames = nullptr;
		}
	}
	if (data.animations != nullptr) {
		delete[] data.animations;
		data.animations = nullptr;
	}
}

bool j1Entity::LoadEntityData(const char* file) {

	bool ret = true;

	pugi::xml_parse_result result = entity_file.load_file(file);

	if (result == NULL)
	{
		LOG("Could not load map xml file %s. pugi error: %s", file, result.description());
		ret = false;
	}

	//fill tileset info
	pugi::xml_node node = entity_file.child("tileset");

	data.tileset.name.create(node.attribute("name").as_string());
	data.tileset.tilewidth = node.attribute("tilewidth").as_uint();
	data.tileset.tileheight = node.attribute("tileheight").as_uint();
	data.tileset.spacing = node.attribute("spacing").as_uint();
	data.tileset.margin = node.attribute("margin").as_uint();
	data.tileset.tilecount = node.attribute("tilecount").as_uint();
	data.tileset.columns = node.attribute("columns").as_uint();
	data.tileset.imagePath = node.child("image").attribute("source").as_string();
	data.tileset.width = node.child("image").attribute("width").as_uint();
	data.tileset.height = node.child("image").attribute("height").as_uint();

	//count how many animations are in file
	node = node.child("tile");
	data.num_animations = 0;
	while (node != NULL) {
		data.num_animations++;
		node = node.next_sibling("tile");
	}

	//reserve memory for all animations
	data.animations = new EntityAnim[data.num_animations];

	//count how many frames for each animation, assign memory for those frames and set id frame start
	node = entity_file.child("tileset").child("tile");
	for (uint i = 0; i < data.num_animations; ++i) {
		data.animations[i].FrameCount(node.child("animation").child("frame"));
		data.animations[i].frames = new SDL_Rect[data.animations[i].num_frames];
		data.animations[i].id = node.attribute("id").as_uint();
		node = node.next_sibling("tile");
	}

	//fill frame array with current information
	node = entity_file.child("tileset").child("tile");
	pugi::xml_node node_frame;
	for (uint i = 0; i < data.num_animations; ++i) {
		node_frame = node.child("animation").child("frame");
		for (uint j = 0; j < data.animations[i].num_frames; ++j) {
			data.animations[i].frames[j] = data.tileset.GetTileRect(node_frame.attribute("tileid").as_uint());
			node_frame = node_frame.next_sibling("frame");
		}
		node = node.next_sibling("tile");
	}
	//LOG all animation information
	for (uint i = 0; i < data.num_animations; ++i) {
		LOG("Animation %i--------", data.animations[i].id);
		for (uint j = 0; j < data.animations[i].num_frames; ++j) {
			LOG("frame %i x: %i y: %i w: %i h: %i",
				j, data.animations[i].frames[j].x, data.animations[i].frames[j].y,
				data.animations[i].frames[j].w, data.animations[i].frames[j].h);
		}
	}

	//Load data
	LoadProperties(entity_file.child("tileset").child("properties").child("property")); //Load properties, is a virtual function because every entity has its variables
	
	LoadCollider(entity_file.child("tileset").child("tile").child("objectgroup").child("object")); //Load collider

	//Convert id animations to enum
	IdAnimToEnum();

	Pushback();

	DeleteAnimation();

	return ret;
}

void j1Entity::LoadProperties(pugi::xml_node & node)
{
	p2SString nameIdentificator;
	while (node) {
		nameIdentificator = node.attribute("name").as_string();

		if (nameIdentificator == "AnimationSpeed")
			animationSpeed = node.attribute("value").as_float();

		node = node.next_sibling();
	}
}

void j1Entity::LoadCollider(pugi::xml_node &node)
{
	collider.offset.x = node.attribute("x").as_int();
	collider.offset.y = node.attribute("y").as_int();
	collider.width = node.attribute("width").as_uint();
	collider.height = node.attribute("height").as_uint();

	p2SString colliderType = node.attribute("type").as_string();
	if (colliderType == "Collider_enemy")
		collider.type = COLLIDER_TYPE::COLLIDER_ENEMY;
	else if(colliderType == "Collider_player")
		collider.type = COLLIDER_TYPE::COLLIDER_PLAYER;
}

void j1Entity::IdAnimToEnum()
{
	data.animations[0].animType = EntityState::IDLE;
}

//Functions to help loading data in xml-------------------------------------
//Get the rect info of an id of tileset
SDL_Rect TileSetEntity::GetTileRect(int id) const {
	SDL_Rect rect;
	rect.w = tilewidth;
	rect.h = tileheight;
	rect.x = margin + ((rect.w + spacing) * (id % columns));
	rect.y = margin + ((rect.h + spacing) * (id / columns));
	return rect;
}

//Return how many frames are in one animation
uint EntityAnim::FrameCount(pugi::xml_node& n) {
	num_frames = 0;
	pugi::xml_node node = n;
	for (; node != NULL; node = node.next_sibling("frame")) {
		num_frames++;
	}

	return num_frames;
}