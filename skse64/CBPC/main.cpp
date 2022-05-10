#include "common/ITypes.h"
#include <string>
#include "skse64/PluginAPI.h"
#include "skse64/PluginManager.h"
#include "skse64_common/skse_version.h"
#include "skse64_common/SafeWrite.h"
#include "skse64/GameAPI.h"
#include "skse64/GameEvents.h"

#include "config.h"
#include <shlobj.h>				// CSIDL_MYCODUMENTS



PluginHandle	g_pluginHandle = kPluginHandle_Invalid;

SKSEMessagingInterface	* g_messaging = NULL;
static SKSEPapyrusInterface         * g_papyrus = NULL;
//SKSEScaleformInterface		* g_scaleform = NULL;
//SKSESerializationInterface	* g_serialization = NULL;
SKSETaskInterface				* g_task = nullptr;
//IDebugLog	gLog("Data\\SKSE\\Plugins\\hook.log");

void DoHook();



//void MessageHandler(SKSEMessagingInterface::Message * msg)
//{
//	switch (msg->type)
//	{
//		case SKSEMessagingInterface::kMessage_DataLoaded:
//		{
//			logger.info("kMessage_DataLoaded\n");
//		}
//		break;
//		case SKSEMessagingInterface::kMessage_NewGame:
//		{
//			logger.info("kMessage_NewGame\n");
//		}
//		break;
//		case SKSEMessagingInterface::kMessage_PreLoadGame:
//		{
//			logger.info("kMessage_PreLoadGame\n");
//		}
//		break;
//		case SKSEMessagingInterface::kMessage_PostLoad:
//		{
//			logger.info("kMessage_PostLoad\n");
//		}
//		break;
//		case SKSEMessagingInterface::kMessage_PostPostLoad:
//		{
//			logger.info("kMessage_PostPostLoad\n");
//		}
//		break;
//		case SKSEMessagingInterface::kMessage_PostLoadGame:
//		{
//			logger.info("kMessage_PostLoadGame\n");
//		}
//		break;
//		case SKSEMessagingInterface::kMessage_SaveGame:
//		{
//			logger.info("kMessage_SaveGame\n");
//		}
//		break;
//		case SKSEMessagingInterface::kMessage_DeleteGame:
//		{
//			logger.info("kMessage_DeleteGame\n");
//		}
//		break;
//		case SKSEMessagingInterface::kMessage_InputLoaded:
//		{
//			logger.info("kMessage_InputLoaded\n");
//		}
//		break;
//
//	}
//}



extern "C"
{

	bool SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info)
	{
		#ifdef RUNTIME_VR_VERSION_1_4_15
		gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim VR\\SKSE\\CBPC-Collision.log");
		#else
		gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim Special Edition\\SKSE\\CBPC-Collision.log");
		#endif
		gLog.SetPrintLevel(IDebugLog::kLevel_Error);
		gLog.SetLogLevel(IDebugLog::kLevel_DebugMessage);

		logger.info("CBPC Physics SKSE Plugin\n");
		logger.error("Query called\n");

		// populate info structure
		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "CBPC plugin";
		info->version = 010300; // 1.3.0

		// store plugin handle so we can identify ourselves later
		g_pluginHandle = skse->GetPluginHandle();

		if (skse->isEditor)
		{
			logger.error("loaded in editor, marking as incompatible\n");
			return false;
		}
		else if (skse->runtimeVersion != CURRENT_RELEASE_RUNTIME)
		{
			logger.error("unsupported runtime version %08X", skse->runtimeVersion);
			return false;
		}
		// supported runtime version

		logger.error("Query complete\n");
		return true;
	}

	void OnSKSEMessage(SKSEMessagingInterface::Message * msg)
	{
		switch (msg->type)
		{
			case SKSEMessagingInterface::kMessage_DataLoaded:
			{
				#ifdef RUNTIME_VR_VERSION_1_4_15
				GetSettings();
				#endif
				loadMasterConfig();
				loadSystemConfig();

				loadCollisionConfig();
				
				loadExtraCollisionConfig();
				#ifdef RUNTIME_VR_VERSION_1_4_15
				LoadWeaponCollisionConfig();
				#endif
			}
			break;
		}
	}
	
	bool SKSEPlugin_Load(const SKSEInterface * skse)
	{
		logger.error("CBPC Loading\n");

		g_task = (SKSETaskInterface *)skse->QueryInterface(kInterface_Task);
		if (!g_task)
		{
			logger.error("Couldn't get Task interface\n");
			return false;
		}

		// Load initial config before the hook.
		logger.error("Loading Config\n");
		loadConfig();
				
		g_papyrus = (SKSEPapyrusInterface *)skse->QueryInterface(kInterface_Papyrus);
		
		g_messaging = (SKSEMessagingInterface*)skse->QueryInterface(kInterface_Messaging);
		g_messaging->RegisterListener(g_pluginHandle, "SKSE", OnSKSEMessage);

		bool bSuccess = g_papyrus->Register(RegisterFuncs);

		if (bSuccess) {
			LOG("Register Succeeded");
		}

		logger.error("Hooking Game\n");
		DoHook();
		logger.error("CBPC Load Complete\n");
		

		return true;
	}



};

BOOL WINAPI DllMain(
	_In_ HINSTANCE hinstDLL,
	_In_ DWORD     fdwReason,
	_In_ LPVOID    lpvReserved
) {
	return true;
}