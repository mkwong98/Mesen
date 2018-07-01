#include "stdafx.h"
#include "EmulationSettings.h"
#include "Console.h"
#include "RewindManager.h"

//Version 0.9.5
uint16_t EmulationSettings::_versionMajor = 0;
uint8_t EmulationSettings::_versionMinor = 9;
uint8_t EmulationSettings::_versionRevision = 5;

Language EmulationSettings::_displayLanguage = Language::English;

const vector<uint32_t> EmulationSettings::_speedValues = { { 1, 3, 6, 12, 25, 50, 75, 100, 150, 200, 250, 300, 350, 400, 450, 500, 750, 1000, 2000, 4000 } };

string EmulationSettings::_pauseScreenMessage;

SimpleLock EmulationSettings::_lock;
uint64_t EmulationSettings::_flags = 0;

bool EmulationSettings::_audioSettingsChanged = true;
uint32_t EmulationSettings::_audioLatency = 50;
double EmulationSettings::_channelVolume[11] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };
double EmulationSettings::_channelPanning[11] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };
EqualizerFilterType EmulationSettings::_equalizerFilterType = EqualizerFilterType::None;
vector<double> EmulationSettings::_bandGains = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
vector<double> EmulationSettings::_bands = { { 40,56,80,113,160,225,320,450,600,750,1000,2000,3000,4000,5000,6000,7000,10000,12500,15000 } };
double EmulationSettings::_masterVolume = 1.0;
double EmulationSettings::_volumeReduction = 0.75;
uint32_t EmulationSettings::_sampleRate = 44100;
StereoFilter EmulationSettings::_stereoFilter = StereoFilter::None;
int32_t EmulationSettings::_stereoDelay = 0;
double EmulationSettings::_stereoAngle = 0;
double EmulationSettings::_reverbStrength = 0;
double EmulationSettings::_reverbDelay = 0;
uint32_t EmulationSettings::_crossFeedRatio = 0;
SimpleLock EmulationSettings::_equalizerLock;

NesModel EmulationSettings::_model = NesModel::Auto;
PpuModel EmulationSettings::_ppuModel = PpuModel::Ppu2C02;

uint32_t EmulationSettings::_emulationSpeed = 100;
uint32_t EmulationSettings::_turboSpeed = 300;
uint32_t EmulationSettings::_rewindSpeed = 100;

uint32_t EmulationSettings::_rewindBufferSize = 300;

bool EmulationSettings::_hasOverclock = false;
uint32_t EmulationSettings::_overclockRate = 100;
uint32_t EmulationSettings::_extraScanlinesBeforeNmi = 0;
uint32_t EmulationSettings::_extraScanlinesAfterNmi = 0;
double EmulationSettings::_effectiveOverclockRate = 100;
bool EmulationSettings::_overclockAdjustApu = true;
bool EmulationSettings::_disableOverclocking = false;

int32_t EmulationSettings::_nsfAutoDetectSilenceDelay = 3000;
int32_t EmulationSettings::_nsfMoveToNextTrackTime = 120;
bool EmulationSettings::_nsfDisableApuIrqs = true;

uint32_t EmulationSettings::_autoSaveDelay = 5;
bool EmulationSettings::_autoSaveNotify = false;

bool EmulationSettings::_keyboardModeEnabled = false;

SimpleLock EmulationSettings::_shortcutLock;
std::unordered_map<uint32_t, KeyCombination> EmulationSettings::_emulatorKeys[3];
std::unordered_map<uint32_t, vector<KeyCombination>> EmulationSettings::_shortcutSupersets[3];

RamPowerOnState EmulationSettings::_ramPowerOnState = RamPowerOnState::AllZeros;
uint32_t EmulationSettings::_dipSwitches = 0;
VsInputType EmulationSettings::_vsInputType = VsInputType::Default;

InputDisplaySettings EmulationSettings::_inputDisplaySettings = { 0, InputDisplayPosition::TopLeft, false };

