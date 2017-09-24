// Turn on debugging
#define DEBUGGING 1

// Turn on fast search heuristic
// #define FAST_SEARCH_ENABLE 1

// Turn on dynammic lambda heuristic
#define DYNAMIC_LAMBDA_ENABLE 1

// Use .ini file configuration
#include "../lib/SimpleIni.h"
#define CONFIG "gls.ini"

#include <algorithm>
#include <vector>
#include <stack>
#include <ctime>

namespace gls {
	/*
	 * =====
	 * Types
	 * =====
	 */
	/* Type: uint
	Alias for unisgned int.
	*/
	typedef unsigned int uint;
	
	/* Type: color
	Alias for a int, representing a color.
	*/
	typedef uint color;
	
	/* Type: colors
	Alias for std::vector<color>.
	*/
	typedef typename std::vector<color> colors;

	/* Type: upair
	Alias for std::pair<uint, uint>.
	*/
	typedef typename std::pair<uint, uint> upair;
	
	/* Type: upairs
	Alias for std::vector<std::pair<uint, uint>>.
	*/
	typedef typename std::vector<upair> upairs;
	
	
	/*
	 * ===========
	 * Enumeratons
	 * ===========
	 */
	/* Enum: BuildStrategy
	How to build an initial coloring
	
	RandomStart - Builds a random coloring, which may have initial conflicts.
	Greedy      - Use a greedy algorithm to build a coloring, which is garanted to not have initial conflicts.
	Bipartite   - Make a coloring with two colors, using a modified DFS. The coloring may have initial conflicts.
	*/
	enum BuildStrategy{ RandomStart, Greedy, Bipartite };
	
	/* Enum: EpocheStrategy
	What to do between two sequent epoches -> going form k colors to k-1 colors in the searched coloring.
	
	Scratch - Builds a new random coloring with k-1 colors.
	Merge   - Use the current coloring by merging one color class to another.
	*/
	enum EpocheStrategy{ Scratch, Merge };
	
	/* Enum: EpocheTarget
	Which color to select in the merges as ssource or target.
	
	Random  		- Choose random color. 
	Minimal 		- Choose the color with minimal count of nodes
	Maximal 		- Choose the color with maximal count of nodes
	Median  		- Choose the color with is the median of the count of nodes
	*/
	enum EpocheTarget{ Random, Minimal, Maximal, Median };
	
	/* Enum: ColoringUpperBoundMethod
	Which is the method for estimating the upper bound of the chromatic number.
	
	For more information: <http://www.sciencedirect.com/science/article/pii/S0166218X11003039>
	
	Simple   - Use the Brooks theorem. 
	Theorem2 - Use Theorem 2.
	Theorem3 - Use Theorem 3.
	*/
	enum ColoringUpperBoundMethod{ Simple, Theorem2, Theorem3 };
	
	/* Enum: SolveResolution
	Resolution of the GLS iterations.
	
	NotFound - The current solution coloring contains conflicts. 
	Solved   - The curent solution coloring does not contain conflicts.
	NoImprove - Make weights update, because the solution score did not updated for some steps.
	LocalMin - Make weights update, because the current solution can not be improved.
	MaxIterations - Terminating, because the maximum iteration limit was reached.
	Timeout - Terminating, because the execution time exceeds a given time limit.
	*/
	enum SolveResolution{ NotFound, Solved, NoImprove, LocalMin, MaxIterations, Timeout };
	
	/*
	 * =============
	 * Configuration
	 * =============
	 */
	
	/* Constants: Configuration fields
	BuildStrategy BUILD_STRATEGY 			- How to build initail graph coloring. Default: *Greedy*
	EpocheStrategy UPDATE_STRATEGY      	- How to update the coloring between the epoches. Default: *Мерге*
	EpocheTarget SOURCE_TARGET				- Source color of the merges. Default: *Минимум*
	EpocheTarget DESTINATION_TARGET  		- Destination color of the merges. Default: *Маьимум*
	ColoringUpperBoundMethod UPPER_BOUND   	- How to calcilate the upper bound of the coloring. Default: *Тхереом 2*
	uint RESET_WEIGHTS 					  	- Should the weights be zeros between the epoches. Default: *Yes*
	uint LOWER_BOUND 					  	- A given lower bound of the chromatic number. Default: *2*
	uint MAX_ITER 					  		- Maximum number of iterations. Default: *1000000000*
	uint MAX_NO_IMPROVE 					- Maximum number of not improving movements before a weights update. Default: *2*
	uint LAMBDA 					  		- Coefficient for combining the conflicts and guidance scores. Default: *10*
	uint DYNAMIC_LAMBDA 					- Sets lambda dynamiclly to the average conflicts decrease before guidance is used.
	uint ASPIRATION 						- Enables the aspiration moves. Default: *Yes*
	uint TIMEOUT 							- Maximum execution time of GLS in seconds. Default: *120*
	uint DEBUG 								- Bitwise AND mask of debug levels. Default: *0*
	*/
	BuildStrategy BUILD_STRATEGY = BuildStrategy::Greedy; 
	EpocheStrategy UPDATE_STRATEGY = EpocheStrategy::Merge;
	EpocheTarget SOURCE_TARGET = EpocheTarget::Minimal;
	EpocheTarget DESTINATION_TARGET = EpocheTarget::Maximal;
	ColoringUpperBoundMethod UPPER_BOUND = ColoringUpperBoundMethod::Theorem2;
	uint RESET_WEIGHTS = 1;
	uint LOWER_BOUND = 2;
	uint MAX_ITER = 0;
	uint MAX_NO_IMPROVE = 2;
	uint LAMBDA = 10;
	uint DYNAMIC_LAMBDA = 1;
	uint ASPIRATION = 1;
	uint FAST_SEARCH = 1;
	uint TIMEOUT = 120;
	uint DEBUG = 0;
	
