// pch.cpp: source file corresponding to the pre-compiled header

#include "pch.h"
#include <Windows.h>
typedef unsigned char byte;
#include "Razer\ChromaAnimationAPI.h"
#include <mutex>

using namespace ChromaSDK;
using namespace std;

// When you are using pre-compiled headers, this source file is necessary for compilation to succeed.

static bool _sInitializedAPI = false;
static bool _sChromaInitialized = false;
static bool _sWaitForExit = true;
static mutex _sMutex;
static std::thread* _sThreadChroma = nullptr;

// This final animation will have a single frame
// Any color changes will immediately display in the next frame update.
const char* ANIMATION_FINAL_CHROMA_LINK = "Dynamic\\Final_ChromaLink.chroma";
const char* ANIMATION_FINAL_HEADSET = "Dynamic\\Final_Headset.chroma";
const char* ANIMATION_FINAL_KEYBOARD = "Dynamic\\Final_Keyboard.chroma";
const char* ANIMATION_FINAL_KEYPAD = "Dynamic\\Final_Keypad.chroma";
const char* ANIMATION_FINAL_MOUSE = "Dynamic\\Final_Mouse.chroma";
const char* ANIMATION_FINAL_MOUSEPAD = "Dynamic\\Final_Mousepad.chroma";

extern "C"
{
	__declspec(dllexport) int ApplicationStart()
	{
		lock_guard<mutex> guard(_sMutex); //make sure we aren't doing things while animations are playing

		if (!_sInitializedAPI)
		{
			if (ChromaAnimationAPI::InitAPI() != 0)
			{
				return -1;
			}
			_sInitializedAPI = true;
		}

		// start the main thread worker
		_sThreadChroma = new std::thread(&WorkerChroma);
		_sThreadChroma->detach();

		return 0;
	}

	__declspec(dllexport) int ApplicationQuit()
	{
		_sWaitForExit = false;
		if (_sThreadChroma != nullptr)
		{
			_sThreadChroma->join();
		}

		lock_guard<mutex> guard(_sMutex); //make sure we aren't doing things while animations are playing

		if (_sInitializedAPI)
		{
			if (ChromaAnimationAPI::IsInitialized())
			{
				ChromaAnimationAPI::StopAll();
				ChromaAnimationAPI::CloseAll();
				return ChromaAnimationAPI::Uninit();
			}
		}

		return 0;
	}

	__declspec(dllexport) long PlayerChromaInit()
	{
		lock_guard<mutex> guard(_sMutex); //make sure we aren't doing things while animations are playing

		if (!_sInitializedAPI)
		{
			if (ChromaAnimationAPI::InitAPI() != 0)
			{
				return -1;
			}
			_sInitializedAPI = true;
		}
		if (_sChromaInitialized)
		{
			return 0;
		}
		RZRESULT result = ChromaAnimationAPI::Init();
		if (result == 0)
		{
			_sChromaInitialized = true;
		}
		return result;
	}

	__declspec(dllexport) long PlayerChromaUninit()
	{
		lock_guard<mutex> guard(_sMutex); //make sure we aren't doing things while animations are playing

		if (_sInitializedAPI)
		{
			if (_sChromaInitialized)
			{
				ChromaAnimationAPI::StopAll();
				ChromaAnimationAPI::CloseAll();
				RZRESULT result = ChromaAnimationAPI::Uninit();
				if (result == 0)
				{
					_sChromaInitialized = false;
				}
				return result;
			}
		}

		return 0;
	}

	__declspec(dllexport) int PlayerLoadScene(const char* path)
	{
		lock_guard<mutex> guard(_sMutex); //make sure we aren't doing things while animations are playing

		return 0;
	}

	__declspec(dllexport) int PlayerSelectScene(int sceneIndex)
	{
		lock_guard<mutex> guard(_sMutex); //make sure we aren't doing things while animations are playing

		return 0;
	}	
}

const int GetColorArraySize1D(EChromaSDKDevice1DEnum device)
{
	const int maxLeds = ChromaAnimationAPI::GetMaxLeds((int)device);
	return maxLeds;
}

const int GetColorArraySize2D(EChromaSDKDevice2DEnum device)
{
	const int maxRow = ChromaAnimationAPI::GetMaxRow((int)device);
	const int maxColumn = ChromaAnimationAPI::GetMaxColumn((int)device);
	return maxRow * maxColumn;
}

