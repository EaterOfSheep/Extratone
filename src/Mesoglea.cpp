#include "plugin.hpp"


struct Mesoglea : Module {
	enum ParamIds {
		MODE_PARAM,
		MODEBUTTON_PARAM,
		FLIPBUTTON_PARAM,
		FLIP_PARAM=FLIPBUTTON_PARAM+16,
		GATE_PARAM=FLIP_PARAM+16,
		NUM_PARAMS=GATE_PARAM+16
	};
	enum InputIds {
		SANDH_INPUT,
		FLIP_INPUT,
		TOMUTE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		SHOFF_LIGHT,
		SHSOLO_LIGHT,
		SHPOLY_LIGHT,
		FLIP_LIGHT,
		GATE_LIGHT=FLIP_LIGHT+16,
		NUM_LIGHTS=GATE_LIGHT+16
	};


	dsp::SchmittTrigger modeTrigger;
	dsp::SchmittTrigger flipTrigger[16];

	Mesoglea() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configParam(MODE_PARAM, 0.0, 1.0, 0.0);
		configBypass(TOMUTE_INPUT,OUTPUT_OUTPUT);
	}

	void process(const ProcessArgs& args) override {


		if(modeTrigger.process(params[MODEBUTTON_PARAM].getValue())){

			params[MODE_PARAM].setValue(params[MODE_PARAM].getValue()+1);

			if(params[MODE_PARAM].getValue()>2){

				params[MODE_PARAM].setValue(0);

			}
		}

		int mode = params[MODE_PARAM].getValue();
		bool monoflip = getMonoflip();

		int channels = 1;
		channels=std::max(channels,inputs[TOMUTE_INPUT].getChannels());
		channels=std::max(channels,inputs[SANDH_INPUT].getChannels());
		channels=std::max(channels,inputs[FLIP_INPUT].getChannels());

		for(int i = 0; i<16; i++){

			if(flipTrigger[i].process(params[FLIPBUTTON_PARAM+i].getValue()||inputs[FLIP_INPUT].getVoltage(i))){
				params[FLIP_PARAM+i].setValue(!params[FLIP_PARAM+i].getValue());

			}

			bool flipping = params[FLIP_PARAM+i].getValue();

			if(mode==0){
				if(flipping){
					params[FLIP_PARAM+i].setValue(0);
					params[GATE_PARAM+i].setValue(!params[GATE_PARAM+i].getValue());
				}

			}

			if(mode==1){
				if(flipping&&monoflip){
					params[FLIP_PARAM+i].setValue(0);
					params[GATE_PARAM+i].setValue(!params[GATE_PARAM+i].getValue());

				}
			}

			if(mode==2){
				if(flipping&&(inputs[SANDH_INPUT].getVoltage(i)>=1)){
					params[FLIP_PARAM+i].setValue(0);
					params[GATE_PARAM+i].setValue(!params[GATE_PARAM+i].getValue());

				}
			}

			float voltage = 0;

			if(inputs[TOMUTE_INPUT].getChannels()>=i+1){

				voltage=inputs[TOMUTE_INPUT].getVoltage(i)*params[GATE_PARAM+i].getValue();

			}else{

				voltage=10.f*params[GATE_PARAM+i].getValue();

			}

			if(params[GATE_PARAM+i].getValue()){
				channels=std::max(i+1,channels);
			}

			outputs[OUTPUT_OUTPUT].setVoltage(voltage,i);


			lights[FLIP_LIGHT+i].setBrightness(params[FLIP_PARAM+i].getValue());
			lights[GATE_LIGHT+i].setBrightness(params[GATE_PARAM+i].getValue());

		}

		outputs[OUTPUT_OUTPUT].channels=channels;

		lights[SHOFF_LIGHT].setBrightness(params[MODE_PARAM].getValue()==0);
		lights[SHSOLO_LIGHT].setBrightness(params[MODE_PARAM].getValue()==1);
		lights[SHPOLY_LIGHT].setBrightness(params[MODE_PARAM].getValue()==2);



	}

	bool getMonoflip(){
		for(int i = 0; i<16; i++){
			if(inputs[SANDH_INPUT].getVoltage(i)>=1){return true;}
		}
		return false;
	}

};


struct MesogleaWidget : ModuleWidget {
	MesogleaWidget(Mesoglea* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Mesoglea.svg")));

		addChild(createWidget<XtrtnScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<XtrtnScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<XtrtnScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<XtrtnScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		int modey = 36;
		int modex = 8;
		int modegap = 12;
		addChild(createLightCentered<MediumLight<XtrtnPinkLight>>(mm2px(Vec(modex, modey)), module, Mesoglea::SHOFF_LIGHT));
		addChild(createLightCentered<MediumLight<XtrtnPinkLight>>(mm2px(Vec(modex+modegap, modey)), module, Mesoglea::SHSOLO_LIGHT));
		addChild(createLightCentered<MediumLight<XtrtnPinkLight>>(mm2px(Vec(modex+modegap*2, modey)), module, Mesoglea::SHPOLY_LIGHT));


		addParam(createParamCentered<TL1105>(mm2px(Vec(28,22)), module, Mesoglea::MODEBUTTON_PARAM));


		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11, 23)), module, Mesoglea::SANDH_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, 104.5)), module, Mesoglea::FLIP_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20, 104.5)), module, Mesoglea::TOMUTE_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32, 104.5)), module, Mesoglea::OUTPUT_OUTPUT));

		int gridy=46;
		int gridx=8;
		int gridxgap=8;
		int gridygap=12;
		for(int y = 0; y<4; y++){
			for(int x = 0; x<4; x++){
				addParam(createParamCentered<TL1105>(mm2px(Vec(gridx+gridxgap*x,gridy+gridygap*y)), module, Mesoglea::FLIPBUTTON_PARAM+x+4*y));

				addChild(createLightCentered<MediumLight<XtrtnBlueLight>>(mm2px(Vec(gridx+gridxgap*x+1,gridy+gridygap*y+6)), module, Mesoglea::GATE_LIGHT+x+4*y));

				addChild(createLightCentered<SmallLight<XtrtnPinkLight>>(mm2px(Vec(gridx+gridxgap*x-3,gridy+gridygap*y+6)), module, Mesoglea::FLIP_LIGHT+x+4*y));

			}
		}



	}
};

Model* modelMesoglea = createModel<Mesoglea, MesogleaWidget>("Mesoglea");
