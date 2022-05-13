#pragma once

#include "SimObj.h"
#include <iostream>
#include <string>
#include <fstream>

#pragma warning(disable : 4996)


extern SKSETaskInterface *g_task;

std::unordered_map<UInt32, SimObj> actors;

//void UpdateWorldDataToChild(NiAVObject)
void dumpTransform(NiTransform t) {
	Console_Print("%8.2f %8.2f %8.2f", t.rot.data[0][0], t.rot.data[0][1], t.rot.data[0][2]);
	Console_Print("%8.2f %8.2f %8.2f", t.rot.data[1][0], t.rot.data[1][1], t.rot.data[1][2]);
	Console_Print("%8.2f %8.2f %8.2f", t.rot.data[2][0], t.rot.data[2][1], t.rot.data[2][2]);

	Console_Print("%8.2f %8.2f %8.2f", t.pos.x, t.pos.y, t.pos.z);
	Console_Print("%8.2f", t.scale);
}


bool visitObjects(NiAVObject  *parent, std::function<bool(NiAVObject*, int)> functor, int depth = 0) {
	if (!parent) return false;
	NiNode * node = parent->GetAsNiNode();
	if (node) {
		if (functor(parent, depth))
			return true;

		for (UInt32 i = 0; i < node->m_children.m_emptyRunStart; i++) {
			NiAVObject * object = node->m_children.m_data[i];
			if (object) {
				if (visitObjects(object, functor, depth + 1))
					return true;
			}
		}
	}
	else if (functor(parent, depth))
		return true;

	return false;
}

std::string spaces(int n) {
	auto s = std::string(n, ' ');
	return s;
}

bool printStuff(NiAVObject *avObj, int depth) {
	std::string sss = spaces(depth);
	const char *ss = sss.c_str();
	logger.info("%savObj Name = %s, RTTI = %s\n", ss, avObj->m_name, avObj->GetRTTI()->name);

	NiNode *node = avObj->GetAsNiNode();
	if (node) {
		logger.info("%snode %s, RTTI %s\n", ss, node->m_name, node->GetRTTI()->name);
	}
	return false;
}


void dumpVec(NiPoint3 p) {
	logger.info("%8.2f %8.2f %8.2f\n", p.x, p.y, p.z);
}




template<class T>
inline void safe_delete(T*& in) {
	if (in) {
		delete in;
		in = NULL;
	}
}

#ifdef RUNTIME_VR_VERSION_1_4_15


#elif RUNTIME_VERSION_1_5_97 || RUNTIME_VERSION_1_5_80 || RUNTIME_VERSION_1_5_73

#elif RUNTIME_VERSION_1_5_62 || RUNTIME_VERSION_1_5_53 || RUNTIME_VERSION_1_5_50

void TESObjectREFR::IncRef()
{
	handleRefObject.IncRef();
}

void TESObjectREFR::DecRef()
{
	handleRefObject.DecRef();
}

typedef bool(*_LookupREFRByHandle2)(UInt32 & refHandle, NiPointer<TESObjectREFR> & refrOut);
extern RelocAddr<_LookupREFRByHandle2> LookupREFRByHandle2;

RelocAddr<_LookupREFRByHandle2> LookupREFRByHandle2(0x00132A90);

#elif RUNTIME_VERSION_1_5_39 || RUNTIME_VERSION_1_5_23

void TESObjectREFR::IncRef()
{
	handleRefObject.IncRef();
}

void TESObjectREFR::DecRef()
{
	handleRefObject.DecRef();
}

typedef bool(*_LookupREFRByHandle2)(UInt32 & refHandle, NiPointer<TESObjectREFR> & refrOut);
extern RelocAddr<_LookupREFRByHandle2> LookupREFRByHandle2;

RelocAddr<_LookupREFRByHandle2> LookupREFRByHandle2(0x00132A20);

#endif


AIProcessManager* AIProcessManager::GetSingleton()
{
#ifdef RUNTIME_VR_VERSION_1_4_15

	static RelocPtr<AIProcessManager*> singleton(0x01F831B0); //For VR 1.4.15

#elif RUNTIME_VERSION_1_5_97 || RUNTIME_VERSION_1_5_80 || RUNTIME_VERSION_1_5_73

	static RelocPtr<AIProcessManager*> singleton(0x01EBEAD0); //For SSE 1.5.73 and up

#elif RUNTIME_VERSION_1_5_62 || RUNTIME_VERSION_1_5_53 || RUNTIME_VERSION_1_5_50 || RUNTIME_VERSION_1_5_39

	static RelocPtr<AIProcessManager*> singleton(0x01EE5AD0); //For SSE 1.5.39 - 1.5.62

#elif RUNTIME_VERSION_1_5_23

	static RelocPtr<AIProcessManager*> singleton(0x01EE4A50); //For SSE 1.5.23

#endif

	return *singleton;
}

