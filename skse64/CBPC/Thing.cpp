#include "Thing.h"

BSFixedString leftPus("NPC L Pussy02");
BSFixedString rightPus("NPC R Pussy02");
BSFixedString belly("HDT Belly");
BSFixedString pelvis("NPC Pelvis [Pelv]");
BSFixedString leftAnus("NPC LB Anus2");
BSFixedString rightAnus("NPC RB Anus2");
BSFixedString upperAnus("NPC RT Anus2");
BSFixedString downAnus("NPC LT Anus2");


Thing::Thing(Actor * actor, NiAVObject *obj, BSFixedString &name)
	: boneName(name)
	, velocity(NiPoint3(0, 0, 0))
{
	float nodescale = 1.0f;
	if (actor)
	{
		if (actor->loadedState && actor->loadedState->node)
		{
			//NiAVObject* obj = actor->loadedState->node->GetObjectByName(&name.data);
			if (obj)
			{
				nodescale = obj->m_worldTransform.scale;

				ownerActor = actor;
				node = obj;
			}
			else
			{
				return;
			}
		}
		else
		{
			return;
		}
	}
	else
	{
		return;
	}

	thingCollisionSpheres = CreateThingCollisionSpheres(actor, name.data, nodescale);

	oldWorldPos = obj->m_worldTransform.pos;
	time = clock();

	if(updateThingFirstRun)
	{
		updateThingFirstRun = false;

		auto mypair = std::make_pair(actor->baseForm->formID, name.data);
		std::map<std::pair<UInt32, const char *>, NiPoint3>::const_iterator posMap = thingDefaultPosList.find(mypair);

		if (posMap == thingDefaultPosList.end())
		{
			if(strcmp(name.data, belly.data) == 0)
			{
				thingDefaultPos = emptyPoint;
			}
			else
			{
				//Add it to the list
				thingDefaultPos = obj->m_localTransform.pos;
			}
			thingDefaultPosList[mypair] = thingDefaultPos;
			LOG("Adding %s to default list for %08x -> %g %g %g", name.data, actor->baseForm->formID, thingDefaultPos.x, thingDefaultPos.y, thingDefaultPos.z);
		}
		else
		{
			if (strcmp(name.data, belly.data) == 0)
			{
				thingDefaultPos = emptyPoint;
			}
			else
			{
				thingDefaultPos = posMap->second;
			}
		}
		
		LOG_INFO("%s default pos -> %g %g %g", boneName.data, thingDefaultPos.x, thingDefaultPos.y, thingDefaultPos.z);
	}

	skipFramesCount = collisionSkipFrames;
	skipFramesPelvisCount = collisionSkipFramesPelvis;
}

Thing::~Thing() {
}

std::vector<Sphere> Thing::CreateThingCollisionSpheres(Actor * actor, std::string nodeName, float nodescale)
{
	auto actorRef = DYNAMIC_CAST(actor, Actor, TESObjectREFR);
	
	actorWeight = CALL_MEMBER_FN(actorRef, GetWeight)();

	std::vector<ConfigLine>* AffectedNodesListPtr;

	const char * actorrefname = "";
	std::string actorRace = "";

	SpecificNPCConfig snc;

	if (actor->formID == 0x14) //If Player
	{
		actorrefname = "Player";
	}
	else
	{
		actorrefname = CALL_MEMBER_FN(actorRef, GetReferenceName)();
	}

	if (actor->race)
	{
		actorRace = actor->race->fullName.GetName();
	}

	bool success = GetSpecificNPCConfigForActor(actorrefname, actorRace, snc);

	if (success)
	{
		AffectedNodesListPtr = &(snc.AffectedNodesList);
		bellybulgemultiplier = snc.cbellybulge;
		bellybulgemax = snc.cbellybulgemax;
		bellybulgeposlowest = snc.cbellybulgeposlowest;
		bellybulgelist = snc.bellybulgenodesList;
	}
	else
	{
		AffectedNodesListPtr = &AffectedNodesList;
		bellybulgemultiplier = cbellybulge;
		bellybulgemax = cbellybulgemax;
		bellybulgeposlowest = cbellybulgeposlowest;
		bellybulgelist = bellybulgenodesList;
	}

	std::vector<Sphere> spheres;

	for (int i = 0; i < AffectedNodesListPtr->size(); i++)
	{
		if (AffectedNodesListPtr->at(i).NodeName == nodeName)
		{
			spheres = AffectedNodesListPtr->at(i).CollisionSpheres;

			for(int j=0; j<spheres.size(); j++)
			{
				spheres[j].offset100 = GetPointFromPercentage(spheres[j].offset0, spheres[j].offset100, actorWeight);

				spheres[j].radius100 = GetPercentageValue(spheres[j].radius0, spheres[j].radius100, actorWeight)*nodescale;

				spheres[j].radius100pwr2 = spheres[j].radius100*spheres[j].radius100;
			}
			break;
		}
	}
	return spheres;
}