	/* Constants: Debug levels
	uint DEBUG_OUTPUT 					- Output the best found graph coloring.
	uint DEBUG_EPOCHE 					- Log the start and end attributes of the sequential epoches.
	uint DEBUG_SOLUTION 				- Log the commulative attributes of the soluvin provess.
	uint DEBUG_MINIMUM 					- Log the attributes at a local minimum.
	uint DEBUG_MOVES 					- Log the attributes of an applied move.
	*/
	const uint DEBUG_OUTPUT 	= 1;
	const uint DEBUG_EPOCHE 	= 2;
	const uint DEBUG_SOLUTION 	= 4;
	const uint DEBUG_MINIMUM 	= 8;
	const uint DEBUG_MOVES 		= 16;
	
	/* Constants: Node status
	uint NODE_ALLOWED 						- The node is allowed for searching.
	uint NODE_MARKED 						- The node color was marked in fast search.
	*/
	const uint NODE_ALLOWED 		= 0;
	const uint NODE_MARKED 			= 1;
	
	/* Function: upair_comparator
	Comparator for two elements of upairs.
	*/
	inline bool upair_comparator(upair i, upair j) {
		return (i.second > j.second); 
	}
	
	/* Function: init
	Initialize the solver by reading the .ini config file.
	*/
	void init(){	
		#ifdef CONFIG
		CSimpleIniA ini;
		ini.SetUnicode();
		ini.LoadFile(CONFIG); 
		
		BUILD_STRATEGY = static_cast<BuildStrategy>(atoi(ini.GetValue("gls", "BUILD_STRATEGY", "1")));
		UPDATE_STRATEGY = static_cast<EpocheStrategy>(atoi(ini.GetValue("gls", "UPDATE_STRATEGY", "1")));
		SOURCE_TARGET = static_cast<EpocheTarget>(atoi(ini.GetValue("gls", "SOURCE_TARGET", "1")));
		DESTINATION_TARGET = static_cast<EpocheTarget>(atoi(ini.GetValue("gls", "DESTINATION_TARGET", "2")));
		UPPER_BOUND = static_cast<ColoringUpperBoundMethod>(atoi(ini.GetValue("gls", "UPPER_BOUND", "1")));
		RESET_WEIGHTS = atoi(ini.GetValue("gls", "RESET_WEIGHTS", "1"));
		DYNAMIC_LAMBDA = atoi(ini.GetValue("gls", "DYNAMIC_LAMBDA", "0"));
		LOWER_BOUND = atoi(ini.GetValue("gls", "LOWER_BOUND", "2"));
		MAX_ITER = atoi(ini.GetValue("gls", "MAX_ITER", "0"));
		MAX_NO_IMPROVE = atoi(ini.GetValue("gls", "MAX_NO_IMPROVE", "5"));
		LAMBDA = atoi(ini.GetValue("gls", "LAMBDA", "10"));
		FAST_SEARCH = atoi(ini.GetValue("gls", "FAST_SEARCH", "1"));
		ASPIRATION = atoi(ini.GetValue("gls", "ASPIRATION", "1"));
		TIMEOUT = atoi(ini.GetValue("gls", "TIMEOUT", "120"));
		DEBUG = atoi(ini.GetValue("gls", "DEBUG", "0"));
		if(BUILD_STRATEGY != BuildStrategy::RandomStart && UPDATE_STRATEGY == EpocheStrategy::Scratch){
			std::cout << "Scratch is supported only for random start" << std::endl; 
			exit(1);
		}
		#endif
		
		#ifdef DYNAMIC_LAMBDA_ENABLE
		if(DYNAMIC_LAMBDA > 0 && RESET_WEIGHTS == 0){
			std::cout << "Dynamic lambda and penalty keeping can not be applied together" << std::endl; 
			exit(1);
		}
		#endif
	}
	
	/*
	 * ==========
	 * Structures
	 * ==========
	 */
	
	/* Struct: Score
	GLS score of a coloring in a given moment.
	*/
	struct Score{
		/* Field: conflicts
		Conflicts of the coloring.
		*/
		uint conflicts = 0;
		/* Field: guidance
		Guidance of the coloring.
		*/
		uint guidance = 0;
		/* Field: total
		Total score of the coloring. For implementation reasons it is equal to 10*conflicts + LAMBDA * guidance.
		*/
		uint total = 0;
	};
	
