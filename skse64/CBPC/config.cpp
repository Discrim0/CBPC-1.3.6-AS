#include "config.h"



#pragma warning(disable : 4996)
#ifdef RUNTIME_VR_VERSION_1_4_15
std::vector<std::string> PlayerNodeLines;
std::vector<ConfigLine> PlayerNodesList;   //Player nodes that can collide nodes
std::vector<WeaponConfigLine> WeaponCollidersList; //Weapon colliders
float MeleeWeaponTranslateX = 0.0;
float MeleeWeaponTranslateY = -5.0;
float MeleeWeaponTranslateZ = 0.0;

unsigned short hapticFrequency = 15;
int hapticStrength = 1;
bool leftHandedMode = false;
#endif
std::vector<std::string> extraRacesList;

std::vector<std::string> AffectedNodeLines;
std::vector<std::string> ColliderNodeLines;



std::vector<ConfigLine> AffectedNodesList; //Nodes that can be collided with

std::vector<ConfigLine> ColliderNodesList; //Nodes that can collide nodes

std::vector<std::string> affectedBones;



std::vector<SpecificNPCConfig> specificNPCConfigList;



int configReloadCount = 60;
int gridsize = 25;
int adjacencyValue = 5;
int tuningModeCollision = 0;
int malePhysics = 0;
int malePhysicsOnlyForExtraRaces = 0;

float actorDistance = 4194304.0f;

float cbellybulge = 2;
float cbellybulgemax = 10;
float cbellybulgeposlowest = -12;

float bellyBulgeReturnTime = 1.5f;
std::vector<std::string> bellybulgenodesList;

float vaginaOpeningLimit = 5.0f;
float vaginaOpeningMultiplier = 4.0f;

int logging = 0;

int useParallelProcessing = 0;

std::atomic<bool> dialogueMenuOpen = false;
std::atomic<bool> raceSexMenuClosed = false;

std::unordered_map<std::string, std::string> configMap;
config_t config;

int collisionSkipFrames = 0; //0
int collisionSkipFramesPelvis = 5; //5

void loadConfig() {
	char buffer[1024];
	//logger.info("loadConfig\n");

	//Console_Print("Reading CBP Config");
	std::string	runtimeDirectory = GetRuntimeDirectory();
	if (!runtimeDirectory.empty())
	{
		config.clear();

		std::string configPath = runtimeDirectory + "Data\\SKSE\\Plugins\\";

		auto configList = get_all_files_names_within_folder(configPath.c_str());
		bool configOpened = false;
		for (std::size_t i = 0; i < configList.size(); i++)
		{
			std::string filename = configList.at(i);

			if (filename == "." || filename == "..")
				continue;
					
			if (stringStartsWith(filename, "cbpconfig"))
			{
				std::string msg = "File found: " + filename;
				LOG(msg.c_str());

				std::string filepath = configPath;
				filepath.append(filename);
				FILE *fh = fopen(filepath.c_str(), "r");
				if (!fh) {
					logger.error("Failed to open config file %s\n", filename);
					//Console_Print("Failed to open config file CBPConfig.txt");
					continue;
				}

				configOpened = true;
				do {
					auto str = fgets(buffer, 1023, fh);
					//logger.error("str %s\n", str);
					if (str && strlen(str) > 1) {
						if (str[0] != '#') {
							char *tok0 = strtok(str, ".");
							char *tok1 = strtok(NULL, " ");
							char *tok2 = strtok(NULL, " ");

							if (tok0 && tok1 && tok2) {
								config[std::string(tok0)][std::string(tok1)] = atof(tok2);
								LOG("config[%s][%s] = %s", tok0, tok1, tok2);
							}
						}
					}
				} while (!feof(fh));
				fclose(fh);
			}
		}

		if (!configOpened)
			configReloadCount = 0;

		configReloadCount = config["Tuning"]["rate"];
	}
}

