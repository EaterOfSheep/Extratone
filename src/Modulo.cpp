#include "plugin.hpp"


struct Modulo : Module {
	enum ParamIds {
		MODULO_PARAM,
		OFF_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		PITCH_INPUT,
		OFF_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		MODULO_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Modulo() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(MODULO_PARAM, 1, 5, 1, "");
		configParam(OFF_PARAM, -1.f, 1.f, 0.f, "");
		configBypass(PITCH_INPUT,MODULO_OUTPUT);

	}

	void process(const ProcessArgs& args) override {

		float pitch[16];
		float mod = params[MODULO_PARAM].getValue();
		float off = params[OFF_PARAM].getValue()*mod;

		if(inputs[OFF_INPUT].isConnected()){
			off = inputs[OFF_INPUT].getVoltage()/5;
		}

		outputs[MODULO_OUTPUT].channels=inputs[PITCH_INPUT].channels;

		for(int i = 0; i<16; i++){

			pitch[i]=inputs[PITCH_INPUT].getVoltage(i);

			float pitchpos = ((pitch[i] > 0) - (pitch[i] < 0))*pitch[i];

			while(pitchpos>mod+off){
				pitchpos=pitchpos-mod;
			}

			pitch[i] = ((pitch[i] > 0) - (pitch[i] < 0))*pitchpos;
			outputs[MODULO_OUTPUT].setVoltage(pitch[i],i);

		}

	}

};


struct ModuloWidget : ModuleWidget {
	ModuloWidget(Modulo* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Modulo.svg")));

		addChild(createWidget<XtrtnScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<XtrtnScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<XtrtnSnapKnob>(mm2px(Vec(10, 46)), module, Modulo::MODULO_PARAM));

		addParam(createParamCentered<XtrtnMedKnob>(mm2px(Vec(10, 67)), module, Modulo::OFF_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 77)), module, Modulo::OFF_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 25)), module, Modulo::PITCH_INPUT));


		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10, 98)), module, Modulo::MODULO_OUTPUT));

	}
};

Model* modelModulo = createModel<Modulo, ModuloWidget>("Modulo");
