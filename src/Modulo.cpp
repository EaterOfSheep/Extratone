#include "plugin.hpp"


struct Modulo : Module {
	enum ParamIds {
		MODULO_PARAM,
		MODULO2_PARAM,
		OFF_PARAM,
		OFF2_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		PITCH_INPUT,
		PITCH2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		MODULO_OUTPUT,
		MODULO2_OUTPUT,
		ABS_OUTPUT,
		ABS2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
	
	float prevpitch=0.f;
	float prevpitch2=0.f;
	float prevmod=1.f;
	float prevmod2=1.f;
	float prevoff=0.f;
	float prevoff2=0.f;

	Modulo() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(MODULO_PARAM, 0.1, 10.f, 1.f, "");
		configParam(MODULO2_PARAM, 0.1, 10.f, 1.f, "");
		configParam(OFF_PARAM, -1.f, 1.f, 0.f, "");
		configParam(OFF2_PARAM, -1.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
	
	float pitch = inputs[PITCH_INPUT].getVoltage();
	float pitchpos = ((pitch > 0) - (pitch < 0))*pitch;
	float mod = params[MODULO_PARAM].getValue();
	float off = params[OFF_PARAM].getValue()*mod;
	
	if(prevpitch!=pitch||prevmod!=mod||prevoff!=off){
	prevmod=mod;
	prevpitch=pitch;
	prevoff=off;
	while(pitchpos>mod+off){
	pitchpos=pitchpos-mod;
	}
	pitch = ((pitch > 0) - (pitch < 0))*pitchpos;
	outputs[MODULO_OUTPUT].setVoltage(pitch);
	outputs[ABS_OUTPUT].setVoltage(pitchpos);
	}
	
	pitch = inputs[PITCH2_INPUT].getVoltage();
	pitchpos = ((pitch > 0) - (pitch < 0))*pitch;
	mod = params[MODULO2_PARAM].getValue();
	off = params[OFF2_PARAM].getValue()*mod;
	
	if(prevpitch2!=pitch||prevmod2!=mod||prevoff2!=off){
	prevmod2=mod;
	prevpitch2=pitch;
	prevoff2=off;
	while(pitchpos>mod+off){
		pitchpos=pitchpos-mod;
	}
	pitch = ((pitch > 0) - (pitch < 0))*pitchpos;
	outputs[MODULO2_OUTPUT].setVoltage(pitch);
	outputs[ABS2_OUTPUT].setVoltage(pitchpos);
	}
	
	}
};


struct ModuloWidget : ModuleWidget {
	ModuloWidget(Modulo* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Modulo.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24-7, 36)), module, Modulo::MODULO_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24-7, 50+36)), module, Modulo::MODULO2_PARAM));
		
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24+7, 36)), module, Modulo::OFF_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24+7, 50+36)), module, Modulo::OFF2_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 49)), module, Modulo::PITCH_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 50+49)), module, Modulo::PITCH2_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24-7, 64)), module, Modulo::MODULO_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24-7, 50+64)), module, Modulo::MODULO2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24+7, 64)), module, Modulo::ABS_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24+7, 50+64)), module, Modulo::ABS2_OUTPUT));

	}
};


Model* modelModulo = createModel<Modulo, ModuloWidget>("Modulo");