void loadSystemConfig()
{
	dialogueMenuOpen.store(false);
	raceSexMenuClosed.store(false);
	
	MenuManager * menuManager = MenuManager::GetSingleton();
	if (menuManager)
		menuManager->MenuOpenCloseEventDispatcher()->AddEventSink(&menuEvent);
	
	std::string	runtimeDirectory = GetRuntimeDirectory();

	if (!runtimeDirectory.empty())
	{
		std::string filepath = runtimeDirectory + "Data\\SKSE\\Plugins\\CBPCSystem.ini";

		std::ifstream file(filepath);

		if (!file.is_open())
		{
			transform(filepath.begin(), filepath.end(), filepath.begin(), ::tolower);
			file.open(filepath);
		}

		if (file.is_open())
		{
			std::string line;
			std::string currentSetting;
			while (std::getline(file, line))
			{
				//trim(line);
				skipComments(line);
				trim(line);
				if (line.length() > 0)
				{
					if (line.substr(0, 1) == "[")
					{
						//newsetting
						currentSetting = line;
					}
					else
					{
						std::string variableName;
						
						int variableValue = GetConfigSettingsValue(line, variableName);
						if (variableName == "UseParallelProcessing")
							useParallelProcessing = variableValue;
						else if (variableName == "SkipFrames")
							collisionSkipFrames = variableValue;
						else if (variableName == "SkipFramesPelvis")
							collisionSkipFramesPelvis = variableValue;
						else if (variableName == "GridSize")
							gridsize = variableValue;
						else if (variableName == "AdjacencyValue")
							adjacencyValue = variableValue;
						else if (variableName == "ActorDistance")
						{
							float variableFloatValue = GetConfigSettingsFloatValue(line, variableName);
							actorDistance = variableFloatValue*variableFloatValue;
						}
						else if (variableName == "Logging")
						{
							logging = variableValue;
						}
#ifdef RUNTIME_VR_VERSION_1_4_15	
						else if (variableName == "HapticFrequency")
							hapticFrequency = variableValue;
						else if (variableName == "HapticStrength")
							hapticStrength = variableValue;
#endif
					}
				}
			}

			LOG("System Config file is loaded successfully.");
			return;
		}
	}

	LOG("System Config file is not loaded.");
}

void loadMasterConfig()
{
	std::string	runtimeDirectory = GetRuntimeDirectory();

	if (!runtimeDirectory.empty())
	{
		configMap.clear();
		affectedBones.clear();
		std::string filepath = runtimeDirectory + "Data\\SKSE\\Plugins\\CBPCMasterConfig.txt";

		std::ifstream file(filepath);

		if (!file.is_open())
		{
			transform(filepath.begin(), filepath.end(), filepath.begin(), ::tolower);
			file.open(filepath);
		}

		if (file.is_open())
		{
			std::string line;
			std::string currentSetting;
			while (std::getline(file, line))
			{
				//trim(line);
				skipComments(line);
				trim(line);
				if (line.length() > 0)
				{
					if (line.substr(0, 1) == "[")
					{
						//newsetting
						currentSetting = line;
					}
					else
					{
						if (currentSetting == "[Settings]")
						{
							std::string variableName;
							std::string variableStrValue = GetConfigSettingsStringValue(line, variableName);
							if (variableName == "ExtraRaces")
							{
								extraRacesList = split(variableStrValue, ',');
							}
							else
							{
								int variableValue = GetConfigSettingsValue(line, variableName);
								if (variableName == "TuningMode")
									tuningModeCollision = variableValue;								
								else if (variableName == "MalePhysics")
									malePhysics = variableValue;
								else if (variableName == "MalePhysicsOnlyForExtraRaces")
									malePhysicsOnlyForExtraRaces = variableValue;								
							}
						}
						else if (currentSetting == "[ConfigMap]")
						{
							std::string variableName;
							std::string variableValue = GetConfigSettingsStringValue(line, variableName);
							affectedBones.emplace_back(variableName);

							if (variableValue != "")
							{
								configMap[variableName] = variableValue;
								LOG("ConfigMap[%s] = %s", variableName, variableValue);
							}
						}
					}
				}
			}

			LOG("Master Config file is loaded successfully.");
			return;
		}
	}

	LOG("Master Config file is not loaded.");
}