void showPos(NiPoint3 &p) {
	logger.info("%8.2f %8.2f %8.2f\n", p.x, p.y, p.z);
}

void showRot(NiMatrix33 &r) {
	logger.info("%8.2f %8.2f %8.2f\n", r.data[0][0], r.data[0][1], r.data[0][2]);
	logger.info("%8.2f %8.2f %8.2f\n", r.data[1][0], r.data[1][1], r.data[1][2]);
	logger.info("%8.2f %8.2f %8.2f\n", r.data[2][0], r.data[2][1], r.data[2][2]);
}


float solveQuad(float a, float b, float c) {
	float k1 = (-b + sqrtf(b*b - 4*a*c)) / (2 * a);
	//float k2 = (-b - sqrtf(b*b - 4*a*c)) / (2 * a);
	//logger.error("k2 = %f\n", k2);
	return k1;
}

void Thing::updateConfig(configEntry_t & centry) {
	stiffness = centry["stiffness"];
	stiffness2 = centry["stiffness2"];
	damping = centry["damping"];
	maxOffset = centry["maxoffset"];
	timeTick = centry["timetick"];
	linearX = centry["linearX"];
	linearY = centry["linearY"];
	linearZ = centry["linearZ"];
	rotational = centry["rotational"];
	// Optional entries for backwards compatability 
	if (centry.find("timeStep") != centry.end())
		timeStep = centry["timeStep"];
	else 
		timeStep = 1.0f;
	gravityBias = centry["gravityBias"];
	gravityCorrection = centry["gravityCorrection"];
	cogOffset = centry["cogOffset"];
	if (timeTick <= 1)
		timeTick = 1;

	//zOffset = solveQuad(stiffness2, stiffness, -gravityBias);

	//logger.error("z offset = %f\n", solveQuad(stiffness2, stiffness, -gravityBias));
}

void Thing::dump() {
	//showPos(obj->m_worldTransform.pos);
	//showPos(obj->m_localTransform.pos);
}

void Thing::reset() {
	// TODO
}

template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