void SetupAnimation1D(const char* path, EChromaSDKDevice1DEnum device)
{
	int animationId = ChromaAnimationAPI::GetAnimation(path);
	if (animationId == -1)
	{
		animationId = ChromaAnimationAPI::CreateAnimationInMemory((int)EChromaSDKDeviceTypeEnum::DE_1D, (int)device);
		ChromaAnimationAPI::CopyAnimation(animationId, path);
		ChromaAnimationAPI::CloseAnimation(animationId);
		ChromaAnimationAPI::MakeBlankFramesName(path, 1, 0.1f, 0);
	}
}

void SetupAnimation2D(const char* path, EChromaSDKDevice2DEnum device)
{
	int animationId = ChromaAnimationAPI::GetAnimation(path);
	if (animationId == -1)
	{
		animationId = ChromaAnimationAPI::CreateAnimationInMemory((int)EChromaSDKDeviceTypeEnum::DE_2D, (int)device);
		ChromaAnimationAPI::CopyAnimation(animationId, path);
		ChromaAnimationAPI::CloseAnimation(animationId);
		ChromaAnimationAPI::MakeBlankFramesName(path, 1, 0.1f, 0);
	}
}

void SetAmbientColor1D(EChromaSDKDevice1DEnum device, int* colors, int ambientColor)
{
	const int size = GetColorArraySize1D(device);
	for (int i = 0; i < size; ++i)
	{
		if (colors[i] == 0)
		{
			colors[i] = ambientColor;
		}
	}
}

void SetAmbientColor2D(EChromaSDKDevice2DEnum device, int* colors, int ambientColor)
{
	const int size = GetColorArraySize2D(device);
	for (int i = 0; i < size; ++i)
	{
		if (colors[i] == 0)
		{
			colors[i] = ambientColor;
		}
	}
}

void SetAmbientColor(int ambientColor,
	int* colorsChromaLink,
	int* colorsHeadset,
	int* colorsKeyboard,
	int* colorsKeypad,
	int* colorsMouse,
	int* colorsMousepad)
{
	// Set ambient color
	for (int d = (int)EChromaSDKDeviceEnum::DE_ChromaLink; d < (int)EChromaSDKDeviceEnum::DE_MAX; ++d)
	{
		switch ((EChromaSDKDeviceEnum)d)
		{
		case EChromaSDKDeviceEnum::DE_ChromaLink:
			SetAmbientColor1D(EChromaSDKDevice1DEnum::DE_ChromaLink, colorsChromaLink, ambientColor);
			break;
		case EChromaSDKDeviceEnum::DE_Headset:
			SetAmbientColor1D(EChromaSDKDevice1DEnum::DE_Headset, colorsHeadset, ambientColor);
			break;
		case EChromaSDKDeviceEnum::DE_Keyboard:
			SetAmbientColor2D(EChromaSDKDevice2DEnum::DE_Keyboard, colorsKeyboard, ambientColor);
			break;
		case EChromaSDKDeviceEnum::DE_Keypad:
			SetAmbientColor2D(EChromaSDKDevice2DEnum::DE_Keypad, colorsKeypad, ambientColor);
			break;
		case EChromaSDKDeviceEnum::DE_Mouse:
			SetAmbientColor2D(EChromaSDKDevice2DEnum::DE_Mouse, colorsMouse, ambientColor);
			break;
		case EChromaSDKDeviceEnum::DE_Mousepad:
			SetAmbientColor1D(EChromaSDKDevice1DEnum::DE_Mousepad, colorsMousepad, ambientColor);
			break;
		}
	}
}