OverscanDimensions EmulationSettings::_overscan;
VideoFilterType EmulationSettings::_videoFilterType = VideoFilterType::None;
VideoResizeFilter EmulationSettings::_resizeFilter = VideoResizeFilter::NearestNeighbor;
double EmulationSettings::_videoScale = 1;
VideoAspectRatio EmulationSettings::_aspectRatio = VideoAspectRatio::NoStretching;
double EmulationSettings::_customAspectRatio = 1.0;
PictureSettings EmulationSettings::_pictureSettings;
NtscFilterSettings EmulationSettings::_ntscFilterSettings;
bool EmulationSettings::_backgroundEnabled = true;
bool EmulationSettings::_spritesEnabled = true;
uint32_t EmulationSettings::_screenRotation = 0;
uint32_t EmulationSettings::_exclusiveRefreshRate = 60;

ConsoleType EmulationSettings::_consoleType = ConsoleType::Nes;
ExpansionPortDevice EmulationSettings::_expansionDevice = ExpansionPortDevice::None;
ControllerType EmulationSettings::_controllerTypes[4] = { ControllerType::None, ControllerType::None, ControllerType::None, ControllerType::None };
KeyMappingSet EmulationSettings::_controllerKeys[4] = { KeyMappingSet(), KeyMappingSet(), KeyMappingSet(), KeyMappingSet() };
bool EmulationSettings::_needControllerUpdate = false;
uint32_t EmulationSettings::_zapperDetectionRadius = 0;
std::unordered_map<int, double> EmulationSettings::_mouseSensitivity;
int32_t EmulationSettings::_inputPollScanline = 241;

uint32_t EmulationSettings::_defaultPpuPalette[64] = { /* 2C02 */ 0xFF666666, 0xFF002A88, 0xFF1412A7, 0xFF3B00A4, 0xFF5C007E, 0xFF6E0040, 0xFF6C0600, 0xFF561D00, 0xFF333500, 0xFF0B4800, 0xFF005200, 0xFF004F08, 0xFF00404D, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFADADAD, 0xFF155FD9, 0xFF4240FF, 0xFF7527FE, 0xFFA01ACC, 0xFFB71E7B, 0xFFB53120, 0xFF994E00, 0xFF6B6D00, 0xFF388700, 0xFF0C9300, 0xFF008F32, 0xFF007C8D, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFEFF, 0xFF64B0FF, 0xFF9290FF, 0xFFC676FF, 0xFFF36AFF, 0xFFFE6ECC, 0xFFFE8170, 0xFFEA9E22, 0xFFBCBE00, 0xFF88D800, 0xFF5CE430, 0xFF45E082, 0xFF48CDDE, 0xFF4F4F4F, 0xFF000000, 0xFF000000, 0xFFFFFEFF, 0xFFC0DFFF, 0xFFD3D2FF, 0xFFE8C8FF, 0xFFFBC2FF, 0xFFFEC4EA, 0xFFFECCC5, 0xFFF7D8A5, 0xFFE4E594, 0xFFCFEF96, 0xFFBDF4AB, 0xFFB3F3CC, 0xFFB5EBF2, 0xFFB8B8B8, 0xFF000000, 0xFF000000 };

uint32_t EmulationSettings::_currentPalette[64] = { 0xFF666666, 0xFF002A88, 0xFF1412A7, 0xFF3B00A4, 0xFF5C007E, 0xFF6E0040, 0xFF6C0600, 0xFF561D00, 0xFF333500, 0xFF0B4800, 0xFF005200, 0xFF004F08, 0xFF00404D, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFADADAD, 0xFF155FD9, 0xFF4240FF, 0xFF7527FE, 0xFFA01ACC, 0xFFB71E7B, 0xFFB53120, 0xFF994E00, 0xFF6B6D00, 0xFF388700, 0xFF0C9300, 0xFF008F32, 0xFF007C8D, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFEFF, 0xFF64B0FF, 0xFF9290FF, 0xFFC676FF, 0xFFF36AFF, 0xFFFE6ECC, 0xFFFE8170, 0xFFEA9E22, 0xFFBCBE00, 0xFF88D800, 0xFF5CE430, 0xFF45E082, 0xFF48CDDE, 0xFF4F4F4F, 0xFF000000, 0xFF000000, 0xFFFFFEFF, 0xFFC0DFFF, 0xFFD3D2FF, 0xFFE8C8FF, 0xFFFBC2FF, 0xFFFEC4EA, 0xFFFECCC5, 0xFFF7D8A5, 0xFFE4E594, 0xFFCFEF96, 0xFFBDF4AB, 0xFFB3F3CC, 0xFFB5EBF2, 0xFFB8B8B8, 0xFF000000, 0xFF000000 };