void loadCollisionConfig()
{
	std::string	runtimeDirectory = GetRuntimeDirectory();

	if (!runtimeDirectory.empty())
	{
		#ifdef RUNTIME_VR_VERSION_1_4_15
		PlayerNodeLines.clear();
		PlayerNodesList.clear();
		#endif
		AffectedNodeLines.clear();
		ColliderNodeLines.clear();
		AffectedNodesList.clear();
		ColliderNodesList.clear();
		
		std::string filepath = runtimeDirectory + "Data\\SKSE\\Plugins\\CBPCollisionConfig.txt";
				
		std::ifstream file(filepath);

		if (!file.is_open())
		{
			transform(filepath.begin(), filepath.end(), filepath.begin(), ::tolower);
			file.open(filepath);
		}

		if (file.is_open())
		{
			std::string line;
			std::string currentSetting;
			while (std::getline(file, line))
			{
				//trim(line);
				skipComments(line);
				trim(line);
				if (line.length() > 0)
				{
					if (line.substr(0, 1) == "[")
					{
						//newsetting
						currentSetting = line;
					}
					else
					{
						if (currentSetting == "[ExtraOptions]")
						{
							std::string variableName;
							float variableValue = GetConfigSettingsFloatValue(line, variableName);
							if (variableName == "BellyBulge")
							{
								cbellybulge = variableValue;
							}
							else if (variableName == "BellyBulgeMax")
							{
								cbellybulgemax = variableValue;
							}
							else if (variableName == "BellyBulgePosLowest")
							{
								cbellybulgeposlowest = variableValue;
							}
							else if (variableName == "BellyBulgeNodes")
							{
								std::string variableStrValue = GetConfigSettingsStringValue(line, variableName);
								bellybulgenodesList = split(variableStrValue, ',');
							}
							else if (variableName == "VaginaOpeningLimit")
							{
								vaginaOpeningLimit = variableValue;
							}
							else if (variableName == "VaginaOpeningMultiplier")
							{
								vaginaOpeningMultiplier = variableValue;
							}
							else if (variableName == "BellyBulgeReturnTime")
							{
								bellyBulgeReturnTime = variableValue;
							}							
						}
						else if (currentSetting == "[AffectedNodes]")
						{
							AffectedNodeLines.emplace_back(line);
							ConfigLine newConfigLine;
							newConfigLine.NodeName = line;
							AffectedNodesList.emplace_back(newConfigLine);
						}
						else if (currentSetting == "[ColliderNodes]")
						{
							ColliderNodeLines.emplace_back(line);
							ConfigLine newConfigLine;
							newConfigLine.NodeName = line;
							ColliderNodesList.emplace_back(newConfigLine);
						}
						#ifdef RUNTIME_VR_VERSION_1_4_15
						else if (currentSetting == "[PlayerNodes]")
						{
							PlayerNodeLines.emplace_back(line);
							ConfigLine newConfigLine;
							newConfigLine.NodeName = line;
							PlayerNodesList.emplace_back(newConfigLine);
						}
						#endif
						else
						{
							Sphere newSphere;
							ConfigLineSplitter(line, newSphere);

							std::string trimmedSetting = gettrimmed(currentSetting);
							#ifdef RUNTIME_VR_VERSION_1_4_15
							if (std::find(PlayerNodeLines.begin(), PlayerNodeLines.end(), trimmedSetting) != PlayerNodeLines.end())
							{
								for (int i = 0; i < PlayerNodesList.size(); i++)
								{
									if (PlayerNodesList[i].NodeName == trimmedSetting)
									{
										newSphere.NodeName = PlayerNodesList[i].NodeName;
										PlayerNodesList[i].CollisionSpheres.emplace_back(newSphere);
										break;
									}
								}
							}
							#endif
							if (std::find(AffectedNodeLines.begin(), AffectedNodeLines.end(), trimmedSetting) != AffectedNodeLines.end())
							{
								for (int i = 0; i < AffectedNodesList.size(); i++)
								{
									if (AffectedNodesList[i].NodeName == trimmedSetting)
									{
										newSphere.NodeName = AffectedNodesList[i].NodeName;
										AffectedNodesList[i].CollisionSpheres.emplace_back(newSphere);
										break;
									}
								}
							}
							if (std::find(ColliderNodeLines.begin(), ColliderNodeLines.end(), trimmedSetting) != ColliderNodeLines.end())
							{
								for (int i = 0; i < ColliderNodesList.size(); i++)
								{
									if (ColliderNodesList[i].NodeName == trimmedSetting)
									{
										newSphere.NodeName = ColliderNodesList[i].NodeName;
										ColliderNodesList[i].CollisionSpheres.emplace_back(newSphere);
										break;
									}
								}
							}
						}
					}					
				}
			}
		}
		LOG("Collision Config file is loaded successfully.");
		return;
	}

	LOG("Collision Config file is not loaded.");
	return;
}

