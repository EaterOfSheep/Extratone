#include "plugin.hpp"


struct Darwinism : Module {
	enum ParamIds {
		SAVE_PARAM,
		LOAD_PARAM,
		VIEWMODE_PARAM,
		MUTATIONRATE_PARAM,
		MUTATE_PARAM,
		STEP_PARAM,
		STEPS_PARAM,
		RANDOM_PARAM,
		DENSITY_PARAM,
		RESET_PARAM,
		ZERO_PARAM,
		ONOFF_PARAM,
		NOTE_PARAM=ONOFF_PARAM+16,
		SAVEDONOFF_PARAM=NOTE_PARAM+16,
		SAVEDNOTE_PARAM=SAVEDONOFF_PARAM+16,
		NUM_PARAMS=SAVEDNOTE_PARAM+16
	};
	enum InputIds {
		SAVE_INPUT,
		LOAD_INPUT,
		ZERO_INPUT,
		RESET_INPUT,
		RANDOM_INPUT,
		MUTATE_INPUT,
		CLOCK_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		STEP_OUTPUT,
		EOP_OUTPUT,
		NOTE_OUTPUT,
		GATE_OUTPUT,
		TRIG_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		STEP_LIGHT,
		NUM_LIGHTS=STEP_LIGHT+16
	};

	float range=1.f;
	int step= 0;

	dsp::PulseGenerator eopPulse;	
	dsp::PulseGenerator trigPulse;	

	dsp::SchmittTrigger randomTrigger;
	dsp::SchmittTrigger clearTrigger;
	dsp::SchmittTrigger stepTrigger;
	dsp::SchmittTrigger saveTrigger;
	dsp::SchmittTrigger loadTrigger;
	dsp::SchmittTrigger onoffTrigger[16];
	dsp::SchmittTrigger mutateTrigger;
	dsp::SchmittTrigger resetTrigger;
	//Add range knob and revert (or save/load?) trigger
	//
	//SAVE/LOAD = Survive/Fossil

	Darwinism() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configParam(STEPS_PARAM, 2, 16, 16, "");
		configParam(VIEWMODE_PARAM, 0, 2, 0, "");

		configParam(MUTATIONRATE_PARAM, 0.f, 1.f, 0.5, "");
		configParam(DENSITY_PARAM, 0.f, 1.f, 0.5, "");
	}

	int viewmode = 0;


	void randomiseStep(int i){
		float density = params[DENSITY_PARAM].getValue();
		params[NOTE_PARAM + i].setValue(random::uniform());
		params[ONOFF_PARAM +i].setValue(random::uniform()<density);


	}

	void process(const ProcessArgs& args) override {

		viewmode=params[VIEWMODE_PARAM].getValue();

		if(resetTrigger.process(params[RESET_PARAM].getValue()+inputs[RESET_INPUT].getVoltage())){
			step=0;
		}

		if(stepTrigger.process(inputs[CLOCK_INPUT].getVoltage())){
			if(step<params[STEPS_PARAM].getValue()-1){step++;}else{step=0; eopPulse.trigger();}
			if(params[ONOFF_PARAM +step].getValue()){trigPulse.trigger();}
		}

		if(saveTrigger.process(params[SAVE_PARAM].getValue()+inputs[SAVE_INPUT].getVoltage())){
			
			for(int i = 0; i<16;i++){
				params[SAVEDNOTE_PARAM + i].setValue(params[NOTE_PARAM +i].getValue());
				params[SAVEDONOFF_PARAM +i].setValue(params[ONOFF_PARAM +i].getValue());
			}
		}


		if(loadTrigger.process(params[LOAD_PARAM].getValue()+inputs[LOAD_INPUT].getVoltage())){
			
			for(int i = 0; i<16;i++){
				params[NOTE_PARAM + i].setValue(params[SAVEDNOTE_PARAM +i].getValue());
				params[ONOFF_PARAM +i].setValue(params[SAVEDONOFF_PARAM +i].getValue());
			}
		}

		if(randomTrigger.process(params[RANDOM_PARAM].getValue()+inputs[RANDOM_INPUT].getVoltage())){
				
			for (int i = 0; i < 16; i++) {

				randomiseStep(i);
			}

		}


		if(mutateTrigger.process(params[MUTATE_PARAM].getValue()+inputs[MUTATE_INPUT].getVoltage())){	
			float mutationrate=params[MUTATIONRATE_PARAM].getValue();
			for (int i = 0; i < 16; i++) {
				if(random::uniform()<mutationrate){
					randomiseStep(i);
				}
			}
		}

		if(clearTrigger.process(params[ZERO_PARAM].getValue()+inputs[ZERO_INPUT].getVoltage()  )){
			
			for (int i = 0; i < 16; i++) {
				params[NOTE_PARAM + i].setValue(0.f);
				params[ONOFF_PARAM + i].setValue(0.f);
			}

		}

		for (int i = 0; i < 16; i++) {

			if(viewmode==0){
				lights[STEP_LIGHT + i].setBrightness((i==step));
			}
			if(viewmode==1){
				lights[STEP_LIGHT + i].setBrightness(params[ONOFF_PARAM+i].getValue());
			}
			if(viewmode==2){
				lights[STEP_LIGHT + i].setBrightness(params[NOTE_PARAM+i].getValue());
			}
			
		}

		outputs[GATE_OUTPUT].setVoltage(10.f*params[ONOFF_PARAM+step].getValue());
		outputs[NOTE_OUTPUT].setVoltage(range*params[NOTE_PARAM+step].getValue());
		outputs[STEP_OUTPUT].setVoltage(10.f*step/(params[STEPS_PARAM].getValue()-1.f));
		outputs[EOP_OUTPUT].setVoltage((eopPulse.process(args.sampleTime) ? 10.0f : 0.0f));
		outputs[TRIG_OUTPUT].setVoltage((trigPulse.process(args.sampleTime) ? 10.0f : 0.0f));
}
};


