#pragma once
#include <time.h>
#include <string>
#include <vector>

#include "skse64\NiNodes.h"
#include "skse64\PapyrusActorValueInfo.h"
#include "skse64\NiTypes.h"
#include "skse64\NiObjects.h"

#include "CollisionHub.h"


extern const char *leftPussy;
extern const char *rightPussy;

class Thing {
	BSFixedString boneName;
	NiPoint3 oldWorldPos;
	NiPoint3 velocity;
	clock_t time;

public:
	float stiffness = 0.5f;
	float stiffness2 = 0.0f;
	float damping = 0.2f;
	float maxOffset = 5.0f;
	float cogOffset = 0.0f; //no
	float gravityBias = 0.0f; //no
	float gravityCorrection = 0.0f; //no
	//float zOffset = 0.0f;	// Computed based on GravityBias value
	float timeTick = 4.0f;
	float linearX = 0;
	float linearY = 0;
	float linearZ = 0;
	float rotational = 0.1;
	float timeStep = 1.0f;	

	Thing(Actor *actor, NiAVObject *obj, BSFixedString &name);
	~Thing();

	Actor *ownerActor;
	NiAVObject * node;

	float actorWeight = 50;

	float bellybulgemultiplier = 2.0f;
	float bellybulgeposmultiplier = -3.0f;
	float bellybulgemax = 10.0f;
	float bellybulgeposlowest = -12.0f;
	std::vector<std::string> bellybulgelist = { "Genitals01" };

	void updateConfig(configEntry_t &centry);
	void dump();
	
	void update(Actor *actor);
	void updatePelvis(Actor *actor);
	bool ApplyBellyBulge(Actor * actor);
	void CalculateDiffVagina(NiPoint3 &collisionDiff, float opening, bool left);
	void reset();


	//Collision Stuff
	//--------------------------------------------------------------

	//Performance skip
	int skipFramesCount = 0;
	int skipFramesPelvisCount = 0;
	bool collisionOnLastFrame = false;


	NiPoint3 lastColliderPosition = emptyPoint;

	std::vector<Sphere> thingCollisionSpheres;

	std::vector<Collision> ownColliders;


	std::vector<Sphere> CreateThingCollisionSpheres(Actor * actor, std::string nodeName, float nodescale);

	double movementMultiplier = 1.0;

	bool updatePussyFirstRun = true;
	NiPoint3 leftPussyDefaultPos;
	NiPoint3 rightPussyDefaultPos;
	NiPoint3 leftAnusDefaultPos;
	NiPoint3 rightAnusDefaultPos;
	NiPoint3 upperAnusDefaultPos;
	NiPoint3 lowerAnusDefaultPos;

	bool updateBellyFirstRun = true;
	NiPoint3 bellyDefaultPos;

	bool updateThingFirstRun = true;
	NiPoint3 thingDefaultPos;
};