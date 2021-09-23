#include "plugin.hpp"


struct Pureneura : Module {
	enum ParamIds {
		CLOCK_PARAM,
		BASE_PARAM,
		MULTI_PARAM,
		NUM_PARAMS=MULTI_PARAM+4
	};
	enum InputIds {
		BASE_INPUT,
		CLOCK_INPUT,
		GATE_INPUT,
		NUM_INPUTS=GATE_INPUT+4
	};
	enum OutputIds {
		CLOCK_OUTPUT,
		PRODUCT_OUTPUT=CLOCK_OUTPUT+4,
		NUM_OUTPUTS=PRODUCT_OUTPUT+4
	};
	enum LightIds {
		BASE_LIGHT,
		GATE_LIGHT,
		NUM_LIGHTS=GATE_LIGHT+4
	};


	dsp::PulseGenerator outPulse[4];
	dsp::PulseGenerator prodPulse[3];

	dsp::SchmittTrigger clockTrigger;
	dsp::SchmittTrigger flipperTrigger;

	float timer1=0;
	float wait[4]={0};
	float pwait[3]={0};
	float goal1=0;
	bool gate[4] = {false};
	bool based = false;

	Pureneura() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		
		for(int i = 0; i<4; i++){
			configParam(MULTI_PARAM+i, 1, 8, 1, ""); } }

	void process(const ProcessArgs& args) override {

		if(inputs[BASE_INPUT].isConnected()){
			based = (inputs[BASE_INPUT].getVoltage()>=1.f);
		}else{
			based = params[BASE_PARAM].getValue();
		}


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


		if(clockTrigger.process(inputs[CLOCK_INPUT].getVoltage())){
			goal1=timer1;
			timer1=0;
		}

		float goal;
		float timer;

		goal=goal1;
		timer=timer1;


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


struct PureneuraWidget : ModuleWidget {
	PureneuraWidget(Pureneura* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Pureneura.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addChild(createLightCentered<MediumLight<XtrtnPinkLight>>(mm2px(Vec(12, 114)), module, Pureneura::BASE_LIGHT));
		addParam(createParamCentered<CKSS>(mm2px(Vec(19, 114)), module, Pureneura::BASE_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30, 114)), module, Pureneura::BASE_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24, 18)), module, Pureneura::CLOCK_INPUT));

		for(int i = 0; i<4; i++){
			
			float top = 39.f;
			float gap = 12.f;

			addChild(createLightCentered<MediumLight<XtrtnBlueLight>>(mm2px(Vec(13, top+gap*i)), module, Pureneura::GATE_LIGHT+i));

			addParam(createParamCentered<XtrtnSnapKnob>(mm2px(Vec(21, top+gap*i)), module, Pureneura::MULTI_PARAM+i));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6, top+gap*i)), module, Pureneura::GATE_INPUT+i));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32, top+gap*i)), module, Pureneura::CLOCK_OUTPUT+i));

		}

		for (int i = 0; i<3; i++){

			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.5+10*i, 97)), module, Pureneura::PRODUCT_OUTPUT+i));
		}


	}
};


Model* modelPureneura = createModel<Pureneura, PureneuraWidget>("Pureneura");
