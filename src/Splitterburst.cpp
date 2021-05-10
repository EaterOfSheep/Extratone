#include "plugin.hpp"


struct Splitterburst : Module {
	enum ParamIds {
		STEPS_PARAM,
		STEPS_ATT,
		MULTI_ATT,
		MULTI_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		MULTI_CV,
		STEPS_CV,
		GATE_INPUT,
		TOG_INPUT,
		OFF_INPUT,
		TRIG_INPUT,
		STEP_INPUT,
		CLOCK_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		STEP_OUTPUT,
		PLAIN_OUTPUT,
		GATE_OUTPUT,
		SOLO_OUTPUT,
		CLOCK_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ON_LIGHT,
		NUM_LIGHTS
	};

	Splitterburst() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		
		configParam(STEPS_PARAM, 1.f, 64.f, 4.f, "");
		configParam(MULTI_PARAM, 1.f, 64.f, 2.f, "");
		configParam(MULTI_ATT, -1.f, 1.f, 0.f, "");
		configParam(STEPS_ATT, -1.f, 1.f, 0.f, "");
	}

	dsp::PulseGenerator plainPulse;	
	dsp::PulseGenerator clockPulse;
	dsp::PulseGenerator soloPulse;
	
	dsp::SchmittTrigger clockTrigger;
	dsp::SchmittTrigger stepTrigger;
	dsp::SchmittTrigger trigTrigger;
	dsp::SchmittTrigger togTrigger;
	dsp::SchmittTrigger offTrigger;
	
	float timer=0;
	float wait=0;
	float goal=0;
	float step = 0;
	bool trigon = false;
	bool gateon = false;

	void process(const ProcessArgs& args) override {
	
	//float maxwait=60*args.sampleTime;
	
	if(trigTrigger.process(inputs[TRIG_INPUT].getVoltage())){
		step=0;
		trigon=true;
	}
	
	if(offTrigger.process(inputs[OFF_INPUT].getVoltage())){
		step=0;
		trigon=false;
	}
	
	if(togTrigger.process(inputs[TOG_INPUT].getVoltage())){
		step=0;
		trigon=(!trigon);
	}
	
	gateon=(inputs[GATE_INPUT].getVoltage()>=1.f);
	
	bool on = (gateon||trigon);
	
	float steps = round(clamp(params[STEPS_PARAM].getValue()+inputs[STEPS_CV].getVoltage()*6.4*params[STEPS_ATT].getValue(),1.f,64.f));
	int multiply = round(clamp(params[MULTI_PARAM].getValue()+inputs[MULTI_CV].getVoltage()*6.4*params[MULTI_ATT].getValue(),1.f,64.f));
	
	wait++;
	timer++;
	
	if(goal!=0){
		if(wait>=goal/multiply&&timer<=goal){
			wait=0;
			if(on){clockPulse.trigger();}
			if(on){soloPulse.trigger();}
		}
	}
	
	if(clockTrigger.process(inputs[CLOCK_INPUT].getVoltage())){
		goal=timer;
		timer=0;
		wait=0;
		clockPulse.trigger();
		if(on){soloPulse.trigger();}else{plainPulse.trigger();}
	}
	
	if(stepTrigger.process(inputs[STEP_INPUT].getVoltage())){
		if(trigon){
			step++;
			if(step>=steps){step=0;trigon=false;}
		}
	}
	

	
	lights[ON_LIGHT].value=(on);
	
	outputs[GATE_OUTPUT].setVoltage(10*on);
	outputs[PLAIN_OUTPUT].setVoltage((plainPulse.process(args.sampleTime) ? 10.0f : 0.0f));
	outputs[SOLO_OUTPUT].setVoltage((soloPulse.process(args.sampleTime) ? 10.0f : 0.0f));
	outputs[CLOCK_OUTPUT].setVoltage((clockPulse.process(args.sampleTime) ? 10.0f : 0.0f));
	outputs[STEP_OUTPUT].setVoltage(step*10/steps);
	
	
	}
};


struct SplitterburstWidget : ModuleWidget {
	SplitterburstWidget(Splitterburst* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Splitterburst.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		
		addChild(createLight<MediumLight<GreenLight>>(mm2px(Vec(16, 20)), module, Splitterburst::ON_LIGHT));


		
		addParam(createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(12, 100)), module, Splitterburst::STEPS_PARAM));
		addParam(createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(24, 100)), module, Splitterburst::MULTI_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(24, 90)), module, Splitterburst::MULTI_ATT));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(12, 90)), module, Splitterburst::STEPS_ATT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24, 80)), module, Splitterburst::MULTI_CV));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(12, 80)), module, Splitterburst::STEPS_CV));
		
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, 35)), module, Splitterburst::CLOCK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, 50)), module, Splitterburst::TRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18, 50)), module, Splitterburst::OFF_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(28, 50)), module, Splitterburst::TOG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, 64)), module, Splitterburst::STEP_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18, 35)), module, Splitterburst::GATE_INPUT));
		
		
		
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(28, 35)), module, Splitterburst::PLAIN_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(28, 64)), module, Splitterburst::GATE_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18, 64)), module, Splitterburst::STEP_OUTPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(12, 116)), module, Splitterburst::SOLO_OUTPUT));		
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(24, 116)), module, Splitterburst::CLOCK_OUTPUT));
	}
};


Model* modelSplitterburst = createModel<Splitterburst, SplitterburstWidget>("Splitterburst");