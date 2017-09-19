#include <iostream>


#include "graph.h"
#include "graph_io.h"
#include "gls.h"

int main(int argc, const char* argv[]) {
    srand(0);
	
	std::string graph_filename;
	
	if (argc <= 1){
		graph_filename = "simple.graph";
	} else {
		graph_filename = argv[1];
	}
	
	graph_access G;
	graph_io::readGraphWeighted(G, graph_filename);
	
	gls::init();
	gls::ColoringBuilder builder = gls::ColoringBuilder();
	gls::EpocheRunner runner = gls::EpocheRunner();
	gls::colors result = runner.solve(G, builder.build(G));
	
	return 0;
 }