	/* Struct: Move
	GLS iteration move
	*/
	struct Move{
		/* Field: node
		Which node to update.
		*/
		NodeID node;
		/* Field: to
		In which color to make the node.
		*/
		color to;
		/* Field: score
		Score of the move
		*/
		Score score;
	};
	
	#ifdef DEBUGGING
	/* Struct: SolveReport
	Report of GLS performance
	*/
	struct SolveReport{
		/* Field: start
		Starting score
		*/
		Score start;
		
		/* Field: end
		Ending score
		*/
		Score end;
		
		/* Field: created
		Timestamp of the start of the report
		*/
		uint created = 0;
		
		/* Field: created
		Timestamp of the finishing moment of the report
		*/
		uint finished = 0;
		
		/* Field: iters
		Iterations
		*/
		uint iters = 0;
		
		/* Field: updates
		Weight updates
		*/
		uint updates = 0;
		
		/* Field: improvements
		How many improvements were made
		*/
		uint improvements = 0;
		
		/* Field: aspirations
		How many aspirations were made
		*/
		uint aspirations = 0;
		
		/* Field: mins
		How many times there was a local minimum
		*/
		uint mins = 0;
		
		/* Method: print
		Print the report to stdout
		*/
		void print(const color &k = 0){
			std::cout << k << ",";
			std::cout << iters << ",";
			std::cout << finished - created << ",";
			std::cout << improvements << ",";
			std::cout << mins << ",";
			std::cout << updates << ",";
			std::cout << aspirations << ",";
			std::cout << start.conflicts << ",";
			std::cout << start.guidance << ",";
			std::cout << start.total << ",";
			std::cout << end.conflicts << ",";
			std::cout << end.guidance << ",";
			std::cout << end.total << std::endl;
		}
	};
	#endif 

	/*
	 * =======
	 * Classes
	 * =======
	 */
	/*
	Class: ColoringUpperBound
	This class calculates the upper bound of the chromatic number of a given graph.
	
	This class is based on the following work:
	<http://www.sciencedirect.com/science/article/pii/S0166218X11003039>
	*/
	class ColoringUpperBound{
		private:
		/*
		Method: get_degrees
		Calculate the degrees of a graph.
		*/
		static upairs get_degrees(const graph_access &G){
			upairs degrees = upairs();
			
			NodeID N = G.number_of_nodes();
			for(NodeID n = 0; n < N; ++n) { 
				uint degree = G.get_first_invalid_edge(n) - G.get_first_edge(n);
				degrees.push_back(std::make_pair(n, degree));
			}
			
			return degrees;
		}
		public:
		/*
		Method: simple
		Apply Brooks' theorem.
		*/
		static uint simple(const graph_access &G){
			upairs degrees = get_degrees(G);
			std::sort(degrees.begin(), degrees.end(), upair_comparator);
			return degrees[0].second + 1;
		}
		
		/*
		Method: theorem2
		Apply Theorem 2 in the referenced work.
		*/
		static uint theorem2(const graph_access &G){
			upairs degrees = get_degrees(G);
			std::sort(degrees.begin(), degrees.end(), upair_comparator);
			
			uint result = 0; 
			for(upair d: degrees){
				if(d.second >= result){ result++; } 
				else { break; }
			}
			
			return (result > 0) ? result : 1;
		}
		
		/*
		Method: theorem3
		Apply Theorem 3 in the referenced work.
		*/
		static uint theorem3(const graph_access &G){
			upairs degrees = get_degrees(G);
			upairs rhos = upairs();
			
			uint N = G.number_of_nodes();
			uint until, rho;
			
			for(NodeID v = 0; v < N; ++v){
				upairs tab = upairs();
				for(NodeID u: G.neighbours(v)){
					tab.push_back(degrees[u]);
				}
				
				std::sort(tab.begin(), tab.end(), upair_comparator);
				
				rho = 0;
				until = tab.size();
				
				for(NodeID j = 0; j < until; ++j){
					if(tab[j].second > rho){ rho++; } 
					else { break; }
				}
				
				rhos.push_back(std::make_pair(v, rho));
			}
			
			std::sort(rhos.begin(), rhos.end(), upair_comparator);
			
			uint result = 0; 
			for(upair d: rhos){
				if(d.second >= result){ result++; } 
				else { break; }
			}
			
			return (result > 0) ? result : 1;
		}
		
		/*
		Method: calculate
		Interface to combine the methods and the configuration field UPPER_BOUMD
		*/
		color calculate(const graph_access &G){
			switch(UPPER_BOUND){
				case ColoringUpperBoundMethod::Simple:
					return simple(G);
				case ColoringUpperBoundMethod::Theorem2:
					return theorem2(G);
				case ColoringUpperBoundMethod::Theorem3:
					return theorem3(G);
				default:
					std::cout << "Coloring upper bound method not implemented" << std::endl;
					exit(1);
			}
		}
	};
	
	
	/*
	Class: ColoringBuilder
	Initial coloring builder
	Supported methods: greedy, bipartite, random
	*/
	class ColoringBuilder{
		private:
		ColoringUpperBound bound;
		
		public:
		ColoringBuilder(){
			bound = ColoringUpperBound();
		}
		
