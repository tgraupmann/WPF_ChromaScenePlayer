// pch.cpp: source file corresponding to the pre-compiled header

#include "pch.h"
#include <Windows.h>
typedef unsigned char byte;
#include "Razer\ChromaAnimationAPI.h"
#include <mutex>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <iostream>
#include <sstream>
#include <json/json.h>

using namespace ChromaSDK;
using namespace std;

// When you are using pre-compiled headers, this source file is necessary for compilation to succeed.

static bool _sInitializedAPI = false;
static bool _sChromaInitialized = false;
static bool _sWaitForExit = true;
static mutex _sMutex;
static std::thread* _sThreadChroma = nullptr;
static string _sLastPath;
static string _sPath;

// This final animation will have a single frame
// Any color changes will immediately display in the next frame update.
const char* ANIMATION_FINAL_CHROMA_LINK = "Dynamic\\Final_ChromaLink.chroma";
const char* ANIMATION_FINAL_HEADSET = "Dynamic\\Final_Headset.chroma";
const char* ANIMATION_FINAL_KEYBOARD = "Dynamic\\Final_Keyboard.chroma";
const char* ANIMATION_FINAL_KEYPAD = "Dynamic\\Final_Keypad.chroma";
const char* ANIMATION_FINAL_MOUSE = "Dynamic\\Final_Mouse.chroma";
const char* ANIMATION_FINAL_MOUSEPAD = "Dynamic\\Final_Mousepad.chroma";

class DeviceFrameIndex
{
public:
	DeviceFrameIndex() {
		_mFrameIndex[(int)EChromaSDKDeviceEnum::DE_ChromaLink] = 0;
		_mFrameIndex[(int)EChromaSDKDeviceEnum::DE_Headset] = 0;
		_mFrameIndex[(int)EChromaSDKDeviceEnum::DE_Keyboard] = 0;
		_mFrameIndex[(int)EChromaSDKDeviceEnum::DE_Keypad] = 0;
		_mFrameIndex[(int)EChromaSDKDeviceEnum::DE_Mouse] = 0;
		_mFrameIndex[(int)EChromaSDKDeviceEnum::DE_Mousepad] = 0;
	}
	// Index corresponds to EChromaSDKDeviceEnum;
	int _mFrameIndex[6];
};

struct Effect
{
public:
	string _mAnimation = "";
	bool _mState = false;
	int _mPrimaryColor = 0;
	int _mSecondaryColor = 0;
	int _mSpeed = 1;
	string _mBlend = "";
	string _mMode = "";

	DeviceFrameIndex _mFrameIndex;
};

struct Scene
{
public:
	vector<Effect> _mEffects;
};

static int _mCurrentScene = 1;

bool StringStartsWith(const string& str, const string& search)
{
	return (str.rfind(search, 0) == 0);
}

int StringParseInt(const string& str, int base)
{
	return stoi(str, nullptr, base);
}

// trim from start
string StringLTrim(std::string s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
		}));
	return s;
}

// trim from end
string StringRTrim(std::string s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
		}).base(), s.end());
	return s;
}

// trim from both ends
string StringTrim(std::string s) {
	s = StringLTrim(s);
	s = StringRTrim(s);
	return s;
}

vector<string> StringSplit(string str, const string& delimiter)
{
	vector<string> result;

	size_t pos = 0;
	std::string token;
	while ((pos = str.find(delimiter)) != std::string::npos) {
		token = str.substr(0, pos);
		result.push_back(token);
		str.erase(0, pos + delimiter.length());
	}
	result.push_back(str);

	return result;
}

int RgbStrToInt(const string& rgbStr) {
	vector<string> step1 = StringSplit(rgbStr, "(");
	vector<string> step2 = StringSplit(step1[1], ")");
	vector<string> step3 = StringSplit(step2[0], ",");
	vector<string> components = step3;
	int red = StringParseInt(StringTrim(components[0]), 10);
	int green = StringParseInt(StringTrim(components[1]), 10);
	int blue = StringParseInt(StringTrim(components[2]), 10);
	return (red & 0xFF) | ((green & 0xFF) << 8) | ((blue & 0xFF) << 16);
}