#ifdef RUNTIME_VR_VERSION_1_4_15
void GetSettings()
{	
	Setting* setting = GetINISetting("fMeleeWeaponTranslateX:VR");
	if (!setting || setting->GetType() != Setting::kType_Float)
		LOG("Failed to get fMeleeWeaponTranslateX from INI.");
	else
		MeleeWeaponTranslateX = setting->data.f32;

	setting = GetINISetting("fMeleeWeaponTranslateY:VR");
	if (!setting || setting->GetType() != Setting::kType_Float)
		LOG("Failed to get fMeleeWeaponTranslateY from INI.");
	else
		MeleeWeaponTranslateY = setting->data.f32;

	setting = GetINISetting("fMeleeWeaponTranslateZ:VR");
	if (!setting || setting->GetType() != Setting::kType_Float)
		LOG("Failed to get fMeleeWeaponTranslateZ from INI.");
	else
		MeleeWeaponTranslateZ = setting->data.f32;

	LOG("Melee Weapon Translate Values From ini: %g %g %g", MeleeWeaponTranslateX, MeleeWeaponTranslateY, MeleeWeaponTranslateZ);
}
#endif

void loadExtraCollisionConfig()
{
	std::string	runtimeDirectory = GetRuntimeDirectory();
	
	if (!runtimeDirectory.empty())
	{
		specificNPCConfigList.clear();

		std::string configPath = runtimeDirectory + "Data\\SKSE\\Plugins\\";
		
		auto configList = get_all_files_names_within_folder(configPath.c_str());
				
		for (int i = 0; i < configList.size(); i++)
		{
			std::string filename = configList.at(i);

			if (filename == "." || filename == "..")
				continue;

			if (stringStartsWith(filename, "cbpcollisionconfig") && filename != "CBPCollisionConfig.txt")
			{
				std::string msg = "File found: " + filename;
				LOG(msg.c_str());

				std::string filepath = configPath;
				filepath.append(filename);

				SpecificNPCConfig newNPCConfig;

				std::ifstream file(filepath);

				if (file.is_open())
				{
					std::string line;
					std::string currentSetting;
					while (std::getline(file, line))
					{
						//trim(line);
						skipComments(line);
						trim(line);
						if (line.length() > 0)
						{
							if (line.substr(0, 1) == "[")
							{
								//newsetting
								currentSetting = line;
							}
							else
							{
								if (currentSetting == "[Options]")
								{
									std::string variableName;
									std::string variableValue = GetConfigSettingsStringValue(line, variableName);
									if (variableName == "Characters")
									{
										newNPCConfig.charactersList = split(variableValue, ',');
									}
									else if(variableName == "Races")
									{
										newNPCConfig.raceList = split(variableValue, ',');
									}
								}
								else if (currentSetting == "[ExtraOptions]")
								{
									std::string variableName;
									float variableValue = GetConfigSettingsFloatValue(line, variableName);
									if (variableName == "BellyBulge")
									{
										newNPCConfig.cbellybulge = variableValue;
									}
									else if (variableName == "BellyBulgeMax")
									{
										newNPCConfig.cbellybulgemax = variableValue;
									}
									else if (variableName == "BellyBulgePosLowest")
									{
										newNPCConfig.cbellybulgeposlowest = variableValue;
									}
									else if (variableName == "BellyBulgeNodes")
									{
										std::string variableStrValue = GetConfigSettingsStringValue(line, variableName);
										newNPCConfig.bellybulgenodesList = split(variableStrValue, ',');
									}
								}
								else if (currentSetting == "[AffectedNodes]")
								{
									newNPCConfig.AffectedNodeLines.emplace_back(line);
									ConfigLine newConfigLine;
									newConfigLine.NodeName = line;
									newNPCConfig.AffectedNodesList.emplace_back(newConfigLine);
								}
								else if (currentSetting == "[ColliderNodes]")
								{
									newNPCConfig.ColliderNodeLines.emplace_back(line);
									ConfigLine newConfigLine;
									newConfigLine.NodeName = line;
									newNPCConfig.ColliderNodesList.emplace_back(newConfigLine);
								}
								else
								{
									Sphere newSphere;
									ConfigLineSplitter(line, newSphere);

									std::string trimmedSetting = gettrimmed(currentSetting);

									if (std::find(newNPCConfig.AffectedNodeLines.begin(), newNPCConfig.AffectedNodeLines.end(), trimmedSetting) != newNPCConfig.AffectedNodeLines.end())
									{
										for (int i = 0; i < newNPCConfig.AffectedNodesList.size(); i++)
										{
											if (newNPCConfig.AffectedNodesList[i].NodeName == trimmedSetting)
											{
												newNPCConfig.AffectedNodesList[i].CollisionSpheres.emplace_back(newSphere);
												break;
											}
										}
									}
									if (std::find(newNPCConfig.ColliderNodeLines.begin(), newNPCConfig.ColliderNodeLines.end(), trimmedSetting) != newNPCConfig.ColliderNodeLines.end())
									{
										for (int i = 0; i < newNPCConfig.ColliderNodesList.size(); i++)
										{
											if (newNPCConfig.ColliderNodesList[i].NodeName == trimmedSetting)
											{
												newNPCConfig.ColliderNodesList[i].CollisionSpheres.emplace_back(newSphere);
												break;
											}
										}
									}
								}
							}
						}
					}
					specificNPCConfigList.emplace_back(newNPCConfig);
				}
			}
		}

		LOG("Specific collision config files(if any) are loaded successfully.");
		return;
	}

	LOG("Specific collision config files are not loaded.");
	return;
}