void WorkerChroma()
{
	const int sizeChromaLink = GetColorArraySize1D(EChromaSDKDevice1DEnum::DE_ChromaLink);
	const int sizeHeadset = GetColorArraySize1D(EChromaSDKDevice1DEnum::DE_Headset);
	const int sizeKeyboard = GetColorArraySize2D(EChromaSDKDevice2DEnum::DE_Keyboard);
	const int sizeKeypad = GetColorArraySize2D(EChromaSDKDevice2DEnum::DE_Keypad);
	const int sizeMouse = GetColorArraySize2D(EChromaSDKDevice2DEnum::DE_Mouse);
	const int sizeMousepad = GetColorArraySize1D(EChromaSDKDevice1DEnum::DE_Mousepad);

	int* colorsChromaLink = new int[sizeChromaLink];
	int* colorsHeadset = new int[sizeHeadset];
	int* colorsKeyboard = new int[sizeKeyboard];
	int* colorsKeypad = new int[sizeKeypad];
	int* colorsMouse = new int[sizeMouse];
	int* colorsMousepad = new int[sizeMousepad];

	int* tempColorsChromaLink = new int[sizeChromaLink];
	int* tempColorsHeadset = new int[sizeHeadset];
	int* tempColorsKeyboard = new int[sizeKeyboard];
	int* tempColorsKeypad = new int[sizeKeypad];
	int* tempColorsMouse = new int[sizeMouse];
	int* tempColorsMousepad = new int[sizeMousepad];

	int ambientColor = ChromaAnimationAPI::GetRGB(0, 255, 0);

	while (_sWaitForExit)
	{
		//comment out for performance reasons
		//lock_guard<mutex> guard(_sMutex); //make sure it's safe to do Chroma things

		if (_sChromaInitialized)
		{
			// start with a blank frame
			memset(colorsChromaLink, 0, sizeof(int) * sizeChromaLink);
			memset(colorsHeadset, 0, sizeof(int) * sizeHeadset);
			memset(colorsKeyboard, 0, sizeof(int) * sizeKeyboard);
			memset(colorsKeypad, 0, sizeof(int) * sizeKeypad);
			memset(colorsMouse, 0, sizeof(int) * sizeMouse);
			memset(colorsMousepad, 0, sizeof(int) * sizeMousepad);

			SetupAnimation1D(ANIMATION_FINAL_CHROMA_LINK, EChromaSDKDevice1DEnum::DE_ChromaLink);
			SetupAnimation1D(ANIMATION_FINAL_HEADSET, EChromaSDKDevice1DEnum::DE_Headset);
			SetupAnimation2D(ANIMATION_FINAL_KEYBOARD, EChromaSDKDevice2DEnum::DE_Keyboard);
			SetupAnimation2D(ANIMATION_FINAL_KEYPAD, EChromaSDKDevice2DEnum::DE_Keypad);
			SetupAnimation2D(ANIMATION_FINAL_MOUSE, EChromaSDKDevice2DEnum::DE_Mouse);
			SetupAnimation1D(ANIMATION_FINAL_MOUSEPAD, EChromaSDKDevice1DEnum::DE_Mousepad);

			SetAmbientColor(ambientColor,
				colorsChromaLink,
				colorsHeadset,
				colorsKeyboard,
				colorsKeypad,
				colorsMouse,
				colorsMousepad);

			ChromaAnimationAPI::UpdateFrameName(ANIMATION_FINAL_CHROMA_LINK, 0, 0.1f, colorsChromaLink, sizeChromaLink);
			ChromaAnimationAPI::UpdateFrameName(ANIMATION_FINAL_HEADSET, 0, 0.1f, colorsHeadset, sizeHeadset);
			ChromaAnimationAPI::UpdateFrameName(ANIMATION_FINAL_KEYBOARD, 0, 0.1f, colorsKeyboard, sizeKeyboard);
			ChromaAnimationAPI::UpdateFrameName(ANIMATION_FINAL_KEYPAD, 0, 0.1f, colorsKeypad, sizeKeypad);
			ChromaAnimationAPI::UpdateFrameName(ANIMATION_FINAL_MOUSE, 0, 0.1f, colorsMouse, sizeMouse);
			ChromaAnimationAPI::UpdateFrameName(ANIMATION_FINAL_MOUSEPAD, 0, 0.1f, colorsMousepad, sizeMousepad);

			// display the change
			ChromaAnimationAPI::PreviewFrameName(ANIMATION_FINAL_CHROMA_LINK, 0);
			ChromaAnimationAPI::PreviewFrameName(ANIMATION_FINAL_HEADSET, 0);
			ChromaAnimationAPI::PreviewFrameName(ANIMATION_FINAL_KEYBOARD, 0);
			ChromaAnimationAPI::PreviewFrameName(ANIMATION_FINAL_KEYPAD, 0);
			ChromaAnimationAPI::PreviewFrameName(ANIMATION_FINAL_MOUSE, 0);
			ChromaAnimationAPI::PreviewFrameName(ANIMATION_FINAL_MOUSEPAD, 0);
		}

		// update at 10 FPS
		this_thread::sleep_for(chrono::milliseconds(100));
	}

	delete[] colorsChromaLink;
	delete[] colorsHeadset;
	delete[] colorsKeyboard;
	delete[] colorsKeypad;
	delete[] colorsMouse;
	delete[] colorsMousepad;

	delete[] tempColorsChromaLink;
	delete[] tempColorsHeadset;
	delete[] tempColorsKeyboard;
	delete[] tempColorsKeypad;
	delete[] tempColorsMouse;
	delete[] tempColorsMousepad;
}