void Thing::updatePelvis(Actor *actor)
{
	if (skipFramesPelvisCount > 0)
	{
		skipFramesPelvisCount--;
		return;
	}
	else
	{
		skipFramesPelvisCount = collisionSkipFramesPelvis;
	}

	/*if (CheckPelvisArmor(actor))
	{
		return;
	}*/

	if (hashSize <= 0)
		return;

	/*LARGE_INTEGER startingTime, endingTime, elapsedMicroseconds;
	LARGE_INTEGER frequency;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&startingTime);*/

	auto loadedState = actor->loadedState;

	if (!loadedState || !loadedState->node) {
		return;
	}

	NiAVObject* leftPusObj = loadedState->node->GetObjectByName(&leftPus.data);
	NiAVObject* rightPusObj = loadedState->node->GetObjectByName(&rightPus.data);
	NiAVObject* leftAnusObj = loadedState->node->GetObjectByName(&leftAnus.data);
	NiAVObject* rightAnusObj = loadedState->node->GetObjectByName(&rightAnus.data);
	NiAVObject* upperAnusObj = loadedState->node->GetObjectByName(&upperAnus.data);
	NiAVObject* downAnusObj = loadedState->node->GetObjectByName(&downAnus.data);
		
	if (!leftPusObj || !rightPusObj)
	{
		if (!leftAnusObj || !rightAnusObj || upperAnusObj || downAnusObj) {
			if (updatePussyFirstRun)
			{
				updatePussyFirstRun = false;
				leftAnusDefaultPos = leftAnusObj->m_localTransform.pos;
				rightAnusDefaultPos = rightAnusObj->m_localTransform.pos;
				upperAnusDefaultPos = upperAnusObj->m_localTransform.pos;
				lowerAnusDefaultPos = downAnusObj->m_localTransform.pos;
			}

			leftAnusObj->m_localTransform.pos = leftAnusDefaultPos;
			rightAnusObj->m_localTransform.pos = rightAnusDefaultPos;
			upperAnusObj->m_localTransform.pos = upperAnusDefaultPos;
			downAnusObj->m_localTransform.pos = lowerAnusDefaultPos;

			// Collision Stuff Start
			NiPoint3 collisionVector = emptyPoint;

			NiMatrix33 pelvisRotation;
			NiPoint3 pelvisPosition;

			NiAVObject* pelvisObj = loadedState->node->GetObjectByName(&pelvis.data);
			if (!pelvisObj)
				return;

			pelvisRotation = pelvisObj->m_worldTransform.rot;
			pelvisPosition = pelvisObj->m_worldTransform.pos;

			std::vector<long> thingIdList;
			std::vector<long> hashIdList;
			for (int i = 0; i < thingCollisionSpheres.size(); i++)
			{
				thingCollisionSpheres[i].worldPos = pelvisPosition + (pelvisRotation * thingCollisionSpheres[i].offset100);
				hashIdList = GetHashIdsFromPos(thingCollisionSpheres[i].worldPos, thingCollisionSpheres[i].radius100, hashSize);
				for (int m = 0; m < hashIdList.size(); m++)
				{
					if (!(std::find(thingIdList.begin(), thingIdList.end(), hashIdList[m]) != thingIdList.end()))
					{
						thingIdList.emplace_back(hashIdList[m]);
					}
				}
			}

			NiPoint3 collisionDiff = emptyPoint;

			for (int j = 0; j < thingIdList.size(); j++)
			{
				long id = thingIdList[j];
				if (partitions.find(id) != partitions.end())
				{
					//LOG_INFO("Pelvis hashId=%d", id);
					for (int i = 0; i < partitions[id].partitionCollisions.size(); i++)
					{
						if (partitions[id].partitionCollisions[i].colliderActor == actor && partitions[id].partitionCollisions[i].colliderNodeName.find("Genital") != std::string::npos)
							continue;

						InterlockedIncrement(&callCount);
						partitions[id].partitionCollisions[i].CollidedWeight = actorWeight;

						collisionDiff = partitions[id].partitionCollisions[i].CheckPelvisCollision(thingCollisionSpheres);
						collisionVector = collisionVector + collisionDiff;
					}
				}
			}

			// Collision Stuff End

			NiPoint3 leftVector = collisionVector;
			NiPoint3 rightVector = collisionVector;
			float opening = distance(collisionVector, emptyPoint);

			CalculateDiffVagina(leftVector, opening, true);
			CalculateDiffVagina(rightVector, opening, false);

			NormalizeNiPoint(leftVector, vaginaOpeningLimit * -1.0f, vaginaOpeningLimit);
			NormalizeNiPoint(rightVector, vaginaOpeningLimit * -1.0f, vaginaOpeningLimit);

			//
			NiPoint3 upperVector;
			upperVector.x = rightVector.y;
			upperVector.y = leftVector.x;
			upperVector.z = leftVector.z;
			NiPoint3 downVector;
			downVector.x = rightVector.y;
			downVector.y = rightVector.x;
			downVector.z = rightVector.z;

			leftAnusObj->m_localTransform.pos = leftAnusDefaultPos + downVector;
			rightAnusObj->m_localTransform.pos = rightAnusDefaultPos + upperVector;
			upperAnusObj->m_localTransform.pos = upperAnusDefaultPos + rightVector;
			downAnusObj->m_localTransform.pos = lowerAnusDefaultPos + leftVector;
		}
		return;
	}
	else
	{
		if(updatePussyFirstRun)
		{
			updatePussyFirstRun = false;
			
			auto leftpair = std::make_pair(actor->baseForm->formID, leftPus.data);
			std::map<std::pair<UInt32, const char *>, NiPoint3>::const_iterator posMap = thingDefaultPosList.find(leftpair);

			if (posMap == thingDefaultPosList.end())
			{
				//Add it to the list
				leftPussyDefaultPos = leftPusObj->m_localTransform.pos;

				thingDefaultPosList[leftpair] = leftPussyDefaultPos;
				LOG("Adding %s to default list for %08x -> %g %g %g", leftPus.data, actor->baseForm->formID, leftPussyDefaultPos.x, leftPussyDefaultPos.y, leftPussyDefaultPos.z);

			}
			else
			{
				leftPussyDefaultPos = posMap->second;
			}

			auto rightpair = std::make_pair(actor->baseForm->formID, rightPus.data);
			posMap = thingDefaultPosList.find(rightpair);

			if (posMap == thingDefaultPosList.end())
			{
				//Add it to the list
				rightPussyDefaultPos = rightPusObj->m_localTransform.pos;

				thingDefaultPosList[rightpair] = rightPussyDefaultPos;
				LOG("Adding %s to default list for %08x -> %g %g %g", rightPus.data, actor->baseForm->formID, rightPussyDefaultPos.x, rightPussyDefaultPos.y, rightPussyDefaultPos.z);

				leftAnusDefaultPos = leftAnusObj->m_localTransform.pos;
				rightAnusDefaultPos = rightAnusObj->m_localTransform.pos;
				upperAnusDefaultPos = upperAnusObj->m_localTransform.pos;
				lowerAnusDefaultPos = downAnusObj->m_localTransform.pos;
			}
			else
			{
				rightPussyDefaultPos = posMap->second;
			}
			
			LOG_INFO("Left pussy default pos -> %g %g %g , Right pussy default pos ->  %g %g %g", leftPussyDefaultPos.x, leftPussyDefaultPos.y, leftPussyDefaultPos.z, rightPussyDefaultPos.x, rightPussyDefaultPos.y, rightPussyDefaultPos.z);
		}
		
		leftPusObj->m_localTransform.pos = leftPussyDefaultPos;

		rightPusObj->m_localTransform.pos = rightPussyDefaultPos;

		leftAnusObj->m_localTransform.pos = leftAnusDefaultPos;
		rightAnusObj->m_localTransform.pos = rightAnusDefaultPos;
		upperAnusObj->m_localTransform.pos = upperAnusDefaultPos;
		downAnusObj->m_localTransform.pos = lowerAnusDefaultPos;

	}

	// Collision Stuff Start
	NiPoint3 collisionVector = emptyPoint;

	NiMatrix33 pelvisRotation;
	NiPoint3 pelvisPosition;
	
	NiAVObject* pelvisObj = loadedState->node->GetObjectByName(&pelvis.data);
	if (!pelvisObj)
		return;
	
	pelvisRotation = pelvisObj->m_worldTransform.rot;
	pelvisPosition = pelvisObj->m_worldTransform.pos;

	std::vector<long> thingIdList;
	std::vector<long> hashIdList;
	for (int i = 0; i < thingCollisionSpheres.size(); i++)
	{
		thingCollisionSpheres[i].worldPos = pelvisPosition + (pelvisRotation*thingCollisionSpheres[i].offset100);
		hashIdList = GetHashIdsFromPos(thingCollisionSpheres[i].worldPos, thingCollisionSpheres[i].radius100, hashSize);
		for (int m = 0; m<hashIdList.size(); m++)
		{
			if (!(std::find(thingIdList.begin(), thingIdList.end(), hashIdList[m]) != thingIdList.end()))
			{
				thingIdList.emplace_back(hashIdList[m]);
			}
		}
	}	

	NiPoint3 collisionDiff = emptyPoint;
		
	for (int j = 0; j < thingIdList.size(); j++)
	{
		long id = thingIdList[j];
		if(partitions.find(id) != partitions.end())
		{
			//LOG_INFO("Pelvis hashId=%d", id);
			for (int i = 0; i < partitions[id].partitionCollisions.size(); i++)
			{
				if (partitions[id].partitionCollisions[i].colliderActor == actor && partitions[id].partitionCollisions[i].colliderNodeName.find("Genital") != std::string::npos)
					continue;

				InterlockedIncrement(&callCount);
				partitions[id].partitionCollisions[i].CollidedWeight = actorWeight;

				collisionDiff = partitions[id].partitionCollisions[i].CheckPelvisCollision(thingCollisionSpheres);
				collisionVector = collisionVector + collisionDiff;
			}
		}
	}

	// Collision Stuff End
	
	NiPoint3 leftVector = collisionVector;
	NiPoint3 rightVector = collisionVector;
	float opening = distance(collisionVector, emptyPoint);

	CalculateDiffVagina(leftVector, opening, true);
	CalculateDiffVagina(rightVector, opening, false);

	NormalizeNiPoint(leftVector, vaginaOpeningLimit * -1.0f, vaginaOpeningLimit);
	leftPusObj->m_localTransform.pos = leftPussyDefaultPos + leftVector;

	NormalizeNiPoint(rightVector, vaginaOpeningLimit * -1.0f, vaginaOpeningLimit);
	rightPusObj->m_localTransform.pos = rightPussyDefaultPos + rightVector;

	//
	NiPoint3 upperVector;
	upperVector.x = rightVector.y;
	upperVector.y = leftVector.x;
	upperVector.z = leftVector.z;
	NiPoint3 downVector;
	downVector.x = rightVector.y;
	downVector.y = rightVector.x;
	downVector.z = rightVector.z;

	leftAnusObj->m_localTransform.pos = leftAnusDefaultPos + downVector;
	rightAnusObj->m_localTransform.pos = rightAnusDefaultPos + upperVector;
	upperAnusObj->m_localTransform.pos = upperAnusDefaultPos + rightVector;
	downAnusObj->m_localTransform.pos = lowerAnusDefaultPos + leftVector;
}