		/*
		Method: random
		Return a random coloring of the graph using at most k colors.
		*/
		static colors random(const graph_access &G, color k){
			const NodeID N = G.number_of_nodes();
			colors result = colors();
			for(NodeID i = 0; i < N; ++i){
				result.push_back(rand() % k);
			}
			return result;
		}
		
		/*
		Method: bipartite
		Return a coloring of the graph using at most 2 colors by applying a modfied DFS.
		Visit: <http://www.techiedelight.com/determine-given-graph-bipartite-graph-using-dfs/>
		*/
		static colors bipartite(const graph_access &G){
			NodeID N = G.number_of_nodes();
			int next;
			int* bi = new int[N];
			for(NodeID s = 0; s < N; ++s){ bi[s] = 0; }
		
			std::stack<std::pair<NodeID, int>> S = std::stack<std::pair<NodeID, int>>();
			for(NodeID s = 0; s < N; ++s){
				if(bi[s] > 0){ continue; }
				S.push(std::make_pair(s, 1));
				while (S.size() > 0){
					std::pair<NodeID, int> m = S.top();
					S.pop();
					if(bi[m.first] > 0){continue;}
					bi[m.first] = m.second;
					next = (m.second == 1) ? 2 : 1;
					for(NodeID v: G.neighbours(m.first)){
						if(bi[v] > 0){continue;}
						S.push(std::make_pair(v, next));
					}
				}
			}
			
			colors result = colors();
			for(NodeID s = 0; s < N; ++s){
				result.push_back(bi[s] - 1);
			}
		
			delete [] bi;
		
			return result;
		}
		
		/*
		Method: greedy
		Return a coloring of the graph using a simple greedy strategy.
		Visit: <http://www.geeksforgeeks.org/graph-coloring-set-2-greedy-algorithm/>
		*/
		static colors greedy(const graph_access &G){
			NodeID N = G.number_of_nodes();
			
			color null = -1;
			color result[N];
			bool available[N];
			
			for (NodeID v = 0; v < N; ++v){ 
				available[v] = false;
				result[v] = -1; 
			}
			
			result[0] = 0;
	
			for (NodeID v = 1; v < N; v++){
				for (NodeID u : G.neighbours(v)) {
					if(result[u] != null){
						available[result[u]] = true;
					}
				}
				
				NodeID cr;
				for (cr = 0; cr < N; ++cr){
					if (available[cr] == false){
						break;
					}
				}
				
				result[v] = cr;
				
				for (NodeID u : G.neighbours(v)) {
					if(result[u] != null){
						available[result[u]] = false;
					}
				}
			}
			
			colors coloring = colors(); 
			for (NodeID v = 0; v < N; ++v){
				coloring.push_back(result[v]);
			}
			
			return coloring;
		}
		
		colors build(const graph_access &G){
			switch(BUILD_STRATEGY){
				case BuildStrategy::RandomStart:
					return random(G, bound.calculate(G));
				case BuildStrategy::Greedy:
					return greedy(G);
				case BuildStrategy::Bipartite:
					return bipartite(G);
				default:
					std::cout << "Not supported build method" << std::endl;
					exit(1);
			}
		}
	};
	
	/*
	Class: GuidedLocalSearch
	Implementation of the Guided Local Search Algorithm
	
	Visit: <http://www.cleveralgorithms.com/nature-inspired/stochastic/guided_local_search.html>
	
	Improvements that are added:
    
	- Dynamic optimization of the restricted one exchange neighbourhood
    - Aspiration movements
	- Optional external reset of the weights
	- Execution timeout
	*/
	class GuidedLocalSearch {
	private:
		/*
		 Field: N
		 Number of nodes
		 */
		uint N;
		
		/*
		 Field: M
		 Number of edges
		 */
		uint M;
		
		/*
		 Field: K
		 Number of colors
		 */
		uint K;
		
		/*
		 Field: iters
		 Number of iterations
		 */
		uint iters;
		
		/*
		 Field: start
		 Start moment of the first epoche
		 */
		uint start;
		
		/*
		 Field: solution
		 The best found so far solution in the current epoche
		 */
		colors solution;
		
		/*
		 Field: solution_score
		 The score of the best found so far solution in the current epoche
		 */
		Score solution_score;
		
		/*
		 Field: conflicts
		 Array with the conflicts with size K * N
		 */
		uint* conflicts;
		
		/*
		 Method: update_conflicts
		 Updates the conflicts structure according to a given coloring
		 */
		void update_conflicts(const graph_access &G, const colors &coloring){
			for(NodeID v = 0; v < N; ++v) {
				for(uint i = 0; i < K; ++i){
					conflicts[K*v + i] = 0;
				}		
				for (NodeID u : G.neighbours(v)) {
					conflicts[K*v + coloring[u]]++;
				}
			}
		}
		
		/*
		 Field: weights
		 Array with the current weights of the edges
		 */
		uint* weights;
		
		/*
		 Method: clear_weights
		 Sets the guidance weights to zeros.
		*/
		void clear_weights(){
			W = 0;
			solution_score.guidance = 0;
			for(uint i = 0; i < M; ++i){ weights[i] = 0; }
		}
		