#ifdef RUNTIME_VR_VERSION_1_4_15
void LoadWeaponCollisionConfig()
{
	WeaponCollidersList.clear();
	std::string	runtimeDirectory = GetRuntimeDirectory();

	if (!runtimeDirectory.empty())
	{
		std::string configPath = runtimeDirectory + "Data\\SKSE\\Plugins\\CBPWeaponCollisionConfig.txt";

		std::ifstream file(configPath);
		std::string line;
		std::string currentSetting;
		while (std::getline(file, line))
		{
			trim(line);
			skipComments(line);
			trim(line);
			if (line.length() > 0)
			{
				if (line.substr(0, 1) == "[")
				{
					//newsetting
					currentSetting = line;					
				}
				else
				{					
					Triangle newTriangle;
					ConfigWeaponLineSplitter(line, newTriangle);

					WeaponConfigLine newLine;
					newLine.WeaponName = gettrimmed(currentSetting);					
					newLine.CollisionTriangle = newTriangle;	

					WeaponCollidersList.emplace_back(newLine);
				}
			}
		}

		LOG("Weapon Config file is loaded successfully.");
		return;
	}

	LOG("Weapon Config file is not loaded.");
	return;
}

void ConfigWeaponLineSplitter(std::string &line, Triangle &newTriangle)
{
	std::vector<std::string> pointsSplitted = split(line, '|');

	if (pointsSplitted.size() >= 3)
	{
		std::vector<std::string> splittedFloats = split(pointsSplitted[0], ',');
		if (splittedFloats.size() > 0)
		{
			newTriangle.a.x = (strtof(splittedFloats[0].c_str(), 0) + MeleeWeaponTranslateX);
		}
		if (splittedFloats.size() > 1)
		{
			newTriangle.a.y = (strtof(splittedFloats[1].c_str(), 0) + MeleeWeaponTranslateY);
		}
		if (splittedFloats.size() > 2)
		{
			newTriangle.a.z = (strtof(splittedFloats[2].c_str(), 0) + MeleeWeaponTranslateZ);
		}

		newTriangle.orga = newTriangle.a;

		splittedFloats = split(pointsSplitted[1], ',');
		if (splittedFloats.size() > 0)
		{
			newTriangle.b.x = (strtof(splittedFloats[0].c_str(), 0) + MeleeWeaponTranslateX);
		}
		if (splittedFloats.size() > 1)
		{
			newTriangle.b.y = (strtof(splittedFloats[1].c_str(), 0) + MeleeWeaponTranslateY);
		}
		if (splittedFloats.size() > 2)
		{
			newTriangle.b.z = (strtof(splittedFloats[2].c_str(), 0) + MeleeWeaponTranslateZ);
		}

		newTriangle.orgb = newTriangle.b;

		splittedFloats = split(pointsSplitted[2], ',');
		if (splittedFloats.size() > 0)
		{
			newTriangle.c.x = (strtof(splittedFloats[0].c_str(), 0) + MeleeWeaponTranslateX);
		}
		if (splittedFloats.size() > 1)
		{
			newTriangle.c.y = (strtof(splittedFloats[1].c_str(), 0) + MeleeWeaponTranslateY);
		}
		if (splittedFloats.size() > 2)
		{
			newTriangle.c.z = (strtof(splittedFloats[2].c_str(), 0) + MeleeWeaponTranslateZ);
		}
		newTriangle.orgc = newTriangle.c;
	}
}
#endif