float lastMaxOffsetY = 0.0f;
float lastMaxOffsetZ = 0.0f;
int bellyBulgeCountDown = 0;
float oldBulgeY = 0.0f;

bool Thing::ApplyBellyBulge(Actor * actor)
{
	NiPoint3 collisionVector = emptyPoint;
	
	if (hashSize <= 0)
		return false;

	NiMatrix33 pelvisRotation;
	NiPoint3 pelvisPosition;

	NiAVObject* bellyObj = actor->loadedState->node->GetObjectByName(&belly.data);
	if (!bellyObj)
		return false;

	if(updateBellyFirstRun)
	{
		updateBellyFirstRun = false;

		auto mypair = std::make_pair(actor->baseForm->formID, belly.data);
		std::map<std::pair<UInt32, const char *>, NiPoint3>::const_iterator posMap = thingDefaultPosList.find(mypair);

		//if (posMap == thingDefaultPosList.end())
		//{
		//	//Add it to the list
		//	bellyDefaultPos = bellyObj->m_localTransform.pos;

		//	thingDefaultPosList[mypair] = bellyDefaultPos;
		//	LOG("Adding %s to default list for %08x -> %g %g %g", belly.data, actor->baseForm->formID, bellyDefaultPos.x, bellyDefaultPos.y, bellyDefaultPos.z);
		//}
		//else
		//{
		//	bellyDefaultPos = posMap->second;
		//}
		//
		if (posMap == thingDefaultPosList.end())
		{
			//Add it to the list
			bellyDefaultPos = emptyPoint;

			thingDefaultPosList[mypair] = bellyDefaultPos;
			LOG("Adding %s to default list for %08x -> %g %g %g", belly.data, actor->baseForm->formID, bellyDefaultPos.x, bellyDefaultPos.y, bellyDefaultPos.z);
		}
		else
		{
			bellyDefaultPos = emptyPoint;
		}
		
		LOG_INFO("Belly default pos -> %g %g %g", bellyDefaultPos.x, bellyDefaultPos.y, bellyDefaultPos.z);
	}

	NiAVObject* pelvisObj = actor->loadedState->node->GetObjectByName(&pelvis.data);
	if (!pelvisObj)
		return false;

	pelvisRotation = pelvisObj->m_worldTransform.rot;
	pelvisPosition = pelvisObj->m_worldTransform.pos;

	std::vector<long> thingIdList;
	std::vector<long> hashIdList;
	
	std::vector<Sphere> pelvisCollisionSpheres;

	Sphere pelvisSphere;
	pelvisSphere.offset100 = NiPoint3(0, 0, -2);
	pelvisSphere.radius100 = 3.5f;
	pelvisSphere.radius100pwr2 = 12.25f;
	pelvisCollisionSpheres.emplace_back(pelvisSphere);

	for (int i = 0; i < pelvisCollisionSpheres.size(); i++)
	{
		pelvisCollisionSpheres[i].worldPos = pelvisPosition + (pelvisRotation*pelvisCollisionSpheres[i].offset100);
		hashIdList = GetHashIdsFromPos(pelvisCollisionSpheres[i].worldPos, pelvisCollisionSpheres[i].radius100, hashSize);
		for (int m = 0; m<hashIdList.size(); m++)
		{
			if (!(std::find(thingIdList.begin(), thingIdList.end(), hashIdList[m]) != thingIdList.end()))
			{
				thingIdList.emplace_back(hashIdList[m]);
			}
		}
	}

	NiPoint3 collisionDiff = emptyPoint;

	bool genitalPenetration = false;

	for (int j = 0; j < thingIdList.size(); j++)
	{
		long id = thingIdList[j];
		if (partitions.find(id) != partitions.end())
		{
			for (int i = 0; i < partitions[id].partitionCollisions.size(); i++)
			{
				if (partitions[id].partitionCollisions[i].colliderActor == actor)
					continue;
				
				for (int m = 0; m < bellybulgelist.size(); m++)
				{
					collisionDiff = emptyPoint;

					if (partitions[id].partitionCollisions[i].colliderNodeName.find(bellybulgelist[m]) != std::string::npos)
					{
						InterlockedIncrement(&callCount);

						partitions[id].partitionCollisions[i].CollidedWeight = actorWeight;

						collisionDiff = partitions[id].partitionCollisions[i].CheckPelvisCollision(pelvisCollisionSpheres);

						if (!CompareNiPoints(collisionDiff, emptyPoint))
						{
							genitalPenetration = true;
						}
					}

					collisionVector = collisionVector + collisionDiff;
				}
			}
		}
	}

	const float opening = distance(collisionVector, emptyPoint);

	if (opening > 0)
	{
		if (bellybulgemultiplier > 0 && genitalPenetration)
		{
			//LOG("opening:%g", opening);
			bellyBulgeCountDown = 1000;
			
			float horPos = opening * bellybulgemultiplier;
			horPos = clamp(horPos, 0.0f, bellybulgemax);
			bellyObj->m_localTransform.pos.y = bellyDefaultPos.y + horPos;

			//float vertPos = opening * bellybulgeposmultiplier;
			//vertPos = clamp(vertPos, bellybulgeposlowest, 0.0f);
			LOG("belly bulge vert:%g horiz:%g", bellybulgeposlowest, horPos);

			if (lastMaxOffsetY < horPos)
			{
				lastMaxOffsetY = abs(horPos);
			}
			if (lastMaxOffsetZ < bellybulgeposlowest)
			{
				lastMaxOffsetZ = abs(bellybulgeposlowest);
			}
			return true;
		}
	}
	if(bellyBulgeCountDown > 0)
	{
		bellyBulgeCountDown--;
		bellyObj->m_localTransform.pos.z = bellyDefaultPos.z + bellybulgeposlowest;
	}
	return false;
}

