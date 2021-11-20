#include "plugin.hpp"


struct Meganeura : Module {
	enum ParamIds {
		FLIPPER_PARAM,
		CLOCK_PARAM,
		BASE_PARAM,
		MULTI_PARAM,
		NUM_PARAMS=MULTI_PARAM+4
	};
	enum InputIds {
		FLIPPER_INPUT,
		CLOCK_INPUT,
		CLOCK2_INPUT,
		GATE_INPUT,
		NUM_INPUTS=GATE_INPUT+4
	};
	enum OutputIds {
		CLOCK_OUTPUT,
		PRODUCT_OUTPUT=CLOCK_OUTPUT+4,
		NUM_OUTPUTS=PRODUCT_OUTPUT+4
	};
	enum LightIds {
		CLOCK1_LIGHT,
		CLOCK2_LIGHT,
		BASE_LIGHT,
		GATE_LIGHT,
		NUM_LIGHTS=GATE_LIGHT+4
	};


	dsp::PulseGenerator outPulse[4];
	dsp::PulseGenerator prodPulse[3];

	dsp::SchmittTrigger clockTrigger;
	dsp::SchmittTrigger clockTrigger2;
	dsp::SchmittTrigger flipperTrigger;

	float timer1=0;
	float timer2=0;
	float wait[4]={0};
	float pwait[3]={0};
	float goal1=0;
	float goal2=0;
	bool gate[4] = {false};
	bool based = false;

	Meganeura() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		for(int i = 0; i<4; i++){
			configParam(MULTI_PARAM+i, 1, 8, 1, "");

			configBypass(CLOCK_INPUT,CLOCK_OUTPUT+i);
			configBypass(CLOCK_INPUT,PRODUCT_OUTPUT+i);

		}

	}

	void process(const ProcessArgs& args) override {

		based = params[BASE_PARAM].getValue();

		bool flipped=params[CLOCK_PARAM].getValue();

		lights[CLOCK1_LIGHT].setBrightness(!flipped);
		lights[CLOCK2_LIGHT].setBrightness(flipped);
		lights[BASE_LIGHT].setBrightness(based);

		for(int i = 0; i<4; i++){

			gate[i]=(!inputs[GATE_INPUT+i].isConnected()||inputs[GATE_INPUT+i].getVoltage()>=5.f);
			lights[GATE_LIGHT+i].setBrightness(gate[i]);

			wait[i]++;

		}

		for(int i = 0; i<3; i++){
			pwait[i]++;
		}

		timer1++;
		timer2++;


		if(flipperTrigger.process(params[FLIPPER_PARAM].getValue()+inputs[FLIPPER_INPUT].getVoltage())){
			params[CLOCK_PARAM].setValue(!params[CLOCK_PARAM].getValue());
		}

		bool basepulse=false;

		if(clockTrigger.process(inputs[CLOCK_INPUT].getVoltage())){
			goal1=timer1;
			timer1=0;
			if(!flipped){
				basepulse=true;
			}
		}

		if(clockTrigger2.process(inputs[CLOCK2_INPUT].getVoltage())){
			goal2=timer2;
			timer2=0;
			if(flipped){
				basepulse=true;
			}
		}

		float goal;
		float timer;

		if(flipped){
			goal=goal2;
			timer=timer2;
		}else{
			goal=goal1;
			timer=timer1;
		}



		if(basepulse){

			for(int i = 0; i<4; i++){

				if(based||gate[i]){
					wait[i]=0;
					outPulse[i].trigger();

				}
			}

			for(int i = 0; i<3; i++){
				pwait[i]=0;
				prodPulse[i].trigger();
			}

		}

		if(timer!=0){

			for(int i = 0; i<4; i++){

				float divide = params[MULTI_PARAM+i].getValue();
				if(wait[i]*divide>goal&&gate[i]){
					wait[i]=0;
					outPulse[i].trigger();
				}
			}

			for(int p = 0; p<3; p++){

				float divide = 1;

				for(int i = 0; i<p+2; i++){

					if(gate[i]){

						divide = divide*params[MULTI_PARAM+i].getValue();
					}

				}

				if(pwait[p]*divide>goal){
						pwait[p]=0;
						prodPulse[p].trigger();
				}
			}
		}

		for(int i = 0; i<4; i++){
			outputs[CLOCK_OUTPUT+i].setVoltage((outPulse[i].process(args.sampleTime) ? 10.0f : 0.0f));

		}


		for(int i = 0; i<3; i++){
			outputs[PRODUCT_OUTPUT+i].setVoltage((prodPulse[i].process(args.sampleTime) ? 10.0f : 0.0f));

		}


	}
};


struct MeganeuraWidget : ModuleWidget {
	MeganeuraWidget(Meganeura* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Meganeura.svg")));

		addChild(createWidget<XtrtnScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<XtrtnScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<XtrtnScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<XtrtnScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<TL1105>(mm2px(Vec(20,108)), module, Meganeura::FLIPPER_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30, 108)), module, Meganeura::FLIPPER_INPUT));

		addChild(createLightCentered<MediumLight<XtrtnPinkLight>>(mm2px(Vec(18, 116)), module, Meganeura::BASE_LIGHT));
		addParam(createParamCentered<CKSS>(mm2px(Vec(25, 116)), module, Meganeura::BASE_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14, 25)), module, Meganeura::CLOCK_INPUT));
		addChild(createLightCentered<MediumLight<XtrtnPinkLight>>(mm2px(Vec(5, 25)), module, Meganeura::CLOCK1_LIGHT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(26, 25)), module, Meganeura::CLOCK2_INPUT));
		addChild(createLightCentered<MediumLight<XtrtnPinkLight>>(mm2px(Vec(35, 25)), module, Meganeura::CLOCK2_LIGHT));

		for(int i = 0; i<4; i++){

			float top = 42.f;
			float gap = 12.f;

			addChild(createLightCentered<MediumLight<XtrtnBlueLight>>(mm2px(Vec(13, top+gap*i)), module, Meganeura::GATE_LIGHT+i));

			addParam(createParamCentered<XtrtnSnapKnob>(mm2px(Vec(21, top+gap*i)), module, Meganeura::MULTI_PARAM+i));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6, top+gap*i)), module, Meganeura::GATE_INPUT+i));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32, top+gap*i)), module, Meganeura::CLOCK_OUTPUT+i));

		}

		for (int i = 0; i<3; i++){

			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.5+10*i, 97)), module, Meganeura::PRODUCT_OUTPUT+i));
		}


	}
};


Model* modelMeganeura = createModel<Meganeura, MeganeuraWidget>("Meganeura");