void ConfigLineSplitter(std::string &line, Sphere &newSphere)
{
	std::vector<std::string> lowHighSplitted = split(line, '|');
	if (lowHighSplitted.size() == 1)
	{
		std::vector<std::string> splittedFloats = split(lowHighSplitted[0], ',');
		if (splittedFloats.size()>0)
		{
			newSphere.offset0.x = strtof(splittedFloats[0].c_str(), 0);
			newSphere.offset100.x = newSphere.offset0.x;
		}
		if (splittedFloats.size()>1)
		{
			newSphere.offset0.y = strtof(splittedFloats[1].c_str(), 0);
			newSphere.offset100.y = newSphere.offset0.y;
		}
		if (splittedFloats.size()>2)
		{
			newSphere.offset0.z = strtof(splittedFloats[2].c_str(), 0);
			newSphere.offset100.z = newSphere.offset0.z;
		}
		if (splittedFloats.size()>3)
		{
			newSphere.radius0 = strtof(splittedFloats[3].c_str(), 0);
			newSphere.radius100 = newSphere.radius0;
		}
	}
	else if (lowHighSplitted.size() > 1)
	{
		std::vector<std::string> splittedFloats = split(lowHighSplitted[0], ',');
		if (splittedFloats.size()>0)
		{
			newSphere.offset0.x = strtof(splittedFloats[0].c_str(), 0);
		}
		if (splittedFloats.size()>1)
		{
			newSphere.offset0.y = strtof(splittedFloats[1].c_str(), 0);
		}
		if (splittedFloats.size()>2)
		{
			newSphere.offset0.z = strtof(splittedFloats[2].c_str(), 0);
		}
		if (splittedFloats.size()>3)
		{
			newSphere.radius0 = strtof(splittedFloats[3].c_str(), 0);
		}

		splittedFloats = split(lowHighSplitted[1], ',');
		if (splittedFloats.size()>0)
		{
			newSphere.offset100.x = strtof(splittedFloats[0].c_str(), 0);
		}
		if (splittedFloats.size()>1)
		{
			newSphere.offset100.y = strtof(splittedFloats[1].c_str(), 0);
		}
		if (splittedFloats.size()>2)
		{
			newSphere.offset100.z = strtof(splittedFloats[2].c_str(), 0);
		}
		if (splittedFloats.size()>3)
		{
			newSphere.radius100 = strtof(splittedFloats[3].c_str(), 0);
		}
	}
}

int GetConfigSettingsValue(std::string line, std::string &variable)
{
	int value=0;
	std::vector<std::string> splittedLine = split(line, '=');
	variable = "";
	if (splittedLine.size() > 1)
	{
		variable = splittedLine[0];
		trim(variable);

		std::string valuestr = splittedLine[1];
		trim(valuestr);
		value = std::stoi(valuestr);
	}

	return value;
}

