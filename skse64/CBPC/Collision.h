#pragma once
#include <string>
#include <time.h>
#include <vector>

#include "skse64/GameRTTI.h"
#include "skse64/GameReferences.h"
#include "skse64/NiNodes.h"
#include "skse64/NiGeometry.h"
#include "skse64/NiRTTI.h"

#include "config.h"

#ifdef RUNTIME_VR_VERSION_1_4_15
#include "skse64/GameVR.h"
#endif

static NiPoint3 emptyPoint = NiPoint3(0, 0, 0);

class Collision
{

public:

	Collision(NiAVObject* node, std::vector<Sphere> colliderSpheres, float actorWeight);
	~Collision();

	float CollidedWeight = 50;

	float ColliderWeight = 50;

	Actor* colliderActor;

	NiPoint3 lastColliderPosition = emptyPoint;
		
	bool Collision::IsItColliding(NiPoint3 &collisiondif, std::vector<Sphere> thingCollisionSpheres, std::vector<Sphere> collisionSpheres, float maxOffset, bool maybe);
	
	NiPoint3 CheckCollision(bool &isItColliding, std::vector<Sphere> thingCollisionSpheres, float timeTick, long deltaT, float maxOffset, bool maybe);

	NiPoint3 CheckPelvisCollision(std::vector<Sphere> thingCollisionSpheres);
	std::vector<Sphere> collisionSpheres;
	
	NiAVObject* CollisionObject;
	std::string colliderNodeName;

	#ifdef RUNTIME_VR_VERSION_1_4_15
	bool Collision::IsItCollidingTriangleToSphere(NiPoint3 &collisiondif, std::vector<Sphere> thingCollisionSpheres, std::vector<Triangle> collisionTriangles, float maxOffset, bool maybe);

	std::vector<Triangle> collisionTriangles;
	
	NiPoint3 MultiplyVector(NiPoint3 A, NiPoint3 B);
	NiPoint3 DotProduct(NiPoint3 A, NiPoint3 B);
	float Dot(NiPoint3 A, NiPoint3 B);
	NiPoint3 FindClosestPointOnTriangletoPoint(Triangle T, NiPoint3 P);
	#endif
};

static inline NiPoint3 GetPointFromPercentage(NiPoint3 lowWeight, NiPoint3 highWeight, float weight)
{
	return ((highWeight - lowWeight)*(weight / 100)) + lowWeight;
}

static inline float distance(NiPoint3 po1, NiPoint3 po2)
{
	/*LARGE_INTEGER startingTime, endingTime, elapsedMicroseconds;
	LARGE_INTEGER frequency;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&startingTime);
	LOG("distance Start");*/
	float x = po1.x - po2.x;
	float y = po1.y - po2.y;
	float z = po1.z - po2.z;
	float result = std::sqrt(x*x + y*y + z*z);
	/*QueryPerformanceCounter(&endingTime);
	elapsedMicroseconds.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
	elapsedMicroseconds.QuadPart *= 1000000000LL;
	elapsedMicroseconds.QuadPart /= frequency.QuadPart;
	LOG("distance Update Time = %lld ns\n", elapsedMicroseconds.QuadPart);*/
	return result;
}