		/*
		 Method: update_weights
		 Calculates the utilites of the edges and then increment the value of the weights with 1 for edges having maximal utility.
		*/
		void update_weights(const graph_access &G, const colors &coloring){
			update_indicators(G, coloring);
			
			std::vector<EdgeID> updates = std::vector<EdgeID>();
			
			float max = 0, utility = 0;
			for(EdgeID e = 0; e < M; ++e){
				utility = float(indicators[e]) / float(1 + weights[e]);
				
				if(utility > max){
					max = utility;
					updates = std::vector<EdgeID>();
				}
				
				if(utility == max){
					updates.push_back(e);
				}
			}
			
			for(EdgeID e: updates){
				weights[e]++;
			}
			
			W += updates.size() / 2;
		}
		
		/*
		 Field: W
		 Sum of the weights
		*/
		uint W = 0;
		
		/*
		 Field: indicators
		 Array, that if has value 1, shows that an edge is connecting two nodes with the same color in the solution. It has value 0 otherwise. 
		*/
		uint* indicators; 
		
		/*
		 Method: update_indicators
		 Fills the indicators array with the corresponding node conflicts.
		*/
		void update_indicators(const graph_access &G, const colors &coloring){
			for(NodeID v = 0; v < N; ++v){
				EdgeID until = G.get_first_invalid_edge(v);
				for(EdgeID e = G.get_first_edge(v); e < until; ++e){
					NodeID u = G.getEdgeTarget(e);
					indicators[e] = (coloring[u] == coloring[v]) ? 1 : 0;
				}
			}
		}
		
		#ifdef FAST_SEARCH_ENABLE
		/*
		 Field: fast_search
		 Array, used for fast search, filled with node status values.
		*/
		uint* fast_search; 
		
		/*
		 Method: clear_fast_search
		 Initialize the fast search with zeros
		*/
		void clear_fast_search(){
			for(NodeID v = 0; v < N; v++){
				for(color c = 0; c < K; c++){
					fast_search[K*v + c] = NODE_ALLOWED;
				}
			}
		}
		#endif
		
		/*
		 Method: build_score
		 Crerates a new score by combining given conflicts and guidance scores.
		*/
		inline Score build_score(const uint conflicts, const uint guidance){
			Score result;
			result.conflicts = conflicts;
			result.guidance = guidance;
			result.total = (W) ? 10 * conflicts + LAMBDA * guidance : 10 * conflicts;
			return result;
		}
		
		/*
		 Method: score_conflicts
		 Uses the conflicts neigbourhood array to calculate the conflicts of the nodes in a given colorung. 
		*/
		uint score_conflicts(const colors &coloring){
			uint result = 0;
			for(NodeID v = 0; v < N; v++){
				result += conflicts[K*v + coloring[v]];
			}
			return result / 2;
		}

		/*
		 Method: score_guidance
		 Uses the edge indicators and the edge weights arrays to calculate the guidance for a given colorung. 
		*/
		uint score_guidance(){
			if(W==0){
				return 0;
			}
			uint result = 0;
			for (EdgeID e = 0; e < M; ++e){
				result += indicators[e] * weights[e]; 
			}
			return result / 2;
		}
		
		/*
		 Method: make_move
		 Applies the next move to a given colring of the graph G with known score.
		 
		 Additionally, it applies the move to the conflicts array.
		*/
		void make_move(const graph_access &G, colors &coloring, Score &score, Move next){
			uint old = coloring[next.node];
			for (NodeID u : G.neighbours(next.node)) {
				conflicts[K*u + old]--;
				conflicts[K*u + next.to]++;
				#ifdef FAST_SEARCH_ENABLE
				if(FAST_SEARCH){
					for(color c=0;c<K;c++){
						fast_search[K * u + c] = NODE_ALLOWED;
					}
				}
				#endif
			}
			
			#ifdef FAST_SEARCH_ENABLE
			if(FAST_SEARCH){
				fast_search[K * next.node + coloring[next.node]] = NODE_MARKED;
			}
			#endif
			
			score.conflicts = next.score.conflicts;
			score.guidance = next.score.guidance;
			score.total = next.score.total;
			
			coloring[next.node] = next.to;
			
			#ifdef DEBUGGING
			if(DEBUG & DEBUG_MOVES){
				std::cout << "MOVE," << iters << "," << next.node << "," << next.to << "," << score.conflicts << "," << score.guidance << "," << score.total << std::endl;
			}
			#endif
		}
		