int HexStrToInt(const string& hexStr) {
	int val = StringParseInt(StringSplit(hexStr, "#")[1], 16);
	int red = (val >> 16) & 0xFF;
	int green = (val >> 8) & 0xFF;
	int blue = val & 0xFF;
	return (red & 0xFF) | ((green & 0xFF) << 8) | ((blue & 0xFF) << 16);
}

int ReadColor(string strRGB)
{
	if (StringStartsWith(strRGB, "rgb")) {
		return RgbStrToInt(strRGB);
	}
	else {
		return HexStrToInt(strRGB);
	}
}

vector<Scene> ReadJsonScenes()
{
	vector<Scene> result;

	std::ifstream inFile;
	inFile.open(_sPath.c_str()); //open the input file

	std::stringstream strStream;
	strStream << inFile.rdbuf(); //read the file
	std::string str = strStream.str(); //str holds the content of the file

	if (str.empty())
	{
		return result;
	}

	Json::Value root;
	Json::Reader reader;
	reader.parse(str, root);

	vector<Scene> newScenes;
	if (root.isArray())
	{
		for (Json::Value::ArrayIndex i = 0; i != root.size(); i++)
		{
			Json::Value scene = root[i];
			Scene newScene;
			if (scene.isMember("effects"))
			{
				Json::Value effects = scene["effects"];
				if (effects.isArray())
				{
					for (Json::Value::ArrayIndex j = 0; j != effects.size(); j++)
					{
						Json::Value effect = effects[j];
						Effect newEffect;
						if (effect.isMember("animation"))
						{
							newEffect._mAnimation = effect["animation"].asString();
						}
						if (effect.isMember("state"))
						{
							newEffect._mState = effect["state"].asBool();
						}
						if (effect.isMember("primaryColor"))
						{
							newEffect._mPrimaryColor = ReadColor(effect["primaryColor"].asString());
						}
						if (effect.isMember("secondaryColor"))
						{
							newEffect._mSecondaryColor = ReadColor(effect["secondaryColor"].asString());
						}
						if (effect.isMember("speed"))
						{
							newEffect._mSpeed = effect["speed"].asInt();
						}
						if (effect.isMember("blend"))
						{
							newEffect._mBlend = effect["blend"].asString();
						}
						if (effect.isMember("mode"))
						{
							newEffect._mMode = effect["mode"].asString();
						}
						newScene._mEffects.push_back(newEffect);
					}
				}
			}
			newScenes.push_back(newScene);
		}
	}

	return newScenes;
}

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
		_sPath = path;
		return 0;
	}

	__declspec(dllexport) int PlayerSelectScene(int sceneIndex)
	{
		lock_guard<mutex> guard(_sMutex); //make sure we aren't doing things while animations are playing

		_mCurrentScene = sceneIndex;

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

int MultiplyColor(int color1, int color2) {
	int redColor1 = color1 & 0xFF;
	int greenColor1 = (color1 >> 8) & 0xFF;
	int blueColor1 = (color1 >> 16) & 0xFF;

	int redColor2 = color2 & 0xFF;
	int greenColor2 = (color2 >> 8) & 0xFF;
	int blueColor2 = (color2 >> 16) & 0xFF;

	int red = floor(255 * ((redColor1 / 255.0f) * (redColor2 / 255.0f)));
	int green = floor(255 * ((greenColor1 / 255.0f) * (greenColor2 / 255.0f)));
	int blue = floor(255 * ((blueColor1 / 255.0f) * (blueColor2 / 255.0f)));

	return ChromaAnimationAPI::GetRGB(red, green, blue);
}

int AverageColor(int color1, int color2) {
	return ChromaAnimationAPI::LerpColor(color1, color2, 0.5f);
}

int AddColor(int color1, int color2) {
	int redColor1 = color1 & 0xFF;
	int greenColor1 = (color1 >> 8) & 0xFF;
	int blueColor1 = (color1 >> 16) & 0xFF;

	int redColor2 = color2 & 0xFF;
	int greenColor2 = (color2 >> 8) & 0xFF;
	int blueColor2 = (color2 >> 16) & 0xFF;

	int red = min(redColor1 + redColor2, 255) & 0xFF;
	int green = min(greenColor1 + greenColor2, 255) & 0xFF;
	int blue = min(blueColor1 + blueColor2, 255) & 0xFF;

	return ChromaAnimationAPI::GetRGB(red, green, blue);
}

int SubtractColor(int color1, int color2) {
	int redColor1 = color1 & 0xFF;
	int greenColor1 = (color1 >> 8) & 0xFF;
	int blueColor1 = (color1 >> 16) & 0xFF;

	int redColor2 = color2 & 0xFF;
	int greenColor2 = (color2 >> 8) & 0xFF;
	int blueColor2 = (color2 >> 16) & 0xFF;

	int red = max(redColor1 - redColor2, 0) & 0xFF;
	int green = max(greenColor1 - greenColor2, 0) & 0xFF;
	int blue = max(blueColor1 - blueColor2, 0) & 0xFF;

	return ChromaAnimationAPI::GetRGB(red, green, blue);
}

int MaxColor(int color1, int color2) {
	int redColor1 = color1 & 0xFF;
	int greenColor1 = (color1 >> 8) & 0xFF;
	int blueColor1 = (color1 >> 16) & 0xFF;

	int redColor2 = color2 & 0xFF;
	int greenColor2 = (color2 >> 8) & 0xFF;
	int blueColor2 = (color2 >> 16) & 0xFF;

	int red = max(redColor1, redColor2) & 0xFF;
	int green = max(greenColor1, greenColor2) & 0xFF;
	int blue = max(blueColor1, blueColor2) & 0xFF;

	return ChromaAnimationAPI::GetRGB(red, green, blue);
}

int MinColor(int color1, int color2) {
	int redColor1 = color1 & 0xFF;
	int greenColor1 = (color1 >> 8) & 0xFF;
	int blueColor1 = (color1 >> 16) & 0xFF;

	int redColor2 = color2 & 0xFF;
	int greenColor2 = (color2 >> 8) & 0xFF;
	int blueColor2 = (color2 >> 16) & 0xFF;

	int red = min(redColor1, redColor2) & 0xFF;
	int green = min(greenColor1, greenColor2) & 0xFF;
	int blue = min(blueColor1, blueColor2) & 0xFF;

	return ChromaAnimationAPI::GetRGB(red, green, blue);
}

int InvertColor(int color) {
	int red = 255 - (color & 0xFF);
	int green = 255 - ((color >> 8) & 0xFF);
	int blue = 255 - ((color >> 16) & 0xFF);

	return ChromaAnimationAPI::GetRGB(red, green, blue);
}

int MultiplyNonZeroTargetColorLerp(int color1, int color2, int inputColor) {
	if (inputColor == 0)
	{
		return inputColor;
	}
	int red = (inputColor & 0xFF) / 255.0;
	int green = ((inputColor & 0xFF00) >> 8) / 255.0;
	int blue = ((inputColor & 0xFF0000) >> 16) / 255.0;
	float t = (red + green + blue) / 3.0f;
	return ChromaAnimationAPI::LerpColor(color1, color2, t);
}

int Thresh(int color1, int color2, int inputColor) {
	int red = (inputColor & 0xFF) / 255.0;
	int green = ((inputColor & 0xFF00) >> 8) / 255.0;
	int blue = ((inputColor & 0xFF0000) >> 16) / 255.0;
	float t = (red + green + blue) / 3.0f;
	if (t == 0.0)
	{
		return 0;
	}
	if (t < 0.5)
	{
		return color1;
	}
	else
	{
		return color2;
	}
}

void BlendAnimation1D(const Effect& effect, DeviceFrameIndex& deviceFrameIndex, int device, EChromaSDKDevice1DEnum device1d, const char* animationName,
	int* colors, int* tempColors)
{
	const int size = GetColorArraySize1D(device1d);
	const int frameId = deviceFrameIndex._mFrameIndex[device];
	const int frameCount = ChromaAnimationAPI::GetFrameCountName(animationName);
	if (frameId < frameCount)
	{
		//cout << animationName << ": " << (1 + frameId) << " of " << frameCount << endl;
		float duration;
		int animationId = ChromaAnimationAPI::GetAnimation(animationName);
		ChromaAnimationAPI::GetFrame(animationId, frameId, &duration, tempColors, size);
		for (int i = 0; i < size; ++i)
		{
			int color1 = colors[i]; //target
			int tempColor = tempColors[i]; //source

			// BLEND
			int color2;
			if (effect._mBlend.compare("none") == 0)
			{
				color2 = tempColor; //source
			}
			else if (effect._mBlend.compare("invert") == 0)
			{
				if (tempColor != 0) //source
				{
					color2 = InvertColor(tempColor); //source inverted
				}
				color2 = 0;
			}
			else if (effect._mBlend.compare("thresh") == 0)
			{
				color2 = Thresh(effect._mPrimaryColor, effect._mSecondaryColor, tempColor); //source
			}
			else // if (effect._mBlend.compare("lerp") == 0) //default
			{
				color2 = MultiplyNonZeroTargetColorLerp(effect._mPrimaryColor, effect._mSecondaryColor, tempColor); //source
			}

			// MODE
			if (effect._mMode.compare("max") == 0)
			{
				colors[i] = MaxColor(color1, color2);
			}
			else if (effect._mMode.compare("min") == 0)
			{
				colors[i] = MinColor(color1, color2);
			}
			else if (effect._mMode.compare("average") == 0)
			{
				colors[i] = AverageColor(color1, color2);
			}
			else if (effect._mMode.compare("multiply") == 0)
			{
				colors[i] = MultiplyColor(color1, color2);
			}
			else if (effect._mMode.compare("add") == 0)
			{
				colors[i] = AddColor(color1, color2);
			}
			else if (effect._mMode.compare("subtract") == 0)
			{
				colors[i] = SubtractColor(color1, color2);
			}
			else // if (effect._mMode.compare("replace") == 0) //default
			{
				if (color2 != 0) {
					colors[i] = color2;
				}
			}
		}
		deviceFrameIndex._mFrameIndex[device] = (frameId + frameCount + effect._mSpeed) % frameCount;
	}
}

void BlendAnimation2D(const Effect& effect, DeviceFrameIndex& deviceFrameIndex, int device, EChromaSDKDevice2DEnum device2D, const char* animationName,
	int* colors, int* tempColors)
{
	const int size = GetColorArraySize2D(device2D);
	const int frameId = deviceFrameIndex._mFrameIndex[device];
	const int frameCount = ChromaAnimationAPI::GetFrameCountName(animationName);
	if (frameId < frameCount)
	{
		//cout << animationName << ": " << (1 + frameId) << " of " << frameCount << endl;
		float duration;
		int animationId = ChromaAnimationAPI::GetAnimation(animationName);
		ChromaAnimationAPI::GetFrame(animationId, frameId, &duration, tempColors, size);
		for (int i = 0; i < size; ++i)
		{
			int color1 = colors[i]; //target
			int tempColor = tempColors[i]; //source

			// BLEND
			int color2;
			if (effect._mBlend.compare("none") == 0)
			{
				color2 = tempColor; //source
			}
			else if (effect._mBlend.compare("invert") == 0)
			{
				if (tempColor != 0) //source
				{
					color2 = InvertColor(tempColor); //source inverted
				}
				color2 = 0;
			}
			else if (effect._mBlend.compare("thresh") == 0)
			{
				color2 = Thresh(effect._mPrimaryColor, effect._mSecondaryColor, tempColor); //source
			}
			else // if (effect._mBlend.compare("lerp") == 0) //default
			{
				color2 = MultiplyNonZeroTargetColorLerp(effect._mPrimaryColor, effect._mSecondaryColor, tempColor); //source
			}

			// MODE
			if (effect._mMode.compare("max") == 0)
			{
				colors[i] = MaxColor(color1, color2);
			}
			else if (effect._mMode.compare("min") == 0)
			{
				colors[i] = MinColor(color1, color2);
			}
			else if (effect._mMode.compare("average") == 0)
			{
				colors[i] = AverageColor(color1, color2);
			}
			else if (effect._mMode.compare("multiply") == 0)
			{
				colors[i] = MultiplyColor(color1, color2);
			}
			else if (effect._mMode.compare("add") == 0)
			{
				colors[i] = AddColor(color1, color2);
			}
			else if (effect._mMode.compare("subtract") == 0)
			{
				colors[i] = SubtractColor(color1, color2);
			}
			else // if (effect._mMode.compare("replace") == 0) //default
			{
				if (color2 != 0) {
					colors[i] = color2;
				}
			}
		}
		deviceFrameIndex._mFrameIndex[device] = (frameId + frameCount + effect._mSpeed) % frameCount;
	}
}

void BlendAnimations(Scene& scene,
	int* colorsChromaLink, int* tempColorsChromaLink,
	int* colorsHeadset, int* tempColorsHeadset,
	int* colorsKeyboard, int* tempColorsKeyboard,
	int* colorsKeypad, int* tempColorsKeypad,
	int* colorsMouse, int* tempColorsMouse,
	int* colorsMousepad, int* tempColorsMousepad)
{
	// blend active animations
	vector<Effect>& effects = scene._mEffects;
	for (vector<Effect>::iterator iter = effects.begin(); iter != effects.end(); ++iter)
	{
		Effect& effect = *iter;
		if (effect._mState)
		{
			DeviceFrameIndex& deviceFrameIndex = effect._mFrameIndex;

			//iterate all device types
			for (int d = (int)EChromaSDKDeviceEnum::DE_ChromaLink; d < (int)EChromaSDKDeviceEnum::DE_MAX; ++d)
			{
				string animationName = effect._mAnimation;

				switch ((EChromaSDKDeviceEnum)d)
				{
				case EChromaSDKDeviceEnum::DE_ChromaLink:
					animationName += "_ChromaLink.chroma";
					BlendAnimation1D(effect, deviceFrameIndex, d, EChromaSDKDevice1DEnum::DE_ChromaLink, animationName.c_str(), colorsChromaLink, tempColorsChromaLink);
					break;
				case EChromaSDKDeviceEnum::DE_Headset:
					animationName += "_Headset.chroma";
					BlendAnimation1D(effect, deviceFrameIndex, d, EChromaSDKDevice1DEnum::DE_Headset, animationName.c_str(), colorsHeadset, tempColorsHeadset);
					break;
				case EChromaSDKDeviceEnum::DE_Keyboard:
					animationName += "_Keyboard.chroma";
					BlendAnimation2D(effect, deviceFrameIndex, d, EChromaSDKDevice2DEnum::DE_Keyboard, animationName.c_str(), colorsKeyboard, tempColorsKeyboard);
					break;
				case EChromaSDKDeviceEnum::DE_Keypad:
					animationName += "_Keypad.chroma";
					BlendAnimation2D(effect, deviceFrameIndex, d, EChromaSDKDevice2DEnum::DE_Keypad, animationName.c_str(), colorsKeypad, tempColorsKeypad);
					break;
				case EChromaSDKDeviceEnum::DE_Mouse:
					animationName += "_Mouse.chroma";
					BlendAnimation2D(effect, deviceFrameIndex, d, EChromaSDKDevice2DEnum::DE_Mouse, animationName.c_str(), colorsMouse, tempColorsMouse);
					break;
				case EChromaSDKDeviceEnum::DE_Mousepad:
					animationName += "_Mousepad.chroma";
					BlendAnimation1D(effect, deviceFrameIndex, d, EChromaSDKDevice1DEnum::DE_Mousepad, animationName.c_str(), colorsMousepad, tempColorsMousepad);
					break;
				}
			}
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

	//int ambientColor = ChromaAnimationAPI::GetRGB(0, 255, 0);

	int currentScene = 0;

	vector<Scene> scenes;

	while (_sWaitForExit)
	{
		//comment out for performance reasons
		//lock_guard<mutex> guard(_sMutex); //make sure it's safe to do Chroma things

		if (!_sPath.empty() &&
			_sLastPath.compare(_sPath) != 0)
		{
			_sLastPath = _sPath;
			scenes = ReadJsonScenes();
		}

		// Change the current scene in the worker
		currentScene = _mCurrentScene;

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

			/*
			SetAmbientColor(ambientColor,
				colorsChromaLink,
				colorsHeadset,
				colorsKeyboard,
				colorsKeypad,
				colorsMouse,
				colorsMousepad);
			*/

			if (currentScene >= 0 && currentScene < (int)scenes.size())
			{
				Scene& scene = scenes[currentScene];
				BlendAnimations(scene,
					colorsChromaLink, tempColorsChromaLink,
					colorsHeadset, tempColorsHeadset,
					colorsKeyboard, tempColorsKeyboard,
					colorsKeypad, tempColorsKeypad,
					colorsMouse, tempColorsMouse,
					colorsMousepad, tempColorsMousepad);
			}

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