float GetConfigSettingsFloatValue(std::string line, std::string &variable)
{
	float value = 0;
	std::vector<std::string> splittedLine = split(line, '=');
	variable = "";
	if (splittedLine.size() > 1)
	{
		variable = splittedLine[0];
		trim(variable);

		std::string valuestr = splittedLine[1];
		trim(valuestr);
		value = strtof(valuestr.c_str(), 0);
	}

	return value;
}

std::string GetConfigSettingsStringValue(std::string line, std::string &variable)
{
	std::string valuestr = "";
	std::vector<std::string> splittedLine = split(line, '=');
	variable = "";
	if (splittedLine.size() > 0)
	{
		variable = splittedLine[0];
		trim(variable);
	}

	if (splittedLine.size() > 1)
	{
		valuestr = splittedLine[1];
		trim(valuestr);
	}

	return valuestr;
}

void printSpheresMessage(std::string message, std::vector<Sphere> spheres)
{
	for (int i = 0; i < spheres.size(); i++)
	{
		message += " Spheres: ";
		message += std::to_string(spheres[i].offset0.x);
		message += ",";
		message += std::to_string(spheres[i].offset0.y);
		message += ",";
		message += std::to_string(spheres[i].offset0.z);
		message += ",";
		message += std::to_string(spheres[i].radius0);
		message += " | ";
		message += std::to_string(spheres[i].offset100.x);
		message += ",";
		message += std::to_string(spheres[i].offset100.y);
		message += ",";
		message += std::to_string(spheres[i].offset100.z);
		message += ",";
		message += std::to_string(spheres[i].radius100);
	}
	LOG(message.c_str());
}

std::vector<std::string> ConfigLineVectorToStringVector(std::vector<ConfigLine> linesList)
{
	std::vector<std::string> outVector;

	for (int i = 0; i < linesList.size(); i++)
	{
		std::string str = linesList[i].NodeName;
		trim(str);
		outVector.emplace_back(str);
	}
	return outVector;
}

bool GetSpecificNPCConfigForActor(std::string actorRefName, std::string actorRace, SpecificNPCConfig &snc)
{
	for (int i = 0; i < specificNPCConfigList.size(); i++)
	{
		if (actorRefName.size() != 0)
		{
			if (std::find(specificNPCConfigList[i].charactersList.begin(), specificNPCConfigList[i].charactersList.end(), actorRefName) != specificNPCConfigList[i].charactersList.end())
			{
				snc = specificNPCConfigList[i];
				return true;
			}
		}
		if (actorRace.size() != 0)
		{
			if (std::find(specificNPCConfigList[i].raceList.begin(), specificNPCConfigList[i].raceList.end(), actorRace) != specificNPCConfigList[i].raceList.end())
			{
				snc = specificNPCConfigList[i];
				return true;
			}
		}
	}

	return false;
}

bool IsActorMale(Actor* actor)
{
	TESNPC * actorNPC = DYNAMIC_CAST(actor->baseForm, TESForm, TESNPC);

	auto npcSex = actorNPC ? CALL_MEMBER_FN(actorNPC, GetSex)() : 1;

	if (npcSex == 0) //Actor is male
		return true;
	else
		return false;
}

#ifdef RUNTIME_VR_VERSION_1_4_15
std::vector<Triangle> GetCollisionTriangles(std::string name, UInt8 kType)
{
	std::vector<Triangle> resultList;
	for (int i = 0; i < WeaponCollidersList.size(); i++)
	{
		if (WeaponCollidersList[i].WeaponName == name)
		{
			resultList.emplace_back(WeaponCollidersList[i].CollisionTriangle);
		}
	}
	if (resultList.size() == 0 && kType > 0)
	{
		std::string typeName = GetWeaponTypeName(kType);
		if (typeName != "")
		{
			for (int i = 0; i < WeaponCollidersList.size(); i++)
			{
				if (WeaponCollidersList[i].WeaponName == typeName)
				{
					resultList.emplace_back(WeaponCollidersList[i].CollisionTriangle);
				}
			}
		}
	}
	return resultList;
}