		/*
		 Method: best_neighbours
		 Searches the neighbourhood of a given coloring of graph G with known score.
         
		 Optionally, it can apply the augumented function.  
		*/
		std::vector<Move> best_neighbours(const graph_access &G, const colors coloring, Score score, bool with_guidance){
			std::vector<Move> result = std::vector<Move>();
			std::vector<Move> aspired = std::vector<Move>();
			Score best = score;
			Score eval;
			
			uint conf = 0, guid = 0;
			uint min_conf = -1;
			for(NodeID v = 0; v < N; ++v){
				if(conflicts[K*v + coloring[v]] == 0){
					continue;
				}
				
				for(uint c = 0; c < K; c++){
					#ifdef FAST_SEARCH_ENABLE
					if(FAST_SEARCH && fast_search[K*v + c] != NODE_ALLOWED){
						continue;
					}
					#endif
					if(c == coloring[v]){
						continue;
					}
					
					conf = score.conflicts + conflicts[K*v + c] - conflicts[K*v + coloring[v]];
					if(with_guidance){
						uint g_add = 0, g_remove = 0;
						
						EdgeID until = G.get_first_invalid_edge(v);
						for(EdgeID e = G.get_first_edge(v); e < until; e++){
							NodeID u = G.getEdgeTarget(e);
							
							if(coloring[u] == coloring[v]){
								g_remove += weights[e];
							}
							
							if(coloring[u] == c){
								g_add += weights[e];
							}
						}
						
						guid = score.guidance + g_add - g_remove;
					}
					
					eval = build_score(conf, guid);
					
					if(eval.total < best.total){
						best = eval;
						result = std::vector<Move>();
					}
					
					if(eval.total == best.total){
						Move next;
						next.node = v;
						next.to = c;
						next.score = eval;
						result.push_back(next);
					}
					
					if(ASPIRATION){
						if(min_conf > eval.conflicts){
							min_conf = eval.conflicts;
							aspired = std::vector<Move>();
						}
						
						if(eval.conflicts == min_conf && eval.conflicts < solution_score.conflicts && eval.total > score.total){
							Move next;
							next.node = v;
							next.to = c;
							next.score = eval;
							aspired.push_back(next);
						}
					}
				}
			}
			
			if(aspired.size() > 0){
				#ifdef DEBUGGING
				if(DEBUG){
					epoche_report.aspirations++;
				}
				#endif
				return aspired;
			}
			
			return result;
		}
		
		public:
		#ifdef DEBUGGING
		/*
		 Field: epoche_report
		 Struct for reporting the epoche.
		*/
		SolveReport epoche_report;
		
		/*
		 Field: solution_report
		 Struct for reporting the solution.
		*/
		SolveReport solution_report;
		#endif
		
		
		/*
		 Method: GuidedLocalSearch
		 Constructs a new solver, by setting the timer for the timeout.
		*/
		GuidedLocalSearch(){
			start = std::time(nullptr);
		}
		
		/*
		 Method: ~GuidedLocalSearch
		 Destructs a solver, by freeing the memory for the internal arrays.
		*/
		~GuidedLocalSearch(){
			delete [] conflicts;
			delete [] weights;
			delete [] indicators;
			#ifdef FAST_SEARCH_ENABLE
			delete [] fast_search;
			#endif
		}
		
		void prepare(const graph_access &G, const colors &coloring, const uint &k){
			N = G.number_of_nodes();
			M = G.number_of_edges();
			K = k;
			
			conflicts = new uint[K * N];
			indicators = new uint[M];
			#ifdef FAST_SEARCH_ENABLE
			fast_search = new uint[K * N];
			#endif
			weights = new uint[M];
			clear_weights();
		}
		
