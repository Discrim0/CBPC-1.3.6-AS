#pragma once
#include <unordered_map>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>

#include "skse64/NiGeometry.h"
#include "skse64\GameReferences.h"
#include "skse64\PapyrusEvents.h"
#include "skse64\GameRTTI.h"
#include "skse64\GameSettings.h"
#include "skse64_common/skse_version.h"

#include "skse64/PapyrusNativeFunctions.h"

#include "log.h"
#include "Utility.hpp"
#include "skse64/GameMenus.h"
#include <atomic>

#ifdef RUNTIME_VR_VERSION_1_4_15
#include "skse64/openvr_1_0_12.h"
#endif

typedef std::unordered_map<std::string, float> configEntry_t;
typedef std::unordered_map<std::string, configEntry_t> config_t;

extern std::unordered_map<std::string, std::string> configMap;

extern int configReloadCount;
extern config_t config;

extern int collisionSkipFrames;
extern int collisionSkipFramesPelvis;

void loadConfig();

extern int gridsize;
extern int adjacencyValue;
extern int tuningModeCollision;
extern int malePhysics;
extern int malePhysicsOnlyForExtraRaces;
extern float actorDistance;

extern float cbellybulge;
extern float cbellybulgemax;
extern float cbellybulgeposlowest;
extern std::vector<std::string> bellybulgenodesList;

extern float vaginaOpeningLimit;
extern float AnusOpeningLimit;
extern float vaginaOpeningMultiplier;
extern float bellyBulgeReturnTime;

extern std::vector<std::string> extraRacesList;

extern int logging;

extern int useParallelProcessing;

extern std::atomic<bool> dialogueMenuOpen;
extern std::atomic<bool> raceSexMenuClosed;

enum eLogLevels
{
	LOGLEVEL_ERR = 0,
	LOGLEVEL_WARN,
	LOGLEVEL_INFO,
};

void Log(const int msgLogLevel, const char * fmt, ...);

#define LOG(fmt, ...) Log(LOGLEVEL_WARN, fmt, ##__VA_ARGS__)
#define LOG_ERR(fmt, ...) Log(LOGLEVEL_ERR, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) Log(LOGLEVEL_INFO, fmt, ##__VA_ARGS__)


#ifdef RUNTIME_VR_VERSION_1_4_15
extern unsigned short hapticFrequency;
extern int hapticStrength;

extern bool leftHandedMode;
#endif

//Collision Stuff

struct Sphere
{
	NiPoint3 offset0 = NiPoint3(0, 0, 0);
	NiPoint3 offset100 = NiPoint3(0, 0, 0);
	double radius0 = 4.0;
	double radius100 = 4.0;
	double radius100pwr2 = 16.0;
	NiPoint3 worldPos = NiPoint3(0, 0, 0);
	std::string NodeName;
};

struct ConfigLine
{
	std::vector<Sphere> CollisionSpheres;
	std::string NodeName;
};

struct SpecificNPCConfig
{
	std::vector<std::string> charactersList;
	std::vector<std::string> raceList;

	std::vector<std::string> AffectedNodeLines;
	std::vector<std::string> ColliderNodeLines;

	std::vector<ConfigLine> AffectedNodesList;

	std::vector<ConfigLine> ColliderNodesList;

	float cbellybulge;
	float cbellybulgemax;
	float cbellybulgeposlowest;
	std::vector<std::string> bellybulgenodesList;
};

extern std::vector<SpecificNPCConfig> specificNPCConfigList;

#ifdef RUNTIME_VR_VERSION_1_4_15
extern std::vector<std::string> PlayerNodeLines;
extern std::vector<ConfigLine> PlayerNodesList;   //Player nodes that can collide nodes
std::string GetWeaponTypeName(UInt8 kType);
void GetSettings();

extern float MeleeWeaponTranslateX;
extern float MeleeWeaponTranslateY;
extern float MeleeWeaponTranslateZ;
//Weapon Collision Stuff

struct Triangle
{
	NiPoint3 orga;
	NiPoint3 orgb;
	NiPoint3 orgc;

	NiPoint3 a;
	NiPoint3 b;
	NiPoint3 c;
};

struct WeaponConfigLine
{
	Triangle CollisionTriangle;
	std::string WeaponName;
};

extern std::vector<WeaponConfigLine> WeaponCollidersList; //Weapon colliders

std::vector<Triangle> GetCollisionTriangles(std::string name, UInt8 kType);

void ConfigWeaponLineSplitter(std::string &line, Triangle &newTriangle);

void LoadWeaponCollisionConfig();
#endif

class AllMenuEventHandler : public BSTEventSink <MenuOpenCloseEvent> {
public:
	virtual EventResult	ReceiveEvent(MenuOpenCloseEvent * evn, EventDispatcher<MenuOpenCloseEvent> * dispatcher);
};

extern AllMenuEventHandler menuEvent;

void MenuOpened(std::string name);

void MenuClosed(std::string name);





extern std::vector<std::string> AffectedNodeLines;
extern std::vector<std::string> ColliderNodeLines;



extern std::vector<ConfigLine> AffectedNodesList; //Nodes that can be collided with

extern std::vector<ConfigLine> ColliderNodesList; //Nodes that can collide nodes

void loadCollisionConfig();
void loadMasterConfig();
void loadExtraCollisionConfig();

void loadSystemConfig();

void ConfigLineSplitter(std::string &line, Sphere &newSphere);

int GetConfigSettingsValue(std::string line, std::string &variable);
std::string GetConfigSettingsStringValue(std::string line, std::string &variable);

float GetConfigSettingsFloatValue(std::string line, std::string &variable);

void printSpheresMessage(std::string message, std::vector<Sphere> spheres);

std::vector<std::string> ConfigLineVectorToStringVector(std::vector<ConfigLine> linesList);

extern std::vector<std::string> affectedBones;

bool GetSpecificNPCConfigForActor(std::string actorRefName, std::string actorRace, SpecificNPCConfig &snc);

bool IsActorMale(Actor* actor);




bool RegisterFuncs(VMClassRegistry* registry);
BSFixedString GetVersion(StaticFunctionTag* base);
BSFixedString GetVersionMinor(StaticFunctionTag* base);
BSFixedString GetVersionBeta(StaticFunctionTag* base);