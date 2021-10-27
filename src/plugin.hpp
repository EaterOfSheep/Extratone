#pragma once
#include <rack.hpp>


using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;


struct XtrtnScrew : app::SvgScrew
{
	widget::TransformWidget *tw;

	XtrtnScrew()
	{
		fb->removeChild(sw);

		tw = new TransformWidget();
		tw->addChild(sw);
		fb->addChild(tw);

		bool pink = (random::uniform()>0.5); //random uniform is between 0 and 1
		if(pink){
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/screw2.svg")));
		}else{
			setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/screw.svg")));
		}

		tw->box.size = sw->box.size;
		box.size = tw->box.size;


		// Random rotation screw code by stoermelder (found on cvly plugin)
		float angle = random::uniform() * M_PI;
		tw->identity();
		// Rotate SVG
		math::Vec center = sw->box.getCenter();
		tw->translate(center);
		tw->rotate(angle);
		tw->translate(center.neg());
	}
};

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
//extern Model *modelHalluciMemory;
extern Model *modelIchneumonid;
extern Model *modelMeganeura;
extern Model *modelPureneura;
extern Model *modelMesoglea;
extern Model *modelMesoglea2;
extern Model *modelMesohyl;


extern Model *modelXtrtnBlank;

// Declare each Model, defined in each module source file
// extern Model* modelMyModule;