		/*
		 Method: solve
		 Applies the GLS strategy to solve the k-coloring problem for an initial coloring of a graph G. 
		*/
		colors solve(const graph_access &G, const colors &coloring, const uint k){
			K = k;
			if(DEBUG & DEBUG_MOVES){
				std::cout << "START," << iters << "," << k << std::endl;
			}
			
			if(RESET_WEIGHTS){ 
				clear_weights(); 
			}
			
			update_conflicts(G, coloring);
			update_indicators(G, coloring);
			
			#ifdef FAST_SEARCH_ENABLE
			clear_fast_search();
			#endif
			
			solution = coloring;
			solution_score = build_score(score_conflicts(solution), score_guidance());
			
			uint first_update_total = 0;
			uint first_update_iters = 0;
			
			uint no_improve = 0;
			colors improvement = solution;
			Score score = solution_score;
			SolveResolution resolution = SolveResolution::NotFound;
			
			#ifdef DEBUGGING
			if(DEBUG & DEBUG_EPOCHE){
				epoche_report.created = std::time(nullptr);
				epoche_report.start = solution_score;
				epoche_report.improvements = 0;
				epoche_report.aspirations = 0;
				epoche_report.updates = 0;
				epoche_report.mins = 0;
				epoche_report.iters = 0;
			}
			
			if(DEBUG & DEBUG_SOLUTION){
				if(solution_report.iters == 0){
					solution_report.created = std::time(nullptr);
				}
				solution_report.start.conflicts += solution_score.conflicts;
				solution_report.start.guidance += solution_score.guidance;
				solution_report.start.total += solution_score.total;
			}
			#endif
			
			while(resolution == SolveResolution::NotFound){
				iters++;
				#ifdef DEBUGGING
				if(DEBUG){
					epoche_report.iters++;
				}
				#endif
				
				if(solution_score.conflicts == 0){
					resolution = SolveResolution::Solved;
					solution = improvement;
					break;
				}
				
				std::vector<Move> moves = best_neighbours(G, improvement, score, W > 0);
				if(moves.size() == 0){
					resolution = SolveResolution::LocalMin;
					#ifdef DEBUGGING
					if(DEBUG){
						epoche_report.mins++;
					}
					#endif
				} else {
					Move next = moves[rand() % moves.size()];
					no_improve = (score.total == next.score.total) ? no_improve + 1 : 0;
					
					#ifdef DYNAMIC_LAMBDA_ENABLE
						if(DYNAMIC_LAMBDA && W == 0){
							first_update_iters++;
							first_update_total += score.total - next.score.total;
						}
					#endif
					
					make_move(G, improvement, score, next);
					
					if(no_improve == MAX_NO_IMPROVE){
						resolution = SolveResolution::NoImprove;
					}
					
					if(score.conflicts < solution_score.conflicts){
						solution = improvement;
						solution_score = score;
						#ifdef DEBUGGING
						if(DEBUG){
							epoche_report.improvements++;
							if(DEBUG & DEBUG_MOVES){
								std::cout << "IMPROVE," << iters << "," << solution_score.conflicts << "," << solution_score.guidance << "," << solution_score.total << std::endl;
							}
						}
						#endif
					}
				}
				
				if(MAX_ITER && iters > MAX_ITER){
					resolution = SolveResolution::MaxIterations;
				}
				
				if(resolution == SolveResolution::NoImprove || resolution == SolveResolution::LocalMin){
					#ifdef DEBUGGING
					if(DEBUG){
						if(DEBUG & DEBUG_MINIMUM){
							std::cout << "MIN," << iters << std::endl;
						}
						epoche_report.updates++;
					}
					#endif
					
					#ifdef DYNAMIC_LAMBDA_ENABLE
						if(DYNAMIC_LAMBDA > 0 && W == 0){
							if(first_update_iters){
								LAMBDA = first_update_total / first_update_iters;
								if (LAMBDA == 0){ LAMBDA = 10; }
							} else {
								LAMBDA = 10;
							}
						}
					#endif
					
					update_weights(G, improvement);
					score = build_score(score.conflicts, score_guidance());
					no_improve = 0;
					resolution = SolveResolution::NotFound;
				}
				
				
				if(TIMEOUT && std::time(nullptr) - start > TIMEOUT){
					resolution = SolveResolution::Timeout;
				}
				
				if(solution_score.conflicts == 0){
					resolution = SolveResolution::Solved;
					solution = improvement;
				}
			}
			
			#ifdef DEBUGGING
			if(DEBUG & DEBUG_EPOCHE){
				epoche_report.end = solution_score;
				epoche_report.finished = std::time(nullptr);
				epoche_report.print(K);
			}
			
			if(DEBUG & DEBUG_SOLUTION){
				if(resolution == SolveResolution::Solved){
					solution_report.end.conflicts = 0;
					solution_report.end.guidance = 0;
					solution_report.end.total = 0;
					solution_report.finished = std::time(nullptr);
					solution_report.iters += epoche_report.iters;
					solution_report.mins += epoche_report.mins;
					solution_report.updates += epoche_report.updates;
					solution_report.aspirations += epoche_report.aspirations;
					solution_report.improvements += epoche_report.improvements;
				} else {
					solution_report.end.conflicts = score.conflicts;
					solution_report.end.guidance = score.guidance;
					solution_report.end.total = score.total;
				}
			}
			#endif
			
			return solution;
		}
	};


	/*
	 Class: EpocheRunner
	 Master of the Guided Local Search
	 
	 First a detection if the graph is bipartite is made. If it is, then it returns the coloring with 2 colors.
	 
	 Then an upper bound of the chromatic number estimation is made.
	 A cycle, that starts from this bound and decreases it on every step (epoche), begins.
	 
	 On every step a call to the guided local search is made.
	 
	 If the result in the current step has no conflicts, the cycle continues.
	 If the result contains conflicts, the cycle stops and returns the result from the previous step.
	 */
	class EpocheRunner{
	private:
		/*
		 Method: evaluate
		 Counts the conflicting nodes in a coloring of a graph G.
		*/
		uint evaluate(const graph_access &G, const colors &coloring){
			uint result = 0;
			NodeID N = G.number_of_nodes();
			
			for(NodeID v = 0; v < N; ++v) {
				for (NodeID u : G.neighbours(v)) {
					if (u < v){ continue; }
					
					if(coloring[u] == coloring[v]){
						result++;
					}
				}
			}
			
			return result;
		}
		
		/*
		 Method: groups
		 Finds the number of nodes grouped by color in a given k-coloring.
		*/
		colors groups(const colors &coloring, const uint k){
			uint* groups = new uint[k];
			for(color i = 0; i < k; i++){ groups[i] = 0; }
			for(color c: coloring){ groups[c]++; }
			upairs sorted = upairs(); 
			for(color i = 0; i < k; i++){ 
				sorted.push_back(std::make_pair(i, groups[i]));
			}
			
			std::sort(sorted.begin(), sorted.end(), upair_comparator);
			
			colors result = colors();
			for(color i = 0; i < k; i++){
				result.push_back(sorted[i].first);
			}
			return result;
		}
		
