#include "plugin.hpp"


struct Mesohyl : Module {

//To add:
//And and Or modes for Global SandH
//Lights for un-updated knobs
//Unupdated-count output (+1V for each unupdated knob)
//Range knob, bipolar switch.


	enum ParamIds {
		ANDOR_PARAM,
		RANGE_PARAM,
		BIPOLAR_PARAM,
		KNOB_PARAM,
		NUM_PARAMS=KNOB_PARAM+6
	};
	enum InputIds {
		GSANDH_INPUT,
		KSANDH_INPUT,
		NUM_INPUTS=KSANDH_INPUT+6
	};
	enum OutputIds {
		KOUT_OUTPUT,
		NUM_OUTPUTS=KOUT_OUTPUT+6
	};
	enum LightIds {
		CHANGE_LIGHT,
		NUM_LIGHTS=CHANGE_LIGHT+6
	};

	float output[6] = { 0.f };

	Mesohyl() {

		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configParam(RANGE_PARAM, 0.f, 10.f, 1.f, "");

		for(int i = 0; i<6; i++){
			configParam(KNOB_PARAM+i, 0.f, 1.f, 0.5, "");
		}

	}

	void process(const ProcessArgs& args) override {

		for(int i = 0; i<6; i++){

			float knob = params[KNOB_PARAM+i].getValue();
			float range = params[RANGE_PARAM].getValue();

			if(params[BIPOLAR_PARAM].getValue()){
				knob = (knob-0.5)*2.f;
			}

			knob = knob*range;


			if(isUpdating(i)){
				output[i] = knob;
			}
			outputs[KOUT_OUTPUT + i].setVoltage(output[i]);
			lights[CHANGE_LIGHT+i].setBrightness(output[i]!=knob);
		}

	}

	bool isUpdating(int i){

		bool gtrue = false;
		bool strue = false;

		bool gplug = true;
		bool splug = true;

		if(!inputs[GSANDH_INPUT].isConnected()){gtrue=true; gplug=false;}else{gtrue = inputs[GSANDH_INPUT].getVoltage()>=1;}
		if(!inputs[KSANDH_INPUT+i].isConnected()){strue=true; splug=false;}else{strue = inputs[KSANDH_INPUT+i].getVoltage()>=1;}

		if(params[ANDOR_PARAM].getValue()){ //AND MODE

			return (gtrue&&strue);


		}else{ //OR MODE

			if(!splug && !gplug){
				return true;
			}

			if(splug && !gplug){
				return strue;
			}

			if(!splug && gplug){
				return gtrue;
			}

			return (gtrue||strue);

		}

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

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15, 22)), module, Mesohyl::GSANDH_INPUT));

		addParam(createParamCentered<CKSS>(mm2px(Vec(25, 22)), module, Mesohyl::ANDOR_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(27, 115)), module, Mesohyl::BIPOLAR_PARAM));
		addParam(createParamCentered<XtrtnKnob>(mm2px(Vec(18,115)), module, Mesohyl::RANGE_PARAM));


		int ygap = 12;
		int ystart = 42;
		for(int i = 0; i<6; i++){

			addChild(createLightCentered<SmallLight<XtrtnPinkLight>>(mm2px(Vec(14, ystart+i*ygap-4)), module, Mesohyl::CHANGE_LIGHT+i));

			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, ystart+i*ygap)), module, Mesohyl::KSANDH_INPUT+i));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32, ystart+i*ygap)), module, Mesohyl::KOUT_OUTPUT+i));
			addParam(createParamCentered<XtrtnKnob>(mm2px(Vec(20,ystart+i*ygap)), module, Mesohyl::KNOB_PARAM+i));
		}



	}
};

Model* modelMesohyl = createModel<Mesohyl, MesohylWidget>("Mesohyl");
