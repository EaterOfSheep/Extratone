#include "plugin.hpp"


struct Puzzlebox : Module {
	enum ParamIds {
		COM1_PARAM,
		COM2_PARAM,
		COM3_PARAM,
		BIG_PARAM,
		SMALL_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		YELLOW_INPUT,
		RED_INPUT,
		MAG_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		RED_OUTPUT,
		GREEN_OUTPUT,
		TARGET_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		START_LIGHT,
		FINALE_LIGHT,
		WIN_LIGHT,
		NUM_LIGHTS
	};

	Puzzlebox() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(BIG_PARAM, 0.f, 10.f, 0.f, "");
		configParam(SMALL_PARAM, 0.f, 1.f, 0.f, "");
		
		configParam(COM1_PARAM, 0.f, 10.f, 0.f, "");
		configParam(COM2_PARAM, 0.f, 20.f, 0.f, "");
		configParam(COM3_PARAM, 0.f, 30.f, 0.f, "");
	}
	
	float target=random::uniform()*10;
	int timer = 0;
	int score = 0;
	int coltimer=0;
	float red=1;
	float green=1;
	bool win = false;

	void process(const ProcessArgs& args) override {
	
		bool com1 = (params[COM1_PARAM].getValue()==8);
		bool com2 = (params[COM2_PARAM].getValue()==3);
		bool com3 = (params[COM3_PARAM].getValue()==27);
	
		//Looking through the source code for clues really ruins the fun!

		float knobs = params[BIG_PARAM].getValue()+params[SMALL_PARAM].getValue();
		
		int progress=0;		
		if(com1||com2||com3){progress++;}
		if(com1&&com2&&com3){progress++;}
		if(inputs[RED_INPUT].getVoltage()==red){progress++;}
		if(inputs[MAG_INPUT].getVoltage()==-green){progress++;}
		if(inputs[YELLOW_INPUT].getVoltage()==red+green){progress++;}
		
		
		coltimer++;
		if(coltimer>2000){
			coltimer=0;
			green=random::uniform();
			red=random::uniform();
		}
		
		if(abs(knobs-target)<=0.001){
			timer++;
		}else{
			timer=0;
		}
		
		int timergap=500;
		if(timer>timergap){
			timer=0;
			target=random::uniform()*10;
			if(score<100*timergap){
				score+=timergap*2;			
			}

		}
		
		if(score>90*timergap){
			win=true;
		}else{
			win=false;
		}
		
		if(score>0){score--;}

		
		if(progress>4){
			outputs[TARGET_OUTPUT].setVoltage(target);
		}else{
			outputs[TARGET_OUTPUT].setVoltage(progress);
		}
		
		outputs[RED_OUTPUT].setVoltage(red*(progress>0));
		outputs[GREEN_OUTPUT].setVoltage(green*(progress>1));
		
		lights[START_LIGHT].value=(progress>0);		
		lights[FINALE_LIGHT].value=(progress>4);		
		lights[WIN_LIGHT].value=(win);
	}
};


struct PuzzleboxWidget : ModuleWidget {
	PuzzleboxWidget(Puzzlebox* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Puzzlebox.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		
		addChild(createLight<MediumLight<GreenLight>>(mm2px(Vec(30, 62)), module, Puzzlebox::WIN_LIGHT));
		addChild(createLight<MediumLight<GreenLight>>(mm2px(Vec(26, 62)), module, Puzzlebox::FINALE_LIGHT));
		addChild(createLight<MediumLight<GreenLight>>(mm2px(Vec(22, 62)), module, Puzzlebox::START_LIGHT));
		
		addParam(createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(25, 75)), module, Puzzlebox::BIG_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(10, 90)), module, Puzzlebox::SMALL_PARAM));
		
		addParam(createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(7, 20)), module, Puzzlebox::COM1_PARAM));
		addParam(createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(33, 20)), module, Puzzlebox::COM2_PARAM));
		addParam(createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(20, 8)), module, Puzzlebox::COM3_PARAM));
		
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 115)), module, Puzzlebox::RED_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30, 50)), module, Puzzlebox::MAG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 50)), module, Puzzlebox::YELLOW_INPUT));
		
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10, 70)), module, Puzzlebox::RED_OUTPUT));		
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20, 20)), module, Puzzlebox::GREEN_OUTPUT));		
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30, 100)), module, Puzzlebox::TARGET_OUTPUT));
	}
};


Model* modelPuzzlebox = createModel<Puzzlebox, PuzzleboxWidget>("Puzzlebox");