		/*
		 Method: filter
		 Merges two color classes in a k-coloring of the graph G.
		 
		 The resulting coloring is granted to have at most k-1 colors, if k is correct.
		 Otherwise the coloring will be truncated.
		*/
		colors filter(const graph_access &G, const colors &coloring, color k){
			if(get_colors(coloring) <= k - 1 && evaluate(G, coloring) == 0){
				return coloring;
			}
			
			if(UPDATE_STRATEGY == EpocheStrategy::Scratch){
				return ColoringBuilder::random(G, k-1);
			}
			
			colors sorted = groups(coloring, k);
			
			color src;
			switch(SOURCE_TARGET){
				case EpocheTarget::Random:
					src = sorted[rand() % k];
					break;
				case EpocheTarget::Minimal:
					src = sorted[k-1];
					break;
				case EpocheTarget::Maximal:
					src = sorted[0];
					break;
				case EpocheTarget::Median:
					src = sorted[k/2];
					break;
				default:
					std::cout << "Invalid source" << std::endl;
					exit(1);
			}
	
			colors result = colors();
			if(DESTINATION_TARGET == EpocheTarget::Random){
				for(color c: coloring){
					while(c == src){ c  = rand() % k; }
					if(c > src){c--;}
					result.push_back(c);
				}
			} else {
				color dest;
				switch(DESTINATION_TARGET){
					case EpocheTarget::Minimal:
						dest = (SOURCE_TARGET == EpocheTarget::Minimal) ? sorted[k-2] : sorted[k-1];
						break;
					case EpocheTarget::Maximal:
						dest = (SOURCE_TARGET == EpocheTarget::Maximal) ? sorted[1] : sorted[0];
						break;
					case EpocheTarget::Median:
						dest = (SOURCE_TARGET == EpocheTarget::Median) ? sorted[k/2 - 1] : sorted[k/2];
						break;
					default:
						std::cout << "Invalid destination" << std::endl;
						exit(1);
				}
		
				for(color c: coloring){
					if(c == src){c = dest;}
					if(c > src){c--;}
					result.push_back(c);
				}
			}
			
			return result;
		}
		
	public:
		/*
		 Method: get_colors
		 Finds the biggest number in a coloring.
		*/
		inline static color get_colors(const colors &coloring){
			color k = 0;
			for (color c: coloring){
				if(k < c){ k = c; }
			}
			return ++k;
		}
		
	    /*
		 Method: solve
		 Applies the GLS strategy iteratively, trying yo minimize the number of the colors in a given coloring of the graph G.
		
		 The input graph and coloring are not changed.
		*/
		colors solve(const graph_access &G, const colors &coloring){
			colors bi = ColoringBuilder::bipartite(G);
			if(evaluate(G, bi) == 0){
				return bi;
			}
			
			ColoringUpperBound bound = ColoringUpperBound();
			color K = bound.calculate(G);
			if(K < get_colors(coloring)){
				K = get_colors(coloring);
			}
			
			if(LOWER_BOUND == 2){LOWER_BOUND++;}
			
			// If the given coloring contains has a color bigger than the upper bound, set it to zero.
			// This is the case random coloring with 10000 the biggest color and K = 10
			colors result = colors();
			for(color c: coloring){
				if (c >= K){ result.push_back(0); }
				else{ result.push_back(c); }
			}
			colors filtered = result; 
			
			GuidedLocalSearch solver = GuidedLocalSearch();
			solver.prepare(G, filtered, K);
			
			color k;
			for (k = K; k >= LOWER_BOUND; k--){
				colors solution = solver.solve(G, filtered, k);
				
				if(evaluate(G, solution) == 0){
					result = solution;
				} else {
					break;
				}
				
				filtered = filter(G, result, k);
			}
			
			#ifdef DEBUGGING
			if(DEBUG & DEBUG_SOLUTION){
				solver.solution_report.print(get_colors(result));
			}
			#endif
			
			if (DEBUG & DEBUG_OUTPUT){
				if(evaluate(G, result) == 0){
					NodeID N = G.number_of_nodes();
					std::cout << N << " " << G.number_of_edges() << std::endl;
					for(NodeID v = 0; v < N; v++){
						std::cout << result[v] << " ";
						for(NodeID u: G.neighbours(v)){
							std::cout << u + 1 << " ";
						}
						std::cout << std::endl;
					}					
				} else {
					std::cout << "NO" << std::endl;
				}
			}
			
			return result;
		}
	};
}

/*
// If the coloring is with signed ints
std::vector<int> GuidedLocalSearch(const std::vector<int> &s, const graph_access &G){
	gls::EpocheRunner solver = gls::EpocheRunner();
	gls::colors coloring;
	for(int c: s){coloring.push_back(c);}
	gls::colors solution = solver.solve(G, coloring);
	std::vector<int> result = std::vector<int>();
	for(gls::color c: solution){result.push_back(c);}
	return result;
}
*/

gls::colors GuidedLocalSearch(const gls::colors &s, const graph_access &G){
	gls::EpocheRunner solver = gls::EpocheRunner();
	return solver.solve(G, s);
}