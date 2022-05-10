#include "SimObj.h"



// Note we don't ref count the nodes becasue it's ignored when the Actor is deleted, and calling Release after that can corrupt memory
const char *leftBreastName = "NPC L Breast";
const char *rightBreastName = "NPC R Breast";
const char *leftButtName = "NPC L Butt";
const char *rightButtName = "NPC R Butt";
const char *bellyName = "HDT Belly";
const char *leftPussy = "NPC L Pussy02";
const char *rightPussy = "NPC R Pussy02";
const char *pelvis = "NPC Pelvis [Pelv]";

//const char *scrotumName = "NPC GenitalsScrotum [GenScrot]";
//const char *leftScrotumName = "NPC L GenitalsScrotum [LGenScrot]";
//const char *rightScrotumName = "NPC R GenitalsScrotum [RGenScrot]";

std::vector<std::string> femaleSpecificBones = { leftBreastName, rightBreastName, leftButtName, rightButtName, bellyName, pelvis };

//std::unordered_map<const char *, std::string> configMap = {
//	{ leftBreastName, "Breast" },{ rightBreastName, "Breast" },
//	{ leftButtName, "Butt" },{ rightButtName, "Butt" },
//	{ bellyName, "Belly" },{ scrotumName, "Scrotum" } };


SimObj::SimObj(Actor *actor, config_t &config)
	: things(5){
	id = actor->formID;
}

SimObj::~SimObj() {
}


bool SimObj::bind(Actor *actor, config_t &config, bool isMale)
{
	//logger.error("bind\n");
	
	auto loadedState = actor->loadedState;
	if (loadedState && loadedState->node) 
	{
		bound = true;

		things.clear();
		actorColliders.clear();
		const std::string actorRace = actor->race->fullName.GetName();

		for (int i = 0; i<affectedBones.size(); i++)
		{
			if (isMale && malePhysics == 0 && !(malePhysicsOnlyForExtraRaces != 0 && std::find(extraRacesList.begin(), extraRacesList.end(), actorRace) != extraRacesList.end()))
			{
				if (std::find(femaleSpecificBones.begin(), femaleSpecificBones.end(), affectedBones.at(i)) != femaleSpecificBones.end())
				{
					continue;
				}
			}
			BSFixedString cs = ReturnUsableString(affectedBones.at(i));
			auto bone = loadedState->node->GetObjectByName(&cs.data);
			if (bone) 
			{
				things.emplace(cs.data, Thing(actor, bone, cs));
			}
		}
		updateConfig(config);
				
		CreateActorColliders(actor, actorColliders);

		#ifdef RUNTIME_VR_VERSION_1_4_15
		if (actor->formID == 0x14)
		{
			CreatePlayerColliders(actorColliders);
		}
		#endif

		return true;
	}
	return false;
}

bool IsActorValid(Actor *actor) {
	if (actor->flags & TESForm::kFlagIsDeleted)
		return false;
	if (actor && actor->loadedState && actor->loadedState->node)
		return true;
	return false;
}


void SimObj::update(Actor *actor) {
	if (!bound)
		return;
	//logger.error("update\n");
	if (useParallelProcessing == 1)
	{
		concurrency::parallel_for_each(things.begin(), things.end(), [&](auto& t)
		{
			if (strcmp(t.first, pelvis) == 0)
			{
				t.second.updatePelvis(actor);
			}
			else
			{
				t.second.update(actor);
			}
		});
	}
	else
	{
		for (auto &t : things) 
		{
			if (strcmp(t.first, pelvis) == 0)
			{
				t.second.updatePelvis(actor);
			}
			else
			{
				t.second.update(actor);
			}
		}
	}
	//logger.error("end SimObj update\n");
}

bool SimObj::updateConfig(config_t & config) {
	for (auto &t : things) {
		//LOG("t.first:[%s]", t.first);

		std::string &section = configMap[t.first];
		//LOG("config section:[%s]", section.c_str());
		auto &centry = config[section];
		t.second.updateConfig(centry);
	}
	return true;
}

