#include "plugin.hpp"


struct Mesoglea2 : Module {

//To add:
//And and Or modes for Global SandH
//Lights for un-updated knobs
//Unupdated-count output (+1V for each unupdated knob)
//Range knob, bipolar switch.


	enum ParamIds {
		ANDOR_PARAM,
		FLIPBUTTON_PARAM,
		NUM_PARAMS=FLIPBUTTON_PARAM+5
	};
	enum InputIds {
		GSANDH_INPUT,
		TOMUTE_INPUT,
		KSANDH_INPUT=TOMUTE_INPUT+5,
		NUM_INPUTS=KSANDH_INPUT+5
	};
	enum OutputIds {
		KOUT_OUTPUT,
		NUM_OUTPUTS=KOUT_OUTPUT+5
	};
	enum LightIds {
		GATE_LIGHT,
		CHANGE_LIGHT=GATE_LIGHT+5,
		NUM_LIGHTS=CHANGE_LIGHT+5
	};

	bool gate[5] = {false};
	float output[5] = { 0.f };
	dsp::SchmittTrigger flipTrigger[5];

	Mesoglea2() {

		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);


		for(int i = 0; i<5; i++){
			//configParam(KNOB_PARAM+i, 0.f, 1.f, 0.5, "");
		}

	}

	void process(const ProcessArgs& args) override {

		for(int i = 0; i<5; i++){

			if(flipTrigger[i].process(params[FLIPBUTTON_PARAM+i].getValue())){
				gate[i] = !gate[i];
			}

			if(isUpdating(i)){
				output[i] = gate[i];

				lights[GATE_LIGHT+i].setBrightness(gate[i]);
			}

			if(inputs[TOMUTE_INPUT+i].isConnected()){
				outputs[KOUT_OUTPUT + i].setVoltage(output[i]*inputs[TOMUTE_INPUT+i].getVoltage());
			}else{
				outputs[KOUT_OUTPUT + i].setVoltage(output[i]*10.f);
			}

			lights[CHANGE_LIGHT+i].setBrightness(output[i]!=gate[i]);
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


struct Mesoglea2Widget : ModuleWidget {
	Mesoglea2Widget(Mesoglea2* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Mesoglea2.svg")));

		addChild(createWidget<XtrtnScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<XtrtnScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<XtrtnScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<XtrtnScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15, 22)), module, Mesoglea2::GSANDH_INPUT));

		addParam(createParamCentered<CKSS>(mm2px(Vec(25, 22)), module, Mesoglea2::ANDOR_PARAM));


		int ygap = 16;
		int ystart = 42;
		for(int i = 0; i<5; i++){

			addChild(createLightCentered<SmallLight<XtrtnPinkLight>>(mm2px(Vec(23, ystart+i*ygap+7)), module, Mesoglea2::CHANGE_LIGHT+i));

			addChild(createLightCentered<SmallLight<XtrtnBlueLight>>(mm2px(Vec(19, ystart+i*ygap+7)), module, Mesoglea2::GATE_LIGHT+i));

			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, ystart+i*ygap)), module, Mesoglea2::KSANDH_INPUT+i));

			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, ystart+i*ygap)), module, Mesoglea2::TOMUTE_INPUT+i));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32, ystart+i*ygap)), module, Mesoglea2::KOUT_OUTPUT+i));

			addParam(createParamCentered<TL1105>(mm2px(Vec(14,ystart+i*ygap+6)), module, Mesoglea2::FLIPBUTTON_PARAM+i));
		}



	}
};

Model* modelMesoglea2 = createModel<Mesoglea2, Mesoglea2Widget>("Mesoglea2");
