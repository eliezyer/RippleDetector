#include <PluginInfo.h>
#include "RippleDetector2.h"
#include <string>
#ifdef WIN32
#include <Windows.h>
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

using namespace Plugin;
#define NUM_PLUGINS 1

extern "C" EXPORT void getLibInfo(Plugin::LibraryInfo* info)
{
	info->apiVersion = PLUGIN_API_VER;
	info->name = "Example library";
	info->libVersion = 1;
	info->numPlugins = NUM_PLUGINS;
}

extern "C" EXPORT int getPluginInfo(int index, Plugin::PluginInfo* info)
{
	switch (index)
	{
	case 0:
//		info->type = Plugin::PLUGIN_TYPE_PROCESSOR;
    info->type = Plugin::ProcessorPlugin;
		info->processor.name = "2Ripple Detector"; //Processor name shown in the GUI
		info->processor.type = Plugin::FilterProcessor;
		info->processor.creator = &(Plugin::createProcessor<RippleDetector2>);
		break;
		
	default:
		return -1;
		break;
	}
	return 0;
}

#ifdef WIN32
BOOL WINAPI DllMain(IN HINSTANCE hDllHandle,
	IN DWORD     nReason,
	IN LPVOID    Reserved)
{
	return TRUE;
}

#endif