int frameCounts = 0;

void Thing::update(Actor *actor) {
	bool collisionsOn = true;
	if (skipFramesCount > 0)
	{
		skipFramesCount--;
		collisionsOn = false;
		if (collisionOnLastFrame)
		{
			return;
		}
	}
	else
	{
		skipFramesCount = collisionSkipFrames;	
		collisionOnLastFrame = false;
	}
		
	/*LARGE_INTEGER startingTime, endingTime, elapsedMicroseconds;
	LARGE_INTEGER frequency;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&startingTime);
	printMessageStr("Thing.update() start", boneName.data);*/
	
	auto newTime = clock();
	auto deltaT = newTime - time;

	time = newTime;
	if (deltaT > 64) deltaT = 64;
	if (deltaT < 8) deltaT = 8;

	NiMatrix33 objRotation;

	NiAVObject* obj;		
	auto loadedState = actor->loadedState;
	
#ifdef RUNTIME_VR_VERSION_1_4_15
	if ((*g_thePlayer) && actor == (*g_thePlayer)) //To check if we can support VR Body
	{
		NiNode* rootNodeTP = (*g_thePlayer)->GetNiRootNode(0);
		
		NiNode* rootNodeFP = (*g_thePlayer)->GetNiRootNode(2);

		NiNode* mostInterestingRoot = (rootNodeFP != nullptr) ? rootNodeFP : rootNodeTP;		

		obj = ni_cast(mostInterestingRoot->GetObjectByName(&boneName.data), NiNode);
		objRotation = mostInterestingRoot->GetAsNiNode()->m_worldTransform.rot;
	}
	else
	{
	#endif
		if (!loadedState || !loadedState->node) {
			logger.error("No loaded state for actor %08x\n", actor->formID);
			return;
		}

		obj = loadedState->node->GetObjectByName(&boneName.data);
		objRotation = loadedState->node->m_worldTransform.rot;
	#ifdef RUNTIME_VR_VERSION_1_4_15	
	}
	#endif
	if (!obj)
	{
		return;
	}

	if (bellybulgemultiplier > 0 && strcmp(boneName.data, belly.data) == 0)
	{
		if (ApplyBellyBulge(actor))
		{
			return;
		}
	}
	bool IsThereCollision = false;
	NiPoint3 collisionDiff = emptyPoint;
	long originalDeltaT = deltaT;
	NiPoint3 collisionVector = emptyPoint;

	float varCogOffset = cogOffset;
	float varGravityCorrection = -1*gravityCorrection;
	float varGravityBias = gravityBias;

	std::vector<long> thingIdList;
	std::vector<long> hashIdList;
	if (collisionsOn && hashSize>0)
	{
		//LOG("Before Collision Stuff Start");
		// Collision Stuff Start
		for (int i = 0; i < thingCollisionSpheres.size(); i++)
		{
			thingCollisionSpheres[i].worldPos = oldWorldPos + (objRotation*thingCollisionSpheres[i].offset100);
			//printNiPointMessage("thingCollisionSpheres[i].worldPos", thingCollisionSpheres[i].worldPos);
			hashIdList = GetHashIdsFromPos(thingCollisionSpheres[i].worldPos, thingCollisionSpheres[i].radius100, hashSize);
			for(int m=0; m<hashIdList.size(); m++)
			{
				if (!(std::find(thingIdList.begin(), thingIdList.end(), hashIdList[m]) != thingIdList.end()))
				{
					thingIdList.emplace_back(hashIdList[m]);
				}
			}
		}

		NiPoint3 lastcollisionVector = emptyPoint;

		for (int j = 0; j < thingIdList.size(); j++)
		{
			long id = thingIdList[j];
			if (partitions.find(id) != partitions.end())
			{
				for (int i = 0; i < partitions[id].partitionCollisions.size(); i++)
				{
					if (partitions[id].partitionCollisions[i].colliderActor == actor && partitions[id].partitionCollisions[i].colliderNodeName.find("Genital") != std::string::npos)
						continue;

					if (partitions[id].partitionCollisions[i].colliderActor == actor && std::strcmp(partitions[id].partitionCollisions[i].colliderNodeName.c_str(), boneName.data)==0 )
						continue;

					InterlockedIncrement(&callCount);

					partitions[id].partitionCollisions[i].CollidedWeight = actorWeight;

					if (!CompareNiPoints(lastcollisionVector, collisionVector))
					{
						for (int l = 0; l < thingCollisionSpheres.size(); l++)
						{
							thingCollisionSpheres[l].worldPos = oldWorldPos + (objRotation*thingCollisionSpheres[l].offset100) + collisionVector;
						}
					}
					lastcollisionVector = collisionVector;

					bool colliding = false;
					collisionDiff = partitions[id].partitionCollisions[i].CheckCollision(colliding, thingCollisionSpheres, timeTick, originalDeltaT, maxOffset, false);
					if (colliding)
						IsThereCollision = true;

					collisionVector = collisionVector + collisionDiff*movementMultiplier;
					collisionVector.x = clamp(collisionVector.x, -maxOffset, maxOffset);
					collisionVector.y = clamp(collisionVector.y, -maxOffset, maxOffset);
					collisionVector.z = clamp(collisionVector.z, -maxOffset, maxOffset);
				}
			}
		}
		if (IsThereCollision)
		{
			float timeMultiplier = timeTick / (float)deltaT;

			collisionVector *= timeMultiplier;

			varCogOffset = 0;
			varGravityCorrection = 0;
			varGravityBias = 0;
		}
		//LOG("After Collision Stuff");
	}

	NiPoint3 newPos = oldWorldPos;

	NiPoint3 posDelta = emptyPoint;

	NiPoint3 target = obj->m_parent->m_worldTransform * NiPoint3(0, varCogOffset, 0);
	
	if (!IsThereCollision)
	{
		//Offset to move Center of Mass make rotaional motion more significant  
		NiPoint3 diff = target - oldWorldPos;

		diff += NiPoint3(0, 0, varGravityCorrection);

		if (fabs(diff.x) > 100 || fabs(diff.y) > 100 || fabs(diff.z) > 100) //prevent shakes
		{
			//logger.error("transform reset\n");
			obj->m_localTransform.pos = thingDefaultPos;
			oldWorldPos = target;
			velocity = emptyPoint;
			time = clock();
			return;
		}

		float timeMultiplier = timeTick / (float)deltaT;
		diff *= timeMultiplier;

		// Compute the "Spring" Force
		NiPoint3 diff2(diff.x * diff.x * sgn(diff.x), diff.y * diff.y * sgn(diff.y), diff.z * diff.z * sgn(diff.z));
		NiPoint3 force = (diff * stiffness) + (diff2 * stiffness2) - NiPoint3(0, 0, varGravityBias);
		//showPos(diff);
		//showPos(force);
		do {
			// Assume mass is 1, so Accelleration is Force, can vary mass by changinf force
			//velocity = (velocity + (force * timeStep)) * (1 - (damping * timeStep));
			velocity = (velocity + (force * timeStep)) - (velocity * (damping * timeStep));

			// New position accounting for time
			posDelta += (velocity * timeStep);
			deltaT -= timeTick;
		} while (deltaT >= timeTick);

		if (collisionsOn && hashSize>0)
		{
			//LOG("Before Maybe Collision Stuff Start");
			NiPoint3 maybePos = newPos + posDelta;

			bool maybeNot = false;

			//After cbp movement collision detection
			thingIdList.clear();
			for (int i = 0; i < thingCollisionSpheres.size(); i++)
			{
				thingCollisionSpheres[i].worldPos = (objRotation*thingCollisionSpheres[i].offset100) + maybePos;
				hashIdList = GetHashIdsFromPos(thingCollisionSpheres[i].worldPos, thingCollisionSpheres[i].radius100, hashSize);
				for (int m = 0; m<hashIdList.size(); m++)
				{
					if (!(std::find(thingIdList.begin(), thingIdList.end(), hashIdList[m]) != thingIdList.end()))
					{
						thingIdList.emplace_back(hashIdList[m]);
					}
				}
			}
			//Prevent normal movement to cause collision (This prevents shakes)			
			collisionVector = emptyPoint;
			NiPoint3 lastcollisionVector = emptyPoint;
			for (int j = 0; j < thingIdList.size(); j++)
			{
				long id = thingIdList[j];
				//LOG_INFO("Thing hashId=%d", id);
				if (partitions.find(id) != partitions.end())
				{
					for (int i = 0; i < partitions[id].partitionCollisions.size(); i++)
					{
						if (partitions[id].partitionCollisions[i].colliderActor == actor && partitions[id].partitionCollisions[i].colliderNodeName.find("Genital") != std::string::npos)
							continue;

						if (partitions[id].partitionCollisions[i].colliderActor == actor && std::strcmp(partitions[id].partitionCollisions[i].colliderNodeName.c_str(), boneName.data) == 0)
							continue;

						InterlockedIncrement(&callCount);
						
						partitions[id].partitionCollisions[i].CollidedWeight = actorWeight;

						if (!CompareNiPoints(lastcollisionVector, collisionVector))
						{
							for (int l = 0; l < thingCollisionSpheres.size(); l++)
							{
								thingCollisionSpheres[l].worldPos = (objRotation*thingCollisionSpheres[l].offset100) + maybePos + collisionVector;
							}
						}
						lastcollisionVector = collisionVector;

						bool colliding = false;
						collisionDiff = partitions[id].partitionCollisions[i].CheckCollision(colliding, thingCollisionSpheres, timeTick, originalDeltaT, maxOffset, false);
						if (colliding)
						{
							velocity = emptyPoint;
							maybeNot = true;
							collisionVector = collisionVector + collisionDiff;
							collisionVector.x = clamp(collisionVector.x, -maxOffset, maxOffset);
							collisionVector.y = clamp(collisionVector.y, -maxOffset, maxOffset);
							collisionVector.z = clamp(collisionVector.z, -maxOffset, maxOffset);
						}
					}
				}
			}
			
			if (!maybeNot)
				newPos = newPos + posDelta;
			else
			{
				varGravityCorrection = 0;
				collisionVector *= timeMultiplier;
				newPos = maybePos + collisionVector;
			}

			//LOG("After Maybe Collision Stuff End");
		}
		else
			newPos = newPos + posDelta;
	}
	else
	{
		newPos = newPos + collisionVector;
		collisionOnLastFrame = true;
	}	
	
	// clamp the difference to stop the breast severely lagging at low framerates
	NiPoint3 diff = newPos - target;

	//Logging
	if (logging != 0)
	{
		std::string sex = "Female";
		if (IsActorMale(actor)) //Actor is male
			sex = "Male";

		auto actorRef = DYNAMIC_CAST(actor, Actor, TESObjectREFR);
		LOG("%s - %s - Thing.update() %s", CALL_MEMBER_FN(actorRef, GetReferenceName)(), sex, boneName.data);
		LOG("%s - %g %g %g", boneName.data, diff.x, diff.y, diff.z);
	}
	
	oldWorldPos = diff + target;

	//logger.error("set positions\n");
	// move the bones based on the supplied weightings
	// Convert the world translations into local coordinates
	auto invRot = obj->m_parent->m_worldTransform.rot.Transpose();
			
	if (bellybulgemultiplier > 0 && strcmp(boneName.data, belly.data) == 0)
	{
		auto localGravity = emptyPoint;
		if (varGravityCorrection < 0)
		{
			localGravity = invRot * NiPoint3(0, 0, varGravityCorrection);
		}
		
		float maxOffsetY = maxOffset;
		float maxOffsetZ = maxOffset;
		
		if (lastMaxOffsetY > maxOffset)
		{
			maxOffsetY = lastMaxOffsetY;
		}
		if (lastMaxOffsetZ > maxOffset)
		{
			maxOffsetZ = lastMaxOffsetZ;
		}
		
		auto ldiff = invRot * diff;
		
		oldWorldPos = (obj->m_parent->m_worldTransform.rot * ldiff) + target;
				
		float maxAllowedInOneFrame = bellybulgemax / (bellyBulgeReturnTime / ((float)originalDeltaT / (float)CLOCKS_PER_SEC));

		obj->m_localTransform.pos.x = thingDefaultPos.x + clamp(ldiff.x * linearX - localGravity.x, -maxOffset, maxOffset) + localGravity.x;
		obj->m_localTransform.pos.y = thingDefaultPos.y + clamp(ldiff.y * linearY - localGravity.y, 0.0f, maxOffsetY) + localGravity.y;

		if(oldBulgeY > 0 && oldBulgeY > obj->m_localTransform.pos.y)
		{
			float posDiff = clamp(oldBulgeY - obj->m_localTransform.pos.y, 0.0f, maxAllowedInOneFrame);

			obj->m_localTransform.pos.y = thingDefaultPos.y + oldBulgeY - posDiff;
		}

		if (bellyBulgeCountDown > 0)
		{
			obj->m_localTransform.pos.z = thingDefaultPos.z + bellybulgeposlowest;
		}
		else
		{
			obj->m_localTransform.pos.z = thingDefaultPos.z + clamp(ldiff.z * linearZ - localGravity.z, -maxOffsetZ, 0.0f) + localGravity.z;
		}
		
		lastMaxOffsetY = abs(obj->m_localTransform.pos.y);
		lastMaxOffsetZ = abs(obj->m_localTransform.pos.z);

		oldBulgeY = obj->m_localTransform.pos.y;

		auto rdiff = ldiff * rotational;
		obj->m_localTransform.rot.SetEulerAngles(0, 0, rdiff.z);
	}
	else
	{
		diff.x = clamp(diff.x, -maxOffset, maxOffset);
		diff.y = clamp(diff.y, -maxOffset, maxOffset);
		diff.z = clamp(diff.z - varGravityCorrection, -maxOffset, maxOffset) + varGravityCorrection;
		
		auto ldiff = invRot * diff;
		
		oldWorldPos = (obj->m_parent->m_worldTransform.rot * ldiff) + target;
		
		obj->m_localTransform.pos.x = thingDefaultPos.x + ldiff.x * linearX;
		obj->m_localTransform.pos.y = thingDefaultPos.y + ldiff.y * linearY;
		obj->m_localTransform.pos.z = thingDefaultPos.z + ldiff.z * linearZ;

		auto rdiff = ldiff * rotational;
		obj->m_localTransform.rot.SetEulerAngles(0, 0, rdiff.z);
	}

	//logger.error("end update()\n");
	/*QueryPerformanceCounter(&endingTime);
	elapsedMicroseconds.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
	elapsedMicroseconds.QuadPart *= 1000000000LL;
	elapsedMicroseconds.QuadPart /= frequency.QuadPart;
	LOG("Thing.update() Update Time = %lld ns\n", elapsedMicroseconds.QuadPart);*/
}

void Thing::CalculateDiffVagina(NiPoint3 &collisionDiff, float opening, bool left)
{
	if (opening > 0)
	{
		if (left)
		{
			collisionDiff = NiPoint3(vaginaOpeningMultiplier*-1, 0, 0)*(opening / 3);
		}
		else
		{
			collisionDiff = NiPoint3(vaginaOpeningMultiplier, 0, 0)*(opening / 3);
		}
	}
	else
	{
		collisionDiff = emptyPoint;
	}
}

