#pragma once

#include "Collision.h"
#include "ActorEntry.h"
#include "skse64\PapyrusActor.h"


extern int gridsize;
extern int a;
extern int b;
extern int c;

extern long hashSize;

#ifdef RUNTIME_VR_VERSION_1_4_15
void CreatePlayerColliders(std::unordered_map<std::string, Collision> &actorCollidersList);
void UpdatePlayerColliders(std::unordered_map<std::string, Collision> &actorCollidersList);
#endif

extern long callCount;

void CreateActorColliders(Actor * actor, std::unordered_map<std::string, Collision> &actorCollidersList);

void UpdateColliderPositions(std::unordered_map<std::string, Collision> &colliderList);


struct Partition
{
	std::vector<Collision> partitionCollisions;
};

typedef std::unordered_map<long, Partition> PartitionMap;

extern PartitionMap partitions;



long GetHashIdFromPos(NiPoint3 pos, long size);

std::vector<long> GetHashIdsFromPos(NiPoint3 pos, float radius, long size);

bool CheckPelvisArmor(Actor* actor);