uint32_t EmulationSettings::_ppuPaletteArgb[11][64] = {
	/* 2C02 */			{ 0xFF666666, 0xFF002A88, 0xFF1412A7, 0xFF3B00A4, 0xFF5C007E, 0xFF6E0040, 0xFF6C0600, 0xFF561D00, 0xFF333500, 0xFF0B4800, 0xFF005200, 0xFF004F08, 0xFF00404D, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFADADAD, 0xFF155FD9, 0xFF4240FF, 0xFF7527FE, 0xFFA01ACC, 0xFFB71E7B, 0xFFB53120, 0xFF994E00, 0xFF6B6D00, 0xFF388700, 0xFF0C9300, 0xFF008F32, 0xFF007C8D, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFEFF, 0xFF64B0FF, 0xFF9290FF, 0xFFC676FF, 0xFFF36AFF, 0xFFFE6ECC, 0xFFFE8170, 0xFFEA9E22, 0xFFBCBE00, 0xFF88D800, 0xFF5CE430, 0xFF45E082, 0xFF48CDDE, 0xFF4F4F4F, 0xFF000000, 0xFF000000, 0xFFFFFEFF, 0xFFC0DFFF, 0xFFD3D2FF, 0xFFE8C8FF, 0xFFFBC2FF, 0xFFFEC4EA, 0xFFFECCC5, 0xFFF7D8A5, 0xFFE4E594, 0xFFCFEF96, 0xFFBDF4AB, 0xFFB3F3CC, 0xFFB5EBF2, 0xFFB8B8B8, 0xFF000000, 0xFF000000 },
	/* 2C03 */			{ 0xFF6D6D6D, 0xFF002491, 0xFF0000DA, 0xFF6D48DA, 0xFF91006D, 0xFFB6006D, 0xFFB62400, 0xFF914800, 0xFF6D4800, 0xFF244800, 0xFF006D24, 0xFF009100, 0xFF004848, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFB6B6B6, 0xFF006DDA, 0xFF0048FF, 0xFF9100FF, 0xFFB600FF, 0xFFFF0091, 0xFFFF0000, 0xFFDA6D00, 0xFF916D00, 0xFF249100, 0xFF009100, 0xFF00B66D, 0xFF009191, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF6DB6FF, 0xFF9191FF, 0xFFDA6DFF, 0xFFFF00FF, 0xFFFF6DFF, 0xFFFF9100, 0xFFFFB600, 0xFFDADA00, 0xFF6DDA00, 0xFF00FF00, 0xFF48FFDA, 0xFF00FFFF, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFB6DAFF, 0xFFDAB6FF, 0xFFFFB6FF, 0xFFFF91FF, 0xFFFFB6B6, 0xFFFFDA91, 0xFFFFFF48, 0xFFFFFF6D, 0xFFB6FF48, 0xFF91FF6D, 0xFF48FFDA, 0xFF91DAFF, 0xFF000000, 0xFF000000, 0xFF000000 },
	/* 2C04-0001 */	{ 0xFFFFB6B6, 0xFFDA6DFF, 0xFFFF0000, 0xFF9191FF, 0xFF009191, 0xFF244800, 0xFF484848, 0xFFFF0091, 0xFFFFFFFF, 0xFF6D6D6D, 0xFFFFB600, 0xFFB6006D, 0xFF91006D, 0xFFDADA00, 0xFF6D4800, 0xFFFFFFFF, 0xFF6DB6FF, 0xFFDAB66D, 0xFF6D2400, 0xFF6DDA00, 0xFF91DAFF, 0xFFDAB6FF, 0xFFFFDA91, 0xFF0048FF, 0xFFFFDA00, 0xFF48FFDA, 0xFF000000, 0xFF480000, 0xFFDADADA, 0xFF919191, 0xFFFF00FF, 0xFF002491, 0xFF00006D, 0xFFB6DAFF, 0xFFFFB6FF, 0xFF00FF00, 0xFF00FFFF, 0xFF004848, 0xFF00B66D, 0xFFB600FF, 0xFF000000, 0xFF914800, 0xFFFF91FF, 0xFFB62400, 0xFF9100FF, 0xFF0000DA, 0xFFFF9100, 0xFF000000, 0xFF000000, 0xFF249100, 0xFFB6B6B6, 0xFF006D24, 0xFFB6FF48, 0xFF6D48DA, 0xFFFFFF00, 0xFFDA6D00, 0xFF004800, 0xFF006DDA, 0xFF009100, 0xFF242424, 0xFFFFFF6D, 0xFFFF6DFF, 0xFF916D00, 0xFF91FF6D },
	/* 2C04-0002 */	{ 0xFF000000, 0xFFFFB600, 0xFF916D00, 0xFFB6FF48, 0xFF91FF6D, 0xFFFF6DFF, 0xFF009191, 0xFFB6DAFF, 0xFFFF0000, 0xFF9100FF, 0xFFFFFF6D, 0xFFFF91FF, 0xFFFFFFFF, 0xFFDA6DFF, 0xFF91DAFF, 0xFF009100, 0xFF004800, 0xFF6DB6FF, 0xFFB62400, 0xFFDADADA, 0xFF00B66D, 0xFF6DDA00, 0xFF480000, 0xFF9191FF, 0xFF484848, 0xFFFF00FF, 0xFF00006D, 0xFF48FFDA, 0xFFDAB6FF, 0xFF6D4800, 0xFF000000, 0xFF6D48DA, 0xFF91006D, 0xFFFFDA91, 0xFFFF9100, 0xFFFFB6FF, 0xFF006DDA, 0xFF6D2400, 0xFFB6B6B6, 0xFF0000DA, 0xFFB600FF, 0xFFFFDA00, 0xFF6D6D6D, 0xFF244800, 0xFF0048FF, 0xFF000000, 0xFFDADA00, 0xFFFFFFFF, 0xFFDAB66D, 0xFF242424, 0xFF00FF00, 0xFFDA6D00, 0xFF004848, 0xFF002491, 0xFFFF0091, 0xFF249100, 0xFF000000, 0xFF00FFFF, 0xFF914800, 0xFFFFFF00, 0xFFFFB6B6, 0xFFB6006D, 0xFF006D24, 0xFF919191 },
	/* 2C04-0003 */	{ 0xFFB600FF, 0xFFFF6DFF, 0xFF91FF6D, 0xFFB6B6B6, 0xFF009100, 0xFFFFFFFF, 0xFFB6DAFF, 0xFF244800, 0xFF002491, 0xFF000000, 0xFFFFDA91, 0xFF6D4800, 0xFFFF0091, 0xFFDADADA, 0xFFDAB66D, 0xFF91DAFF, 0xFF9191FF, 0xFF009191, 0xFFB6006D, 0xFF0048FF, 0xFF249100, 0xFF916D00, 0xFFDA6D00, 0xFF00B66D, 0xFF6D6D6D, 0xFF6D48DA, 0xFF000000, 0xFF0000DA, 0xFFFF0000, 0xFFB62400, 0xFFFF91FF, 0xFFFFB6B6, 0xFFDA6DFF, 0xFF004800, 0xFF00006D, 0xFFFFFF00, 0xFF242424, 0xFFFFB600, 0xFFFF9100, 0xFFFFFFFF, 0xFF6DDA00, 0xFF91006D, 0xFF6DB6FF, 0xFFFF00FF, 0xFF006DDA, 0xFF919191, 0xFF000000, 0xFF6D2400, 0xFF00FFFF, 0xFF480000, 0xFFB6FF48, 0xFFFFB6FF, 0xFF914800, 0xFF00FF00, 0xFFDADA00, 0xFF484848, 0xFF006D24, 0xFF000000, 0xFFDAB6FF, 0xFFFFFF6D, 0xFF9100FF, 0xFF48FFDA, 0xFFFFDA00, 0xFF004848 },
	/* 2C04-0004 */	{ 0xFF916D00, 0xFF6D48DA, 0xFF009191, 0xFFDADA00, 0xFF000000, 0xFFFFB6B6, 0xFF002491, 0xFFDA6D00, 0xFFB6B6B6, 0xFF6D2400, 0xFF00FF00, 0xFF00006D, 0xFFFFDA91, 0xFFFFFF00, 0xFF009100, 0xFFB6FF48, 0xFFFF6DFF, 0xFF480000, 0xFF0048FF, 0xFFFF91FF, 0xFF000000, 0xFF484848, 0xFFB62400, 0xFFFF9100, 0xFFDAB66D, 0xFF00B66D, 0xFF9191FF, 0xFF249100, 0xFF91006D, 0xFF000000, 0xFF91FF6D, 0xFF6DB6FF, 0xFFB6006D, 0xFF006D24, 0xFF914800, 0xFF0000DA, 0xFF9100FF, 0xFFB600FF, 0xFF6D6D6D, 0xFFFF0091, 0xFF004848, 0xFFDADADA, 0xFF006DDA, 0xFF004800, 0xFF242424, 0xFFFFFF6D, 0xFF919191, 0xFFFF00FF, 0xFFFFB6FF, 0xFFFFFFFF, 0xFF6D4800, 0xFFFF0000, 0xFFFFDA00, 0xFF48FFDA, 0xFFFFFFFF, 0xFF91DAFF, 0xFF000000, 0xFFFFB600, 0xFFDA6DFF, 0xFFB6DAFF, 0xFF6DDA00, 0xFFDAB6FF, 0xFF00FFFF, 0xFF244800 },
	/* 2C05-01 */		{ 0xFF6D6D6D, 0xFF002491, 0xFF0000DA, 0xFF6D48DA, 0xFF91006D, 0xFFB6006D, 0xFFB62400, 0xFF914800, 0xFF6D4800, 0xFF244800, 0xFF006D24, 0xFF009100, 0xFF004848, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFB6B6B6, 0xFF006DDA, 0xFF0048FF, 0xFF9100FF, 0xFFB600FF, 0xFFFF0091, 0xFFFF0000, 0xFFDA6D00, 0xFF916D00, 0xFF249100, 0xFF009100, 0xFF00B66D, 0xFF009191, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF6DB6FF, 0xFF9191FF, 0xFFDA6DFF, 0xFFFF00FF, 0xFFFF6DFF, 0xFFFF9100, 0xFFFFB600, 0xFFDADA00, 0xFF6DDA00, 0xFF00FF00, 0xFF48FFDA, 0xFF00FFFF, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFB6DAFF, 0xFFDAB6FF, 0xFFFFB6FF, 0xFFFF91FF, 0xFFFFB6B6, 0xFFFFDA91, 0xFFFFFF48, 0xFFFFFF6D, 0xFFB6FF48, 0xFF91FF6D, 0xFF48FFDA, 0xFF91DAFF, 0xFF000000, 0xFF000000, 0xFF000000 },
	/* 2C05-02 */		{ 0xFF6D6D6D, 0xFF002491, 0xFF0000DA, 0xFF6D48DA, 0xFF91006D, 0xFFB6006D, 0xFFB62400, 0xFF914800, 0xFF6D4800, 0xFF244800, 0xFF006D24, 0xFF009100, 0xFF004848, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFB6B6B6, 0xFF006DDA, 0xFF0048FF, 0xFF9100FF, 0xFFB600FF, 0xFFFF0091, 0xFFFF0000, 0xFFDA6D00, 0xFF916D00, 0xFF249100, 0xFF009100, 0xFF00B66D, 0xFF009191, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF6DB6FF, 0xFF9191FF, 0xFFDA6DFF, 0xFFFF00FF, 0xFFFF6DFF, 0xFFFF9100, 0xFFFFB600, 0xFFDADA00, 0xFF6DDA00, 0xFF00FF00, 0xFF48FFDA, 0xFF00FFFF, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFB6DAFF, 0xFFDAB6FF, 0xFFFFB6FF, 0xFFFF91FF, 0xFFFFB6B6, 0xFFFFDA91, 0xFFFFFF48, 0xFFFFFF6D, 0xFFB6FF48, 0xFF91FF6D, 0xFF48FFDA, 0xFF91DAFF, 0xFF000000, 0xFF000000, 0xFF000000 },
	/* 2C05-03 */		{ 0xFF6D6D6D, 0xFF002491, 0xFF0000DA, 0xFF6D48DA, 0xFF91006D, 0xFFB6006D, 0xFFB62400, 0xFF914800, 0xFF6D4800, 0xFF244800, 0xFF006D24, 0xFF009100, 0xFF004848, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFB6B6B6, 0xFF006DDA, 0xFF0048FF, 0xFF9100FF, 0xFFB600FF, 0xFFFF0091, 0xFFFF0000, 0xFFDA6D00, 0xFF916D00, 0xFF249100, 0xFF009100, 0xFF00B66D, 0xFF009191, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF6DB6FF, 0xFF9191FF, 0xFFDA6DFF, 0xFFFF00FF, 0xFFFF6DFF, 0xFFFF9100, 0xFFFFB600, 0xFFDADA00, 0xFF6DDA00, 0xFF00FF00, 0xFF48FFDA, 0xFF00FFFF, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFB6DAFF, 0xFFDAB6FF, 0xFFFFB6FF, 0xFFFF91FF, 0xFFFFB6B6, 0xFFFFDA91, 0xFFFFFF48, 0xFFFFFF6D, 0xFFB6FF48, 0xFF91FF6D, 0xFF48FFDA, 0xFF91DAFF, 0xFF000000, 0xFF000000, 0xFF000000 },
	/* 2C05-04 */		{ 0xFF6D6D6D, 0xFF002491, 0xFF0000DA, 0xFF6D48DA, 0xFF91006D, 0xFFB6006D, 0xFFB62400, 0xFF914800, 0xFF6D4800, 0xFF244800, 0xFF006D24, 0xFF009100, 0xFF004848, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFB6B6B6, 0xFF006DDA, 0xFF0048FF, 0xFF9100FF, 0xFFB600FF, 0xFFFF0091, 0xFFFF0000, 0xFFDA6D00, 0xFF916D00, 0xFF249100, 0xFF009100, 0xFF00B66D, 0xFF009191, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF6DB6FF, 0xFF9191FF, 0xFFDA6DFF, 0xFFFF00FF, 0xFFFF6DFF, 0xFFFF9100, 0xFFFFB600, 0xFFDADA00, 0xFF6DDA00, 0xFF00FF00, 0xFF48FFDA, 0xFF00FFFF, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFB6DAFF, 0xFFDAB6FF, 0xFFFFB6FF, 0xFFFF91FF, 0xFFFFB6B6, 0xFFFFDA91, 0xFFFFFF48, 0xFFFFFF6D, 0xFFB6FF48, 0xFF91FF6D, 0xFF48FFDA, 0xFF91DAFF, 0xFF000000, 0xFF000000, 0xFF000000 },
	/* 2C05-05 */		{ 0xFF6D6D6D, 0xFF002491, 0xFF0000DA, 0xFF6D48DA, 0xFF91006D, 0xFFB6006D, 0xFFB62400, 0xFF914800, 0xFF6D4800, 0xFF244800, 0xFF006D24, 0xFF009100, 0xFF004848, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFB6B6B6, 0xFF006DDA, 0xFF0048FF, 0xFF9100FF, 0xFFB600FF, 0xFFFF0091, 0xFFFF0000, 0xFFDA6D00, 0xFF916D00, 0xFF249100, 0xFF009100, 0xFF00B66D, 0xFF009191, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF6DB6FF, 0xFF9191FF, 0xFFDA6DFF, 0xFFFF00FF, 0xFFFF6DFF, 0xFFFF9100, 0xFFFFB600, 0xFFDADA00, 0xFF6DDA00, 0xFF00FF00, 0xFF48FFDA, 0xFF00FFFF, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFB6DAFF, 0xFFDAB6FF, 0xFFFFB6FF, 0xFFFF91FF, 0xFFFFB6B6, 0xFFFFDA91, 0xFFFFFF48, 0xFFFFFF6D, 0xFFB6FF48, 0xFF91FF6D, 0xFF48FFDA, 0xFF91DAFF, 0xFF000000, 0xFF000000, 0xFF000000 }
};

