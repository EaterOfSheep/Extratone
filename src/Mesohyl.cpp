#include "plugin.hpp"


struct Mesohyl : Module {

//To add:
//And and Or modes for Global SandH
//Lights for un-updated knobs
//Unupdated-count output (+1V for each unupdated knob)


	enum ParamIds {
		KNOB_PARAM,
		NUM_PARAMS=KNOB_PARAM+5
	};
	enum InputIds {
		GSANDH_INPUT,
		KSANDH_INPUT,
		NUM_INPUTS=KSANDH_INPUT+5
	};
	enum OutputIds {
		KOUT_OUTPUT,
		NUM_OUTPUTS=KOUT_OUTPUT+5
	};
	enum LightIds {
		CHANGE_LIGHT,
		NUM_LIGHTS=CHANGE_LIGHT+5
	};


	dsp::SchmittTrigger globalTrigger;
	dsp::SchmittTrigger knobTrigger[5];

	Mesohyl() {

		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	}

	void process(const ProcessArgs& args) override {

	}

};


struct MesohylWidget : ModuleWidget {
	MesohylWidget(Mesohyl* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Mesohyl.svg")));

		addChild(createWidget<XtrtnScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<XtrtnScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<XtrtnScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<XtrtnScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 22)), module, Mesohyl::GSANDH_INPUT));


		int ygap = 12;
		int ystart = 42;
		for(int i = 0; i<5; i++){


			addChild(createLightCentered<SmallLight<XtrtnPinkLight>>(mm2px(Vec(14, ystart+i*ygap-4)), module, Mesohyl::CHANGE_LIGHT+i));

			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, ystart+i*ygap)), module, Mesohyl::KSANDH_INPUT+i));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32, ystart+i*ygap)), module, Mesohyl::KOUT_OUTPUT+i));
			addParam(createParamCentered<XtrtnKnob>(mm2px(Vec(20,ystart+i*ygap)), module, Mesohyl::KNOB_PARAM+i));
		}



	}
};

Model* modelMesohyl = createModel<Mesohyl, MesohylWidget>("Mesohyl");
