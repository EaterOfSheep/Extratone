#pragma once
#include <rack.hpp>


using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;




struct XtrtnKnob : app::SvgKnob {
    XtrtnKnob() {
        minAngle = -0.83 * M_PI;
        maxAngle = 0.83 * M_PI;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/XtrtnKnob.svg")));
    }
};

struct XtrtnSnapKnob : XtrtnKnob {
    XtrtnSnapKnob() {
        snap = true;
    }
};

struct XtrtnBlueLight : ModuleLightWidget {
	XtrtnBlueLight() {
		firstLightId = 1;
		this->bgColor = nvgRGB(40, 40, 40);
		addBaseColor(nvgRGB(25, 150, 255));
	}
};

extern Model *modelModulo;
extern Model *modelOpabinia;
extern Model *modelSplitterburst;
extern Model *modelPuzzlebox;
extern Model *modelDarwinism;

// Declare each Model, defined in each module source file
// extern Model* modelMyModule;
