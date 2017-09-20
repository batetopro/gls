#include <iostream>

#include "../data_structure/graph.h"
#include "../data_structure/io/graph_io.h"
#include "../coloring/gls.h"

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
	
	#ifdef UPPER
	std::cout << gls::ColoringUpperBound::simple(G) << ",";
	std::cout << gls::ColoringUpperBound::theorem2(G) << ",";
	std::cout << gls::ColoringUpperBound::theorem3(G) << ",";
	std::cout << gls::EpocheRunner::get_colors(gls::ColoringBuilder::greedy(G)) << std::endl;
	#else
	gls::init();
	gls::ColoringBuilder builder = gls::ColoringBuilder();
	gls::EpocheRunner runner = gls::EpocheRunner();
	gls::colors result = runner.solve(G, builder.build(G));
	#endif
	return 0;
 }
