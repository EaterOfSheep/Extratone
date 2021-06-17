#include "plugin.hpp"


struct Ichneumonid : Module {
	enum ParamIds {
		U2UD2D_PARAM,
		PROBABILITY_PARAM,
		SELECT_PARAM,
		SELECTION_PARAM=SELECT_PARAM+4,
		DEPARTURE_PARAM=SELECTION_PARAM+1,
		ARRIVAL_PARAM=DEPARTURE_PARAM+4,
		NUM_PARAMS=ARRIVAL_PARAM+4
	};
	enum InputIds {
		ENABLE_INPUT,
		SELECT_INPUT=ENABLE_INPUT+4,
		INCOMING_INPUT=SELECT_INPUT+4,
		NUM_INPUTS=INCOMING_INPUT+4
	};
	enum OutputIds {
		SWITCHED_OUTPUT,
		TRUE_OUTPUT,
		GATE_OUTPUT,
		NUM_OUTPUTS=GATE_OUTPUT+4
	};
	enum LightIds {
		UUDD_LIGHT,
		ENABLE_LIGHT,
		SELECT_LIGHT=ENABLE_LIGHT+4,
		NUM_LIGHTS=SELECT_LIGHT+4
	};


	dsp::PulseGenerator switchPulse;	
	dsp::SchmittTrigger selectTrigger[4];

	bool bans[4]={false};
	float prevs[4]={0};
	bool uudd=false;
	int selection = 0;
	float current = 0;
	float tolerance = 0.05;

	Ichneumonid() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configParam(PROBABILITY_PARAM, 0.f, 1.f, 0.1f, "");

		for(int i = 0; i<4; i++){
			configParam(ARRIVAL_PARAM+i, 0.f, 1.f, 1.f, "");
			configParam(DEPARTURE_PARAM+i, 0.f, 1.f, 1.f, "");
		}
	}

	void process(const ProcessArgs& args) override {

		for(int i = 0; i<4; i++){

			lights[SELECT_LIGHT + i].setBrightness((i==params[SELECTION_PARAM].getValue()));

			outputs[GATE_OUTPUT + i].setVoltage((10*(i==params[SELECTION_PARAM].getValue())));

			if(selectTrigger[i].process(params[SELECT_PARAM+i].getValue()+inputs[SELECT_INPUT+i].getVoltage())){
				params[SELECTION_PARAM].setValue(i);

			}
		}
		uudd=(params[U2UD2D_PARAM].getValue()>0);
		selection = params[SELECTION_PARAM].getValue();
		current = inputs[INCOMING_INPUT+selection].getVoltage();

		bool enabled[4];
		for(int i = 0; i<4; i++){
			enabled[i]=((inputs[ENABLE_INPUT+i].getVoltage()>=5)||!inputs[ENABLE_INPUT+i].isConnected());
			enabled[i]=(inputs[INCOMING_INPUT+i].isConnected()&&enabled[i]);

			lights[ENABLE_LIGHT + i].setBrightness(enabled[i]);
		}

		outputs[TRUE_OUTPUT].setVoltage(current);


		for(int j = 0; j<4; j++){

			int i = wraparound(j+selection,4);

			if(i!=selection){

				float other = inputs[INCOMING_INPUT+i].getVoltage();
				
				if(!inRange(other,current,tolerance*1.1)&&bans[i]){
					bans[i]=false;
				}


				if(jumpable(selection,i)&&!bans[i]&&enabled[i]){

					if(test(selection,i)){
						switchPulse.trigger();
						params[SELECTION_PARAM].setValue(i);
						bans[selection]=true;
					}else{
						bans[i]=true;
					}
				}
			}

		}


		lights[UUDD_LIGHT].setBrightness(uudd);

		outputs[SWITCHED_OUTPUT].setVoltage((switchPulse.process(args.sampleTime) ? 10.0f : 0.0f));

		for(int i = 0; i<4; i++){
			prevs[i]=inputs[INCOMING_INPUT+i].getVoltage();
		}
	}

	int wraparound(int n, int m){
		while(n>=m){
			n-=m;
		}
		return n;

	}

	bool jumpable(int from, int to){


		float fromvalue = inputs[INCOMING_INPUT+from].getVoltage();
		float tovalue = inputs[INCOMING_INPUT+to].getVoltage();

		if(!inRange(tovalue,fromvalue,tolerance)){return false;}

		if(uudd){
			float fromprev = prevs[from];
			float toprev = prevs[to];
			bool fromup = (fromvalue>fromprev);
			bool toup = (tovalue>toprev);
			if(toup!=fromup){return false;}

		}

		return true;
	}

	bool inRange(float x, float y, float tol){
		return abs(x-y)<=tol;
	}

	bool test(int from, int to){
		double probability = params[PROBABILITY_PARAM].getValue()*params[ARRIVAL_PARAM+to].getValue()*params[DEPARTURE_PARAM+from].getValue();
		return random::uniform()<probability;
	}
};


struct IchneumonidWidget : ModuleWidget {
	IchneumonidWidget(Ichneumonid* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Ichneumonid.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


		for(int i = 0; i<4; i++){

			addChild(createLightCentered<MediumLight<XtrtnBlueLight>>(mm2px(Vec(10+10*i, 19)), module, Ichneumonid::SELECT_LIGHT+i));
			addChild(createLightCentered<MediumLight<XtrtnPinkLight>>(mm2px(Vec(10+10*i, 14)), module, Ichneumonid::ENABLE_LIGHT+i));

			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10+10*i, 27)), module, Ichneumonid::INCOMING_INPUT+i));	
			addParam(createParamCentered<TL1105>(mm2px(Vec(10+10*i,37)), module, Ichneumonid::SELECT_PARAM+i));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10+10*i, 47)), module, Ichneumonid::SELECT_INPUT+i));	
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10+10*i, 57)), module, Ichneumonid::GATE_OUTPUT+i));	

			addParam(createParamCentered<XtrtnMedKnob>(mm2px(Vec(10+10*i,67)), module, Ichneumonid::ARRIVAL_PARAM+i));
			addParam(createParamCentered<XtrtnMedKnob>(mm2px(Vec(10+10*i,77)), module, Ichneumonid::DEPARTURE_PARAM+i));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10+10*i, 87)), module, Ichneumonid::ENABLE_INPUT+i));	

		}

		addParam(createParamCentered<XtrtnKnob>(mm2px(Vec(10,107)), module, Ichneumonid::PROBABILITY_PARAM));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30, 106)), module, Ichneumonid::TRUE_OUTPUT));	
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(50, 106)), module, Ichneumonid::SWITCHED_OUTPUT));	
		addParam(createParamCentered<CKSS>(mm2px(Vec(47, 122)), module, Ichneumonid::U2UD2D_PARAM));
		addChild(createLightCentered<MediumLight<XtrtnPinkLight>>(mm2px(Vec(53, 120)), module, Ichneumonid::UUDD_LIGHT));
	}
};


Model* modelIchneumonid = createModel<Ichneumonid, IchneumonidWidget>("Ichneumonid");
