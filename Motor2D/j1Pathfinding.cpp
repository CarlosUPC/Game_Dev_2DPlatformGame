#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Pathfinding.h"

#include "Brofiler\Brofiler.h"

j1PathFinding::j1PathFinding() : j1Module(), map(NULL), last_path(DEFAULT_PATH_LENGTH),width(0), height(0)
{
	name.create("pathfinding");
}

// Destructor
j1PathFinding::~j1PathFinding()
{
	RELEASE_ARRAY(map);
}

// Called before quitting
bool j1PathFinding::CleanUp()
{
	LOG("Freeing pathfinding library");

	last_path.Clear();
	RELEASE_ARRAY(map);
	return true;
}

// Sets up the walkability map
void j1PathFinding::SetMap(uint width, uint height, uchar* data)
{
	this->width = width;
	this->height = height;

	RELEASE_ARRAY(map);
	map = new uchar[width*height];
	memcpy(map, data, width*height);
}

// Utility: return true if pos is inside the map boundaries
bool j1PathFinding::CheckBoundaries(const iPoint& pos) const
{
	return (pos.x >= 0 && pos.x <= (int)width &&
			pos.y >= 0 && pos.y <= (int)height);
}

// Utility: returns true is the tile is walkable
bool j1PathFinding::IsWalkable(const iPoint& pos) const
{
	uchar t = GetTileAt(pos);
	return t != INVALID_WALK_CODE && t > 0;
}

// Utility: return the walkability value of a tile
uchar j1PathFinding::GetTileAt(const iPoint& pos) const
{
	if(CheckBoundaries(pos))
		return map[(pos.y*width) + pos.x];

	return INVALID_WALK_CODE;
}

// To request all tiles involved in the last generated path
const p2DynArray<iPoint>* j1PathFinding::GetLastPath() const
{
	return &last_path;
}

// PathList ------------------------------------------------------------------------
// Looks for a node in this list and returns it's list node or NULL
// ---------------------------------------------------------------------------------
const p2List_item<PathNode>* PathList::Find(const iPoint& point) const
{
	p2List_item<PathNode>* item = list.start;
	while(item)
	{
		if(item->data.pos == point)
			return item;
		item = item->next;
	}
	return NULL;
}

// PathList ------------------------------------------------------------------------
// Returns the Pathnode with lowest score in this list or NULL if empty
// ---------------------------------------------------------------------------------
p2List_item<PathNode>* PathList::GetNodeLowestScore() const
{
	p2List_item<PathNode>* ret = NULL;
	int min = 65535;

	p2List_item<PathNode>* item = list.end;
	while(item)
	{
		if(item->data.Score() < min)
		{
			min = item->data.Score();
			ret = item;
		}
		item = item->prev;
	}
	return ret;
}

// PathNode -------------------------------------------------------------------------
// Convenient constructors
// ----------------------------------------------------------------------------------
PathNode::PathNode() : g(-1), h(-1), pos(-1, -1), parent(NULL)
{}

PathNode::PathNode(int g, int h, const iPoint& pos, const PathNode* parent) : g(g), h(h), pos(pos), parent(parent)
{}

PathNode::PathNode(const PathNode& node) : g(node.g), h(node.h), pos(node.pos), parent(node.parent)
{}

// PathNode -------------------------------------------------------------------------
// Fills a list (PathList) of all valid adjacent pathnodes
// ----------------------------------------------------------------------------------
uint PathNode::FindWalkableAdjacents(PathList& list_to_fill) const
{
	iPoint cell;

	// north
	cell.create(pos.x, pos.y + 1);
	if(App->pathfinding->IsWalkable(cell))
		list_to_fill.list.add(PathNode(-1, -1, cell, this));

	// south
	cell.create(pos.x, pos.y - 1);
	if(App->pathfinding->IsWalkable(cell))
		list_to_fill.list.add(PathNode(-1, -1, cell, this));

	// east
	cell.create(pos.x + 1, pos.y);
	if(App->pathfinding->IsWalkable(cell))
		list_to_fill.list.add(PathNode(-1, -1, cell, this));

	// west
	cell.create(pos.x - 1, pos.y);
	if(App->pathfinding->IsWalkable(cell))
		list_to_fill.list.add(PathNode(-1, -1, cell, this));

	return list_to_fill.list.count();
}

// PathNode -------------------------------------------------------------------------
// Calculates this tile score
// ----------------------------------------------------------------------------------
int PathNode::Score() const
{
	return g + h;
}

// PathNode -------------------------------------------------------------------------
// Calculate the F for a specific destination tile
// ----------------------------------------------------------------------------------
int PathNode::CalculateF(const iPoint& destination, TypePathDistance distance_type)
{
	g = parent->g + 1;
	h = CalculateDistance(pos, destination, distance_type);

	return g + h;
}

// ----------------------------------------------------------------------------------
// Actual A* algorithm: return number of steps in the creation of the path or -1 ----
// ----------------------------------------------------------------------------------
int j1PathFinding::CreatePath(const iPoint& origin, const iPoint& destination, TypePathDistance distance_type)
{
	BROFILER_CATEGORY("CreatePath", Profiler::Color::Azure);
	if (!IsWalkable(origin) || !IsWalkable(destination))
		return -1;
	last_path.Clear();

	PathList open;
	PathList close;

	open.list.add(PathNode(0, CalculateDistance(origin, destination, distance_type), origin, nullptr));

	p2List_item<PathNode>* changer = nullptr;
	PathNode* item = nullptr;

	while (open.list.count() > 0) {

		changer = open.GetNodeLowestScore();

		item = &close.list.add(changer->data)->data;
		open.list.del(changer);

		if (item->pos == destination) {
			PathNode* item_back = item;
			
			while (item_back != nullptr) {
				last_path.PushBack(item_back->pos);

				item_back = (PathNode*)item_back->parent;
			}
			last_path.Flip();
			open.list.clear();
			break;
		}

		PathList adjacent_list;
		item->FindWalkableAdjacents(adjacent_list);

		p2List_item<PathNode>* adjacent_node = adjacent_list.list.start;

		while (adjacent_node != nullptr) {
			if (close.Find(adjacent_node->data.pos) == NULL) {
				adjacent_node->data.CalculateF(destination, distance_type);
				const p2List_item<PathNode>* open_node = open.Find(adjacent_node->data.pos);
				if (open_node == NULL)
					open.list.add(adjacent_node->data);
			}
			adjacent_node = adjacent_node->next;
		}
		adjacent_list.list.clear();
	}

	return last_path.Count();
}


int CalculateDistance(iPoint origin, iPoint destination, TypePathDistance distance_type)
{
	int distance = 0;

	switch (distance_type) {
	case DISTANCE_TO:
		distance = origin.DistanceTo(destination);
		break;
	case DISTANCE_NO_SQRT:
		distance = origin.DistanceNoSqrt(destination);
		break;
	case MANHATTAN:
		distance = origin.DistanceManhattan(destination);
		break;
	default:
		distance = origin.DistanceManhattan(destination);
		break;
	}

	return distance;
}