std::string GetWeaponTypeName(UInt8 kType)
{
	if (kType == TESObjectWEAP::GameData::kType_TwoHandSword || kType == TESObjectWEAP::GameData::kType_2HS)
		return "Type_TwoHandSword";
	else if (kType == TESObjectWEAP::GameData::kType_TwoHandAxe || kType == TESObjectWEAP::GameData::kType_2HA)
		return "Type_TwoHandAxe";
	else if (kType == TESObjectWEAP::GameData::kType_OneHandSword || kType == TESObjectWEAP::GameData::kType_1HS)
		return "Type_OneHandSword";
	else if (kType == TESObjectWEAP::GameData::kType_OneHandAxe || kType == TESObjectWEAP::GameData::kType_1HA)
		return "Type_OneHandAxe";
	else if (kType == TESObjectWEAP::GameData::kType_OneHandDagger || kType == TESObjectWEAP::GameData::kType_1HD)
		return "Type_OneHandDagger";
	else if (kType == TESObjectWEAP::GameData::kType_OneHandMace || kType == TESObjectWEAP::GameData::kType_1HM)
		return "Type_OneHandMace";
	else if (kType == TESObjectWEAP::GameData::kType_CrossBow || kType == TESObjectWEAP::GameData::kType_CBow)
		return "Type_CrossBow";
	else if (kType == TESObjectWEAP::GameData::kType_Bow || kType == TESObjectWEAP::GameData::kType_Bow2)
		return "Type_Bow";
	else if (kType == TESObjectWEAP::GameData::kType_Staff || kType == TESObjectWEAP::GameData::kType_Staff2)
		return "Type_Staff";
	else
		return "";
}

void LeftHandedModeChange()
{
	const int value = vlibGetSetting("bLeftHandedMode:VRInput");
	if (value != leftHandedMode)
	{
		leftHandedMode = value;
		LOG_ERR("Left Handed Mode is %s.", leftHandedMode ? "ON" : "OFF");
	}
}
#endif

//Menu Stuff

AllMenuEventHandler menuEvent;

EventResult AllMenuEventHandler::ReceiveEvent(MenuOpenCloseEvent * evn, EventDispatcher<MenuOpenCloseEvent> * dispatcher)
{
	if (evn->opening)
	{
		MenuOpened(evn->menuName.c_str());
	}
	else
	{
		MenuClosed(evn->menuName.c_str());
	}

	return EventResult::kEvent_Continue;
}

void MenuOpened(std::string name)
{
	if (name == "Dialogue Menu")
	{
		dialogueMenuOpen.store(true);
	}
}

void MenuClosed(std::string name)
{
	if (name == "Dialogue Menu")
	{
		dialogueMenuOpen.store(false);
	}
	else if (name == "RaceSex Menu")
	{
		raceSexMenuClosed.store(true);
	}
#ifdef RUNTIME_VR_VERSION_1_4_15
	else if(name == "Console" || name == "Journal Menu")
	{
		LeftHandedModeChange();
	}
#endif
}

BSFixedString GetVersion(StaticFunctionTag* base)
{
	return BSFixedString("1");
}

BSFixedString GetVersionMinor(StaticFunctionTag* base)
{
	return BSFixedString("3");
}

BSFixedString GetVersionBeta(StaticFunctionTag* base)
{
	return BSFixedString("0");
}

//Initializes openvr system. Required for haptic triggers.


bool RegisterFuncs(VMClassRegistry* registry)
{
#ifdef RUNTIME_VR_VERSION_1_4_15
	LeftHandedModeChange();
#endif

	registry->RegisterFunction(
		new NativeFunction0<StaticFunctionTag, BSFixedString>("GetVersion", "CBPCPluginScript", GetVersion, registry));

	registry->RegisterFunction(
		new NativeFunction0<StaticFunctionTag, BSFixedString>("GetVersionMinor", "CBPCPluginScript", GetVersionMinor, registry));

	registry->RegisterFunction(
		new NativeFunction0<StaticFunctionTag, BSFixedString>("GetVersionBeta", "CBPCPluginScript", GetVersionBeta, registry));

	LOG("CBPC registerFunction");
	return true;
}



void Log(const int msgLogLevel, const char * fmt, ...)
{
	if (msgLogLevel > logging)
	{
		return;
	}

	va_list args;
	char logBuffer[4096];

	va_start(args, fmt);
	vsprintf_s(logBuffer, sizeof(logBuffer), fmt, args);
	va_end(args);

	_MESSAGE(logBuffer);
}