uint8_t EmulationSettings::_paletteLut[11][64] = {
	/* 2C02 */      { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63 },
	/* 2C03 */      { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,15,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,15,62,63 },
	/* 2C04-0001 */ { 53,35,22,34,28,9,29,21,32,0,39,5,4,40,8,32,33,62,31,41,60,50,54,18,63,43,46,30,61,45,36,1,14,49,51,42,44,12,27,20,46,7,52,6,19,2,38,46,46,25,16,10,57,3,55,23,15,17,11,13,56,37,24,58 },
	/* 2C04-0002 */ { 46,39,24,57,58,37,28,49,22,19,56,52,32,35,60,11,15,33,6,61,27,41,30,34,29,36,14,43,50,8,46,3,4,54,38,51,17,31,16,2,20,63,0,9,18,46,40,32,62,13,42,23,12,1,21,25,46,44,7,55,53,5,10,45 },
	/* 2C04-0003 */ { 20,37,58,16,11,32,49,9,1,46,54,8,21,61,62,60,34,28,5,18,25,24,23,27,0,3,46,2,22,6,52,53,35,15,14,55,13,39,38,32,41,4,33,36,17,45,46,31,44,30,57,51,7,42,40,29,10,46,50,56,19,43,63,12 },
	/* 2C04-0004 */ { 24,3,28,40,46,53,1,23,16,31,42,14,54,55,11,57,37,30,18,52,46,29,6,38,62,27,34,25,4,46,58,33,5,10,7,2,19,20,0,21,12,61,17,15,13,56,45,36,51,32,8,22,63,43,32,60,46,39,35,49,41,50,44,9 },
	/* 2C05-01 */   { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,15,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,15,62,63 },
	/* 2C05-02 */   { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,15,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,15,62,63 },
	/* 2C05-03 */   { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,15,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,15,62,63 },
	/* 2C05-04 */   { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,15,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,15,62,63 },
	/* 2C05-05 */   { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,15,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,15,62,63 },
};

