#pragma once
#include <rack.hpp>


using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;


struct XtrtnSmallKnob : app::SvgKnob {
    XtrtnSmallKnob() {
        minAngle = -0.83 * M_PI;
        maxAngle = 0.83 * M_PI;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/XtrtnSmallKnob.svg")));
    }
};

struct XtrtnMedKnob : app::SvgKnob {
    XtrtnMedKnob() {
        minAngle = -0.83 * M_PI;
        maxAngle = 0.83 * M_PI;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/XtrtnMedKnob.svg")));
    }
};


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

struct XtrtnPinkLight : ModuleLightWidget {
	XtrtnPinkLight() {
		firstLightId = 1;
		this->bgColor = nvgRGB(40, 40, 40);
		addBaseColor(nvgRGB(255, 25, 150));
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
extern Model *modelIchneumonid;

// Declare each Model, defined in each module source file
// extern Model* modelMyModule;