struct DarwinismWidget : ModuleWidget {
	DarwinismWidget(Darwinism* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Darwinism.svg")));

		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(25,115)), module, Darwinism::VIEWMODE_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(mm2px(Vec(40,73)), module, Darwinism::STEPS_PARAM));

		addParam(createParam<TL1105>(mm2px(Vec(26,45)), module, Darwinism::ZERO_PARAM));
		addParam(createParam<TL1105>(mm2px(Vec(26,55)), module, Darwinism::RANDOM_PARAM));
		addParam(createParam<TL1105>(mm2px(Vec(26,65)), module, Darwinism::MUTATE_PARAM));
		addParam(createParam<TL1105>(mm2px(Vec(26,75)), module, Darwinism::RESET_PARAM));
		addParam(createParam<TL1105>(mm2px(Vec(54,45)), module, Darwinism::SAVE_PARAM));
		addParam(createParam<TL1105>(mm2px(Vec(54,55)), module, Darwinism::LOAD_PARAM));


		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 48)), module, Darwinism::ZERO_INPUT));	
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 58)), module, Darwinism::RANDOM_INPUT));	
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 68)), module, Darwinism::MUTATE_INPUT));	
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 78)), module, Darwinism::RESET_INPUT));	
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(49, 48)), module, Darwinism::SAVE_INPUT));	
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(49, 58)), module, Darwinism::LOAD_INPUT));	


		addParam(createParam<RoundBlackKnob>(mm2px(Vec(21,101)), module, Darwinism::DENSITY_PARAM));
		addParam(createParam<RoundBlackKnob>(mm2px(Vec(21,87)), module, Darwinism::MUTATIONRATE_PARAM));
		//addParam(createParam<TL1105>(mm2px(Vec(25,100)), module, Darwinism::STEP_PARAM));


		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(38.5, 23.25)), module, Darwinism::TRIG_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(28.25, 23.25)), module, Darwinism::GATE_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(48.75, 23.25)), module, Darwinism::NOTE_OUTPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(28.25, 38.25)), module, Darwinism::CLOCK_INPUT));	
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(38.5, 38.25)), module, Darwinism::STEP_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(48.75, 38.25)), module, Darwinism::EOP_OUTPUT));

		int id=0;
		for(int y = 0;y<4;y++){
			for(int x=0;x<4;x++){
				
				addChild(createLight<MediumLight<GreenLight>>(mm2px(Vec(3+5*x, 16+7*y)), module, Darwinism::STEP_LIGHT+id));

				id++;
			}
		}

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	}
};


Model* modelDarwinism = createModel<Darwinism, DarwinismWidget>("Darwinism");