uint32_t EmulationSettings::GetEmulationSpeed(bool ignoreTurbo)
{
	if(ignoreTurbo) {
		return _emulationSpeed;
	} else if(CheckFlag(EmulationFlags::ForceMaxSpeed)) {
		return 0;
	} else if(CheckFlag(EmulationFlags::Turbo)) {
		return _turboSpeed;
	} else if(CheckFlag(EmulationFlags::Rewind)) {
		return _rewindSpeed;
	} else {
		return _emulationSpeed;
	}
}

double EmulationSettings::GetAspectRatio(shared_ptr<Console> console)
{
	switch(_aspectRatio) {
		case VideoAspectRatio::NoStretching: return 0.0;

		case VideoAspectRatio::Auto:
		{
			NesModel model = GetNesModel();
			if(model == NesModel::Auto) {
				model = console->GetModel();
			}
			return (model == NesModel::PAL || model == NesModel::Dendy) ? (9440000.0 / 6384411.0) : (128.0 / 105.0);
		}

		case VideoAspectRatio::NTSC: return 128.0 / 105.0;
		case VideoAspectRatio::PAL: return 9440000.0 / 6384411.0;
		case VideoAspectRatio::Standard: return 4.0 / 3.0;
		case VideoAspectRatio::Widescreen: return 16.0 / 9.0;
		case VideoAspectRatio::Custom: return _customAspectRatio;
	}
	return 0.0;
}