std::atomic<TESObjectCELL *> curCell = nullptr;

int frameCount = 0;

int cbpcenabled = 0;

void updateActors() {
	/*LARGE_INTEGER startingTime, endingTime, elapsedMicroseconds;
	LARGE_INTEGER frequency;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&startingTime);*/

	// We scan the cell and build the list every time - only look up things by ID once
	// we retain all state by actor ID, in a map - it's cleared on cell change
	actorEntries.clear();

	frameCount++;
	if (frameCount % (120) == 0)
	{
		std::string	runtimeDirectory = GetRuntimeDirectory();
		std::string filepath = runtimeDirectory + "Data\\SKSE\\Plugins\\CBPCAnalSupport\\CBPAnalCollisionSupportEnabled.txt";
		std::ifstream file(filepath);

		int parsedmode = 0;

		std::ifstream input_file(filepath);
		if (!input_file.is_open()) {
			//idk
		}
		else {
			parsedmode = std::stoi(std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>()));
		}

		if (parsedmode != cbpcenabled)
		{
			cbpcenabled = parsedmode;
			loadMasterConfig();
			loadCollisionConfig();
			loadExtraCollisionConfig();
			actors.clear();
		}
	}
	if (frameCount >= 1000000)
		frameCount = 0;

	if (tuningModeCollision != 0)
	{
		frameCount++;
		if (frameCount % (120 * tuningModeCollision) == 0)
		{
			loadMasterConfig();
			loadCollisionConfig();
			loadExtraCollisionConfig();

#ifdef RUNTIME_VR_VERSION_1_4_15
			LoadWeaponCollisionConfig();
#endif
			actors.clear();
		}
		if (frameCount >= 1000000)
			frameCount = 0;
	}


	//logger.error("scan Cell\n");
	if (!(*g_thePlayer) || !(*g_thePlayer)->loadedState)
		return;

	AIProcessManager* processMan = AIProcessManager::GetSingleton();
	TESObjectCELL* cell = (*g_thePlayer)->parentCell;
	if (!cell)
		return;

	callCount = 0;

	//TESNPC * actorNPC;
	Actor* actor;
	bool female = true;

	float xLow = 9999999.0;
	float xHigh = -9999999.0;
	float yLow = 9999999.0;
	float yHigh = -9999999.0;
	float zLow = 9999999.0;
	float zHigh = -9999999.0;

	NiPoint3 actorPos;
	
	NiPointer<TESObjectREFR> ToRefr = NULL;
	
	if (cell != curCell.load() || raceSexMenuClosed.load())
	{
		raceSexMenuClosed.store(false);
		//logger.error("cell change %d\n", cell);
		curCell.store(cell);
		actors.clear();
	}
	else
	{
		if (processMan)
		{
			for (UInt32 i = 0; i < processMan->actorsHigh.count+1; i++)
			{
				if (i < processMan->actorsHigh.count)
				{
#ifdef RUNTIME_VR_VERSION_1_4_15
					LookupREFRByHandle(processMan->actorsHigh[i], ToRefr);
#elif RUNTIME_VERSION_1_5_97 || RUNTIME_VERSION_1_5_80 || RUNTIME_VERSION_1_5_73
					LookupREFRByHandle(processMan->actorsHigh[i], ToRefr);
#else
					LookupREFRByHandle2(processMan->actorsHigh[i], ToRefr);
#endif
				}
				if (ToRefr != nullptr || i == processMan->actorsHigh.count)
				{
					if (i == processMan->actorsHigh.count)
					{
						actor = (*g_thePlayer);
					}
					else
					{
						actor = DYNAMIC_CAST(ToRefr, TESObjectREFR, Actor);
					}

					if (actor && actor->loadedState && actor->loadedState->node)
					{
						if (actor->race && actor != (*g_thePlayer))
						{
							std::string actorRace = actor->race->fullName.GetName();

							if (!(actor->race->data.raceFlags & TESRace::kRace_AllowPCDialogue) && !(actor->race->data.raceFlags & TESRace::kRace_Playable))
							{
								if (extraRacesList.empty())
								{
									continue;
								}
								else if (std::find(extraRacesList.begin(), extraRacesList.end(), actorRace) == extraRacesList.end())
								{
									continue;
								}
							}
							else if (actor->race->data.raceFlags & TESRace::kRace_Child)
								continue;

							LOG("actorRace: %s", actorRace.c_str());
						}

						if (actor->loadedState->node)
						{
							//Getting border values;
							actorPos = actor->loadedState->node->m_worldTransform.pos;

							if (distanceNoSqrt((*g_thePlayer)->loadedState->node->m_worldTransform.pos, actorPos) > actorDistance)
								continue;
							
							if (xLow > actorPos.x)
								xLow = actorPos.x;
							if (xHigh < actorPos.x)
								xHigh = actorPos.x;
							if (yLow > actorPos.y)
								yLow = actorPos.y;
							if (yHigh < actorPos.y)
								yHigh = actorPos.y;
							if (zLow > actorPos.z)
								zLow = actorPos.z;
							if (zHigh < actorPos.z)
								zHigh = actorPos.z;
						}

						auto soIt = actors.find(actor->formID);
						if (soIt == actors.end()) 
						{
							//logger.info("Tracking Actor with form ID %08x in cell %ld\n", actor->formID, actor->parentCell);
							if (IsActorValid(actor))
							{
								auto obj = SimObj(actor, config);
								actors.emplace(actor->formID, obj);
								actorEntries.emplace_back(ActorEntry{ actor->formID, actor, IsActorMale(actor) ? 1 : 0 });
							}
						}
						else if (IsActorValid(actor))
						{
							actorEntries.emplace_back(ActorEntry{ actor->formID, actor, IsActorMale(actor) ? 1 : 0 });
						}
					}
				}
			}
		}
	}

	#ifdef RUNTIME_VR_VERSION_1_4_15
	auto playerIt = actors.find(0x14);
	if (playerIt != actors.end())
	{
		UpdatePlayerColliders(playerIt->second.actorColliders);
	}
	#endif

	/*if(frameCount % 45 == 0)
	LOG_INFO("Cell borders: xLow:%f  xHigh:%f  yLow:%f  yHigh:%f  zLow:%f  zHigh:%f", xLow, xHigh, yLow, yHigh, zLow, zHigh);*/

	if (xLow < 9999999 && yLow < 9999999 && zLow < 9999999 && xHigh > -9999999 && yHigh > -9999999 && zHigh > -9999999)
	{
		xLow -= 100.0;
		yLow -= 100.0;
		zLow -= 100.0;
		xHigh += 100.0;
		yHigh += 100.0;
		zHigh += 100.0;

		LOG("ActorCount: %d", actorEntries.size());
				
		//Spatial Hashing
		hashSize = floor((xHigh - xLow) / gridsize) * floor((yHigh - yLow) / gridsize) * floor((zHigh - zLow) / gridsize);

		/*if (frameCount % 45 == 0)
		LOG_INFO("Hashsize=%d", hashSize);*/

		partitions.clear();

		LOG("Starting collider hashing");
		std::vector<long> ids;

		std::vector<long> hashIdList;
				
		for (int u = 0; u < actorEntries.size(); u++)
		{
			auto objIt = actors.find(actorEntries[u].id);
			if (objIt != actors.end())
			{				
				UpdateColliderPositions(objIt->second.actorColliders);

				for (auto &collider : objIt->second.actorColliders)
				{
					ids.clear();
					for (int j = 0; j < collider.second.collisionSpheres.size(); j++)
					{
						hashIdList = GetHashIdsFromPos(collider.second.collisionSpheres[j].worldPos, collider.second.collisionSpheres[j].radius100, hashSize);

						for (int m = 0; m < hashIdList.size(); m++)
						{
							if (std::find(ids.begin(), ids.end(), hashIdList[m]) != ids.end())
								continue;
							else
							{
								//LOG_INFO("ids.emplace_back(%d)", hashIdList[m]);
								ids.emplace_back(hashIdList[m]);
								partitions[hashIdList[m]].partitionCollisions.emplace_back(collider.second);
							}
						}
					}
					#ifdef RUNTIME_VR_VERSION_1_4_15
					for (int j = 0; j < collider.second.collisionTriangles.size(); j++)
					{
						for (int k = 0; k < 101; k = k + 20)
						{
							NiPoint3 pos = GetPointFromPercentage(collider.second.collisionTriangles[j].a, collider.second.collisionTriangles[j].b, k);

							long id = GetHashIdFromPos(pos, hashSize);
							if (id != -1)
							{
								if (std::find(ids.begin(), ids.end(), id) != ids.end())
									continue;
								else
								{
									ids.emplace_back(id);
									partitions[id].partitionCollisions.emplace_back(collider.second);
								}
							}
						}
						for (int k = 0; k < 101; k = k + 20)
						{
							NiPoint3 pos = GetPointFromPercentage(collider.second.collisionTriangles[j].a, collider.second.collisionTriangles[j].c, k);

							long id = GetHashIdFromPos(pos, hashSize);
							if (id != -1)
							{
								if (std::find(ids.begin(), ids.end(), id) != ids.end())
									continue;
								else
								{
									ids.emplace_back(id);
									partitions[id].partitionCollisions.emplace_back(collider.second);
								}
							}
						}
						for (int k = 0; k < 101; k = k + 20)
						{
							NiPoint3 pos = GetPointFromPercentage(collider.second.collisionTriangles[j].b, collider.second.collisionTriangles[j].c, k);

							long id = GetHashIdFromPos(pos, hashSize);
							if (id != -1)
							{
								if (std::find(ids.begin(), ids.end(), id) != ids.end())
									continue;
								else
								{
									ids.emplace_back(id);
									partitions[id].partitionCollisions.emplace_back(collider.second);
								}
							}
						}
					}
					#endif
				}
			}
		}
		//LOG_INFO("End of collider hashing");
		//LOG_INFO("Partitions size=%d", partitions.size());
	}
	//static bool done = false;
	//if (!done && player->loadedState->node) {
	//	visitObjects(player->loadedState->node, printStuff);
	//	BSFixedString cs("UUNP");
	//	auto bodyAV = player->loadedState->node->GetObjectByName(&cs.data);
	//	BSTriShape *body = bodyAV->GetAsBSTriShape();
	//	logger.info("GetAsBSTriShape returned  %lld\n", body);
	//	auto geometryData = body->geometryData;
	//	//logger.info("Num verts = %d\n", geometryData->m_usVertices);


	//	done = true;
	//}
	LOG("Hashsize=%d", hashSize);

	static int count = 0;
	if (configReloadCount && count++ > configReloadCount) {
		count = 0;
		loadConfig();
		for (auto &a : actors) {
			a.second.updateConfig(config);
		}
	}
	//logger.error("Updating %d entites\n", actorEntries.size());

	if (useParallelProcessing >= 2)
	{
		concurrency::parallel_for_each(actorEntries.begin(), actorEntries.end(), [&](const auto& a)
		{
			auto objIt = actors.find(a.id);
			if (objIt == actors.end())
			{
				//logger.error("missing Sim Object\n");
			}
			else
			{
				SimObj &obj = objIt->second;
				if (obj.isBound())
				{
					obj.update(a.actor);
				}
				else
				{
					obj.bind(a.actor, config, a.sex == 1);
				}
			}
		});
	}
	else
	{
		for each(auto &a in actorEntries) 
		{
			auto objIt = actors.find(a.id);
			if (objIt == actors.end()) 
			{
				//logger.error("missing Sim Object\n");
			}
			else 
			{
				SimObj &obj = objIt->second;
				if (obj.isBound()) 
				{
					obj.update(a.actor);
				}
				else 
				{				
					obj.bind(a.actor, config, a.sex == 1);
				}
			}
		}
	}
	LOG("Collider Check Call Count: %d", callCount);

	/*QueryPerformanceCounter(&endingTime);
	elapsedMicroseconds.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
	elapsedMicroseconds.QuadPart *= 1000000000LL;
	elapsedMicroseconds.QuadPart /= frequency.QuadPart;
	LOG_INFO("Update Time = %lld ns\n", elapsedMicroseconds.QuadPart);*/

	//logger.info("Update Time = %lld ns\n", elapsedMicroseconds.QuadPart);

	return;

}

/*
class ScanDelegate : public TaskDelegate {
public:
virtual void Run() {
updateActors();
}
virtual void Dispose() {
delete this;
}

};


void scaleTest() {
g_task->AddTask(new ScanDelegate());
return;
}
*/