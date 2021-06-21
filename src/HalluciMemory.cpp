#include "plugin.hpp"


struct HalluciMemory : Module {
	enum ParamIds {
		SAVE_PARAM,
		LOAD_PARAM,
		ZERO_PARAM,
		RANDOM_PARAM,
		ONOFF_PARAM,
		NOTE_PARAM=ONOFF_PARAM+16,
		NUM_PARAMS=NOTE_PARAM+16
	};
	enum InputIds {
		SAVE_INPUT,
		LOAD_INPUT,
		ZERO_INPUT,
		RANDOM_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		CONNECT_LIGHT,
		NUM_LIGHTS
	};

	dsp::SchmittTrigger randomTrigger;
	dsp::SchmittTrigger zeroTrigger;
	dsp::SchmittTrigger saveTrigger;
	dsp::SchmittTrigger loadTrigger;

	float leftMessages[2][32]={};


	HalluciMemory() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		leftExpander.producerMessage=leftMessages[0];

		leftExpander.consumerMessage=leftMessages[1];
	}


	void process(const ProcessArgs& args) override {
		
		bool connected = leftExpander.module 
			&& (leftExpander.module->model == modelHalluciMemory || leftExpander.module->model == modelDarwinism);

		lights[CONNECT_LIGHT].setBrightness(connected);

		if(randomTrigger.process(params[RANDOM_PARAM].getValue()+inputs[RANDOM_INPUT].getVoltage())){
			for (int i = 0; i < 16; i++) {
				randomiseStep(i);
			}
		}

		if(zeroTrigger.process(params[ZERO_PARAM].getValue()+inputs[ZERO_INPUT].getVoltage()  )){
			for (int i = 0; i < 16; i++) {
				params[NOTE_PARAM + i].setValue(0.f);
				params[ONOFF_PARAM + i].setValue(0.f);
			}
		}


		if(connected){

			float *messageFromMother = (float*) leftExpander.consumerMessage;
			float *messageToSendToMother = (float*) leftExpander.module->rightExpander.producerMessage;


			if(saveTrigger.process(params[SAVE_PARAM].getValue()+inputs[SAVE_INPUT].getVoltage())){
				for(int i = 0; i<16;i++){

					params[NOTE_PARAM +i].setValue(messageFromMother[i]);
					params[ONOFF_PARAM +i].setValue(messageFromMother[16+i]);

				}
			}

			if(loadTrigger.process(params[SAVE_PARAM].getValue()+inputs[SAVE_INPUT].getVoltage())){
				for(int i = 0; i<16;i++){

					messageToSendToMother[i]=params[NOTE_PARAM+i].getValue();
					messageToSendToMother[16+i]=params[NOTE_PARAM+i].getValue();

				//params[ONOFF_PARAM +i].getValue();
				}
			}
		}
	}



	void randomiseStep(int i){
		float density = 0.5;//params[DENSITY_PARAM].getValue();
		params[NOTE_PARAM + i].setValue(random::uniform());
		params[ONOFF_PARAM +i].setValue(random::uniform()<density);
	}

	
};


struct HalluciMemoryWidget : ModuleWidget {
	HalluciMemoryWidget(HalluciMemory* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/HalluciMemory.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		//addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		//addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addChild(createLightCentered<MediumLight<XtrtnPinkLight>>(mm2px(Vec(10, 100)), module, HalluciMemory::CONNECT_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6, 28)), module, HalluciMemory::SAVE_INPUT));	
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6, 43)), module, HalluciMemory::LOAD_INPUT));	
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6, 63)), module, HalluciMemory::ZERO_INPUT));	
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6, 78)), module, HalluciMemory::RANDOM_INPUT));	


		addParam(createParamCentered<TL1105>(mm2px(Vec(16,28)), module, HalluciMemory::SAVE_PARAM));
		addParam(createParamCentered<TL1105>(mm2px(Vec(16,43)), module, HalluciMemory::LOAD_PARAM));
		addParam(createParamCentered<TL1105>(mm2px(Vec(16,63)), module, HalluciMemory::ZERO_PARAM));
		addParam(createParamCentered<TL1105>(mm2px(Vec(16,78)), module, HalluciMemory::RANDOM_PARAM));
	}
};


Model* modelHalluciMemory = createModel<HalluciMemory, HalluciMemoryWidget>("HalluciMemory");
