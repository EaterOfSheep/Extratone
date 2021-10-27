#include "plugin.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	// p->addModel(modelMyModule);
	p->addModel(modelModulo);
	p->addModel(modelMesoglea);
	p->addModel(modelMesoglea2);
	p->addModel(modelOpabinia);
	p->addModel(modelSplitterburst);
	p->addModel(modelPuzzlebox);
	p->addModel(modelDarwinism);
//	p->addModel(modelHalluciMemory);
	p->addModel(modelIchneumonid);
	p->addModel(modelMeganeura);
	p->addModel(modelPureneura);
	p->addModel(modelMesohyl);


	p->addModel(modelXtrtnBlank);


	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
