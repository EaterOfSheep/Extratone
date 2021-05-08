#include "plugin.hpp"


struct Opabinia : Module {
	enum ParamIds {
		FDECAY_PARAM,
		ADECAY_PARAM,
		FDECAY_ATT,
		ADECAY_ATT,
		FREQ_PARAM,
		BASEFREQ_PARAM,
		FREQ_ATT,
		BASEFREQ_ATT,
		SQUARE_PARAM,
		FOLD_PARAM,
		SQUARE_ATT,
		FOLD_ATT,
		NUM_PARAMS
	};
	enum InputIds {
		FDECAY_CV,
		ADECAY_CV,
		FREQ_CV,
		BASEFREQ_CV,
		TRIGGER_INPUT,
		SQUARE_CV,
		FOLD_CV,
		NUM_INPUTS
	};
	enum OutputIds {
		FDECAY_OUTPUT,
		ADECAY_OUTPUT,
		DRUM_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
	
	dsp::SchmittTrigger trigger;
	
	float basefreq = 20.f;
	float peakfreq = 500.f;
	float phase = 0.f;
	float freq = 100.f;
	float amplitude = 1.f;
	float fdecay = 1.f;
	float adecay = 1.f;
	float fold = 0.f;
	float squareness=0.f;
	
	Opabinia() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		
		configParam(FDECAY_PARAM, 0.f, 1.f, 0.1, "");
		configParam(ADECAY_PARAM, 0.f, 1.f, 0.5, "");
		configParam(BASEFREQ_PARAM, 0.f, 100.f, 30.f, "");
		configParam(FREQ_PARAM, 100.f, 500.f, 350.f, "");
		configParam(FOLD_PARAM, 0.f, 1.f, 0.5, "");
		configParam(SQUARE_PARAM, 0.f, 1.f, 0.2, "");
		
		configParam(FREQ_ATT, -1.f, 1.f, 0.f, "");
		configParam(BASEFREQ_ATT, -1.f, 1.f, 0.f, "");
		configParam(FOLD_ATT, -1.f, 1.f, 0.f, "");
		configParam(SQUARE_ATT, -1.f, 1.f, 0.f, "");
		configParam(ADECAY_ATT, -1.f, 1.f, 0.f, "");
		configParam(FDECAY_ATT, -1.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
	
		fdecay = params[FDECAY_PARAM].getValue()+inputs[FDECAY_CV].getVoltage()*0.1*params[FDECAY_ATT].getValue();
		if(fdecay>1){fdecay=1;}
		if(fdecay<0){fdecay=0;}
		
		adecay = params[ADECAY_PARAM].getValue()+inputs[ADECAY_CV].getVoltage()*0.1*params[ADECAY_ATT].getValue();
		if(adecay>1){adecay=1;}
		if(adecay<0){adecay=0;}
		
		fdecay=0.001+fdecay*0.2;
		adecay=0.001+adecay*0.5;
		
		peakfreq=params[FREQ_PARAM].getValue()*std::pow(2.f,inputs[FREQ_CV].getVoltage()*params[FREQ_ATT].getValue());
		basefreq=params[BASEFREQ_PARAM].getValue()*std::pow(2.f,inputs[BASEFREQ_CV].getVoltage()*params[BASEFREQ_ATT].getValue());
		
		squareness=params[SQUARE_PARAM].getValue()+params[SQUARE_ATT].getValue()*inputs[SQUARE_CV].getVoltage()/10.f;
		fold=params[FOLD_PARAM].getValue()+params[FOLD_ATT].getValue()*inputs[FOLD_CV].getVoltage()/10.f;
		
		if(squareness>1){squareness=1;}
		if(squareness<0){squareness=0;}
		
		if(fold>1){fold=1;}
		if(fold<0){fold=0;}
	
		if (trigger.process(inputs[TRIGGER_INPUT].getVoltage())) {
			phase = 0.f;
			amplitude=1.f;
			freq = peakfreq;
		}
	
		phase += freq * args.sampleTime;
		if (phase >= 0.5f){phase -= 1.f;}
		
		float sine = std::sin(2.f * M_PI * phase);
		//float square = 2*(phase>0)-1.f;
		//float mix = square*squareness+sine*(1.f-squareness);
		

		sine=sine*(1.f+fold*2.f);
		for(int i = 0; i<9; i++){
			if(sine>1){
				sine = 1.f-(sine-1.f);
			}
			if(sine<-1){
				sine = -1.f-(sine+1.f);
			}
		}
		
		sine=sine*(1.f+squareness*4.f);
		if(sine>1){
			sine = 1.f;
		}
		if(sine<-1){
			sine = -1.f;
		}




		float aratio = std::pow(0.5, args.sampleTime*(1.f/adecay));
		float fratio = std::pow(0.5, args.sampleTime*(1.f/fdecay));
		amplitude = amplitude*aratio;
		freq = (freq-basefreq)*fratio+basefreq;
		
		outputs[DRUM_OUTPUT].setVoltage(5.f * amplitude * sine);
		outputs[ADECAY_OUTPUT].setVoltage(5.f*amplitude);
		outputs[FDECAY_OUTPUT].setVoltage(5.f*(freq-basefreq)/(peakfreq-basefreq));
	
	}
};


struct OpabiniaWidget : ModuleWidget {
	OpabiniaWidget(Opabinia* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Opabinia.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		

		
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(24, 65)), module, Opabinia::FREQ_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(24, 100)), module, Opabinia::BASEFREQ_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(24, 55)), module, Opabinia::FREQ_ATT));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(24, 90)), module, Opabinia::BASEFREQ_ATT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24, 45)), module, Opabinia::FREQ_CV));	
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24, 80)), module, Opabinia::BASEFREQ_CV));		

		
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(36, 100)), module, Opabinia::FDECAY_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(36, 65)), module, Opabinia::ADECAY_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(36, 90)), module, Opabinia::FDECAY_ATT));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(36, 55)), module, Opabinia::ADECAY_ATT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(36, 45)), module, Opabinia::ADECAY_CV));	
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(36, 80)), module, Opabinia::FDECAY_CV));		

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(12, 100)), module, Opabinia::SQUARE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(12, 65)), module, Opabinia::FOLD_PARAM));	
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(12, 90)), module, Opabinia::SQUARE_ATT));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(12, 55)), module, Opabinia::FOLD_ATT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(12, 45)), module, Opabinia::FOLD_CV));	
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(12, 80)), module, Opabinia::SQUARE_CV));		

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24, 30)), module, Opabinia::TRIGGER_INPUT));
		
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(36, 116)), module, Opabinia::DRUM_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(24, 116)), module, Opabinia::FDECAY_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(12, 116)), module, Opabinia::ADECAY_OUTPUT));
	}
};


Model* modelOpabinia = createModel<Opabinia, OpabiniaWidget>("Opabinia");