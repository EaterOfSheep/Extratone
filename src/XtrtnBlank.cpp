#include "plugin.hpp"


struct XtrtnBlank : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	XtrtnBlank() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	}

	void process(const ProcessArgs& args) override {
	}

};


struct XtrtnBlankWidget : ModuleWidget {
	XtrtnBlankWidget(XtrtnBlank* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/XtrtnBlank.svg")));

		addChild(createWidget<XtrtnScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<XtrtnScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	}
};

Model* modelXtrtnBlank = createModel<XtrtnBlank, XtrtnBlankWidget>("XtrtnBlank");