const vector<string> NesModelNames = {
	"Auto",
	"NTSC",
	"PAL",
	"Dendy"
};

const vector<string> ConsoleTypeNames = {
	"Nes",
	"Famicom",
};

const vector<string> ControllerTypeNames = {
	"None",
	"StandardController",
	"Zapper",
	"ArkanoidController",
	"SnesController",
	"PowerPad",
	"SnesMouse",
	"SuborMouse",
	"VsZapper"
};

const vector<string> ExpansionPortDeviceNames = {
	"None",
	"Zapper",
	"FourPlayerAdapter",
	"ArkanoidController",
	"OekaKidsTablet",
	"FamilyTrainerMat",
	"KonamiHyperShot",
	"FamilyBasicKeyboard",
	"PartyTap",
	"Pachinko",
	"ExcitingBoxing",
	"JissenMahjong",
	"SuborKeyboard",
	"BarcodeBattler",
	"HoriTrack",
	"BandaiHyperShot",
	"AsciiTurboFile",
	"BattleBox",
};

const vector<string> PpuModelNames = {
	"Ppu2C02",
	"Ppu2C03",
	"Ppu2C04A",
	"Ppu2C04B",
	"Ppu2C04C",
	"Ppu2C04D",
	"Ppu2C05A",
	"Ppu2C05B",
	"Ppu2C05C",
	"Ppu2C05D",
	"Ppu2C05E"
};