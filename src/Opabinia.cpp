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
		FOLDN_PARAM,
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
		SOUND_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		FDECAY_OUTPUT,
		ADECAY_OUTPUT,
		DRUM_OUTPUT,
		SOUND_OUTPUT,
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
	int foldnum = 2;
	
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

		configParam(FOLDN_PARAM, 1.f, 5.f, 1, "");


	}

	float distort(float wave, float squareness, float fold, int foldnum){

		bool folding = true;

		while(foldnum>0&&folding)
		{
			folding=false;
			foldnum--;

			wave=wave*(1.f+fold*2.f);
			for(int i = 0; i<3; i++){
				if(wave>1){
					wave = 1.f-(wave-1.f);
					folding=true;
				}
				if(wave<-1){
					wave = -1.f-(wave+1.f);
					folding=true;
				}
			}
		}
		
		wave=wave*(1.f+squareness*4.f);
		if(wave>1){
			wave = 1.f;
		}
		if(wave<-1){
			wave = -1.f;
		}


	return wave;

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
	
		foldnum=params[FOLDN_PARAM].getValue();

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
	
		float wave = 0.f;
		

		float sine = std::sin(2.f * M_PI * phase); //sine

		wave = sine;

		wave=distort(wave,squareness,fold,foldnum);
		


		float sound = inputs[SOUND_INPUT].getVoltage()/5.f;
		sound = distort(sound,squareness,fold,foldnum);
		
		float aratio = std::pow(0.5, args.sampleTime*(1.f/adecay));
		float fratio = std::pow(0.5, args.sampleTime*(1.f/fdecay));
		amplitude = amplitude*aratio;
		freq = (freq-basefreq)*fratio+basefreq;
		
		outputs[DRUM_OUTPUT].setVoltage(5.f * amplitude * wave);
		outputs[ADECAY_OUTPUT].setVoltage(5.f*amplitude);
		outputs[FDECAY_OUTPUT].setVoltage(5.f*(freq-basefreq)/(peakfreq-basefreq));

		outputs[SOUND_OUTPUT].setVoltage(5.f*sound);
	
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
		

		
		addParam(createParamCentered<XtrtnKnob>(mm2px(Vec(24, 55)), module, Opabinia::FREQ_PARAM));
		addParam(createParamCentered<XtrtnKnob>(mm2px(Vec(24, 90)), module, Opabinia::BASEFREQ_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(24, 45)), module, Opabinia::FREQ_ATT));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(24, 80)), module, Opabinia::BASEFREQ_ATT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24, 35)), module, Opabinia::FREQ_CV));	
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24, 70)), module, Opabinia::BASEFREQ_CV));		

		
		addParam(createParamCentered<XtrtnKnob>(mm2px(Vec(36, 90)), module, Opabinia::FDECAY_PARAM));
		addParam(createParamCentered<XtrtnKnob>(mm2px(Vec(36, 55)), module, Opabinia::ADECAY_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(36, 80)), module, Opabinia::FDECAY_ATT));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(36, 45)), module, Opabinia::ADECAY_ATT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(36, 35)), module, Opabinia::ADECAY_CV));	
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(36, 70)), module, Opabinia::FDECAY_CV));		

		addParam(createParamCentered<XtrtnKnob>(mm2px(Vec(12, 90)), module, Opabinia::SQUARE_PARAM));
		addParam(createParamCentered<XtrtnKnob>(mm2px(Vec(12, 55)), module, Opabinia::FOLD_PARAM));	
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(12, 80)), module, Opabinia::SQUARE_ATT));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(12, 45)), module, Opabinia::FOLD_ATT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(12, 35)), module, Opabinia::FOLD_CV));	
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(12, 70)), module, Opabinia::SQUARE_CV));		

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, 106)), module, Opabinia::TRIGGER_INPUT));
		
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(39, 106)), module, Opabinia::DRUM_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(29, 106)), module, Opabinia::FDECAY_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(19, 106)), module, Opabinia::ADECAY_OUTPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24, 20)), module, Opabinia::SOUND_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(36, 20)), module, Opabinia::SOUND_OUTPUT));


		addParam(createParamCentered<XtrtnSnapKnob>(mm2px(Vec(12, 22)), module, Opabinia::FOLDN_PARAM));


	}
};


Model* modelOpabinia = createModel<Opabinia, OpabiniaWidget>("Opabinia");
