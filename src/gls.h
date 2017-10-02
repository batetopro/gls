// Turn on dynammic lambda heuristic
//#define DYNAMIC_LAMBDA_ENABLE 1

// Turn on move queue
#define MOVE_QUEUE_ENABLE 1

// Turn on moves queue debugging 
// #define DEBUG_QUEUE 1

// Use .ini file configuration
#include "../lib/SimpleIni.h"
#define CONFIG "gls.ini"

#include <algorithm>
#include <vector>
#include <queue>
#include <stack>
#include <set>
#include <ctime>
#include <limits>
#include <chrono>
#define SENTINEL(T) (T{})


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
	/* Type: ulong
	Alias for unisgned long.
	*/
	typedef unsigned long ulong;
	
	/* Type: color
	Alias for a int, representing a color.
	*/
	typedef uint color;
	
	/* Type: delta
	Alias for a signed int, representing an improvement or a score structure.
	*/
	typedef signed short delta;
	
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
	
	/* Function: upair_comparator
	Comparator for two elements of upairs.
	*/
	inline bool upair_comparator(upair i, upair j) {
		return (i.second > j.second); 
	}
	
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
	uint MAX_PLATAEU 						- Maximum number of not improving movements before a weights update. Default: *2*
	uint MAX_NO_IMPROVE 					- Maximum number of not improving movements before termination. Default: *20000*
	delta LAMBDA 					  		- Coefficient for combining the conflicts and guidance scores. Default: *10*
	uint MOVE_QUEUE 					  	- Enables the priority queues. Default: *1*
	uint HEAD_CAPACITY 					  	- Capacity of the items fetched in the priority head. Default: *1*
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
	uint MAX_PLATAEU = 2;
	uint MAX_NO_IMPROVE = 20000;
	delta LAMBDA = 10;
	uint MOVE_QUEUE=1;
	uint HEAD_CAPACITY=1;
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
		MAX_PLATAEU = atoi(ini.GetValue("gls", "MAX_PLATAEU", "2"));
		MAX_NO_IMPROVE = atoi(ini.GetValue("gls", "MAX_NO_IMPROVE", "20000"));
		LAMBDA = atoi(ini.GetValue("gls", "LAMBDA", "10"));
		MOVE_QUEUE = atoi(ini.GetValue("gls", "MOVE_QUEUE", "1"));
		HEAD_CAPACITY = atoi(ini.GetValue("gls", "HEAD_CAPACITY", "1"));
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
		
		/* Field: total
		Build a score by given conflicts and guidance
		*/
		inline static Score build(const uint &c, const uint &g){
			Score result;
			result.conflicts = c;
			result.guidance = g;
			result.total = (g) ? 10 * c + LAMBDA * g : 10 * c;
			return result;
		}
	};
	
	/* Struct: DeltaScore
	GLS delta score of a coloring in a given and the previous moment.
	*/
	struct DeltaScore{
		/* Field: conflicts
		Conflicts of the coloring.
		*/
		delta conflicts = 0;
		/* Field: guidance
		Guidance of the coloring.
		*/
		delta guidance = 0;
		/* Field: total
		Total score of the coloring. For implementation reasons it is equal to 10*conflicts + LAMBDA * guidance.
		*/
		delta total = 0;
		
		/* Field: total
		Build a score by given conflicts and guidance
		*/
		inline static DeltaScore build(const delta &c, const delta &g){
			DeltaScore result;
			result.conflicts = c;
			result.guidance = g;
			result.total = (g) ? 10 * c + LAMBDA * g : 10 * c;
			return result;
		}
	};
	
	/* Struct: Move
	GLS iteration move
	*/
	struct Move{
		/* Field: ID
		ID of the move = K * node + to
		*/
		uint ID = -1;
		/* Field: node
		Which node to update.
		*/
		NodeID node = 0;
		/* Field: to
		In which color to make the node.
		*/
		color to = 0;
		/* Field: score
		Delta score a.k.a score step of the move.
		*/
		DeltaScore score;
		
		Move(){}
		Move(const NodeID n, const color t, const color K){
			ID = K * n + t;
			node = n;
			to = t;
		}
	};
	
	/* Type: Moves
	Alias for std::vector<Move>.
	*/
	typedef typename std::vector<Move> Moves;
	
	/* Type: MovesQueueNode
	Combination of ID and score.
	*/
	struct MovesQueueNode{
		size_t node;
		DeltaScore score;
		MovesQueueNode(size_t n, DeltaScore s){
			node=n; score=s;
		}
	};
	
	/* Type: MoveTotalCompare
	Compare moves based on their augumented score, ID
	*/
	struct MoveTotalCompare{
		bool operator()(const Move& a, const Move& b) const {
			if (a.score.total == b.score.total){ 
				return a.ID < b.ID;
			}
			return a.score.total < b.score.total;
		}
		
		bool operator()(const MovesQueueNode& a, const MovesQueueNode& b) const {
			return a.score.total <= b.score.total;
		}
		
		bool weak(const MovesQueueNode& a, const MovesQueueNode& b) const {
			return a.score.total < b.score.total;
		}
		
		bool operator()(const DeltaScore& a, const DeltaScore& b) const {
			return a.total <= b.total;
		}
		
		bool weak(const DeltaScore& a, const DeltaScore& b) const {
			return a.total < b.total;
		}
	};
	
	/* Type: MoveConflictsCompare
	Compare moves based on their conflicts score, ID
	*/
	struct MoveConflictsCompare{
		bool operator()(const Move& a, const Move& b) const {
			if (a.score.conflicts == b.score.conflicts){ 
				return a.ID < b.ID;
			}
			return a.score.conflicts < b.score.conflicts;
		}
		
		bool operator()(const MovesQueueNode& a, const MovesQueueNode& b) const {
			return a.score.conflicts <= b.score.conflicts;
		}
		
		bool weak(const MovesQueueNode& a, const MovesQueueNode& b) const {
			return a.score.conflicts < b.score.conflicts;
		}
				
		bool operator()(const DeltaScore& a, const DeltaScore& b) const {
			return a.total <= b.total;
		}
		
		bool weak(const DeltaScore& a, const DeltaScore& b) const {
			return a.total < b.total;
		}
	};
	
	/* Struct: SolveReport
	Report of GLS performance
	*/
	struct SolveReport{
		uint e_iters;
		uint s_iters;
		
		uint e_K;
		uint s_K;
		
		uint e_improvements;
		uint s_improvements;
		
		uint e_aspirations;
		uint s_aspirations;
		
		uint e_updates;
		uint s_updates;
		
		uint e_minimums;
		uint s_minimums;
		
		Score e_start_score;
		Score s_start_score;
		
		Score e_final_score;
		Score s_final_score;
		
		uint first_update_total;
		uint first_update_iters;
		
		std::chrono::high_resolution_clock::time_point e_start;
		std::chrono::high_resolution_clock::time_point s_start;
		uint no_improves;
		
		double time_diff(std::chrono::high_resolution_clock::time_point start){
			std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> time_span = now-start;
			return time_span.count();
		}
		
		void prepare(color K){
			s_K = K;
			
			s_iters=0;
			s_improvements = 0;
			s_aspirations = 0;
			s_start = std::chrono::system_clock::now();
			
			s_start_score.conflicts = 0;
			s_start_score.guidance = 0;
			s_start_score.total = 0;
			
			s_final_score.conflicts = 0;
			s_final_score.guidance = 0;
			s_final_score.total = 0;
		}
		
		void prepare_epoch(color K, Score s){
			e_K = K;
			e_iters = 0;
			e_improvements = 0;
			e_aspirations = 0;
			e_updates = 0;
			e_minimums = 0;
			
			e_start = std::chrono::system_clock::now();
			e_start_score = s;
			s_start_score.conflicts += s.conflicts;
			s_start_score.guidance += s.guidance;
			s_start_score.total += s.total;
			
			first_update_total = 0;
			first_update_iters = 0;
			
			no_improves = 0;
			
			if(DEBUG & DEBUG_MOVES){
				std::cout << "START," << s_iters << "," << K << std::endl;
			}
		}
		
		void skip_epoch(color K){
			s_K = K;
			s_iters++;
			if(DEBUG & DEBUG_SOLUTION){
				std::cout << "SKIP," << K << std::endl;
			}		
		}
		
		void finish_epoch(Score s){
			e_final_score = s;
			
			if(s.conflicts == 0){
				s_K = e_K;
				
				s_final_score.conflicts = 0;
				s_final_score.guidance = 0;
				s_final_score.total = 0;
				
				s_iters += e_iters;
				s_minimums += e_minimums;
				s_updates += e_updates;
				s_aspirations += e_aspirations;
				s_improvements += e_improvements;
			} else {
				s_final_score = s;
			}
			
			if(DEBUG && DEBUG_EPOCHE){
				std::cout << e_K << "," << time_diff(e_start) << "," 
							<< e_improvements << "," << e_minimums << "," << e_updates << "," << e_aspirations << ","
							<< e_start_score.conflicts << "," << e_start_score.guidance << "," << e_start_score.total << ","
							<< e_final_score.conflicts << "," << e_final_score.guidance << "," << e_final_score.total << std::endl;	
			}
		}
		
		void finish_solution(){
			if(DEBUG && DEBUG_SOLUTION){
				std::cout << s_K << "," << time_diff(s_start) << "," 
							<< s_improvements << "," << s_minimums << "," << e_updates << "," << s_aspirations << ","
							<< s_start_score.conflicts << "," << s_start_score.guidance << "," << s_start_score.total << ","
							<< s_final_score.conflicts << "," << s_final_score.guidance << "," << s_final_score.total << std::endl;	
			}
		}
		
		void weight_update(){
			if(DYNAMIC_LAMBDA && e_updates == 0){
				if(first_update_iters){
					delta nl = first_update_total / first_update_iters;
					if(nl > 0){ LAMBDA  = nl; }
				}
			}
			
			no_improves = 0;
			e_updates++;
			
			if(DEBUG & DEBUG_MINIMUM){ 
				std::cout << "MIN," << e_iters + s_iters << std::endl; 
			}
		}
		
		SolveResolution minimum(){
			e_minimums++;
			//std::cout<<":M:"<<std::endl;
			return check(SolveResolution::LocalMin);
		}
		
		SolveResolution check_move(Move next){
			if(next.score.total == 0){no_improves++;}
			else{no_improves=0;}
			if(DYNAMIC_LAMBDA && e_updates == 0){
				first_update_iters++;
				first_update_total += next.score.total;
			}
			return check(SolveResolution::NotFound);
		}
		
		void report_move(Move next, Score s){
			if(DEBUG & DEBUG_MOVES){
				std::cout << "MOVE," << e_iters + s_iters 
							<< "," << next.node << "," << next.to << "," 
							<< s.conflicts << "," << s.guidance << "," << s.total << std::endl;
			}	
		}
		
		
		SolveResolution check(SolveResolution result){
			if(no_improves == MAX_PLATAEU){ 
				result = SolveResolution::NoImprove; 
			}
			
			if(MAX_NO_IMPROVE && no_improves >= MAX_NO_IMPROVE){
				result = SolveResolution::MaxIterations;
			}
			
			if(MAX_ITER && e_iters + s_iters > MAX_ITER){
				result = SolveResolution::MaxIterations;
			}
			
			if(TIMEOUT && time_diff(s_start) > TIMEOUT){
				result = SolveResolution::Timeout; 
			}
			
			return result;
		}
		
		void aspiration(){
			e_aspirations++;
			//std::cout<<":A:"<<std::endl;
		}
		
		void iteration(){
			s_iters++;
		}
		
		void improvement(Score s){
			e_final_score = s;
			e_improvements++;
			if(DEBUG & DEBUG_MOVES){
				std::cout << "IMPROVE," << s_iters << "," << s.conflicts << "," << s.guidance << "," << s.total << std::endl; 
			}					
			//std::cout << "I:" << s.conflicts << ":";
		}
	};
	
	/*
	 * =======
	 * Classes
	 * =======
	 */
	 /*
	Class: MovesQueue
	Priority queue for moves. 
	Realises addressable binary heap with an additional lookup array.
	
	This class is based on the following work:
		https://codereview.stackexchange.com/questions/75539/binary-heap-with-o1-lookup-and-olog-n-change-key
	*/
	template <typename T, typename Cmp = MoveTotalCompare>
	class MovesQueue {
		/*
		Field: index
		Associate each key with an index.
		*/ 
		uint* index;
		
		/*
		Field: elems
		Default representation is as a vector.
		The zero element is awlays SANTINEL(T)
		*/
		std::vector<T> elems;
		
		
		/*
		Field: cmp
		Comparator for sorting with the following meanings
		*/
		Cmp cmp;
		
		uint size=0;
		
		static inline uint parent(uint i) { return i >> 1; }
		static inline uint left(uint i) { return i << 1; }
		static inline uint right(uint i) { return (i << 1) + 1; }
		
		void siftUp(uint i){
			while (i > 1 && cmp(elems[i], elems[parent(i)])) {
				std::swap(elems[i], elems[parent(i)]);
				std::swap(index[elems[i].ID], index[elems[parent(i)].ID]);
				i = parent(i);
			}
		}
		
		// adjusts elems[i] assuming it's been modified to be smaller than its children
		// runs in O(lgn) time, floats elems[i] down
		void siftDown(uint i){
			uint length = elems.size();
			while (true) {
				uint l = left(i), r = right(i);
				uint largest = i;
				
				if (l < length && cmp(elems[l], elems[largest]))
					largest = l;
				
				if (r < length && cmp(elems[r], elems[largest]))
					largest = r;
				
				if (largest == i) { break; }
				std::swap(elems[largest], elems[i]);
				std::swap(index[elems[largest].ID], index[elems[i].ID]);
				i = largest;
			}
		}
		public:
		// construction ------------
		MovesQueue(){
			cmp = Cmp();
		}
		
		~MovesQueue(){
			if(size){
				delete [] index;
			}
		}
		
		void prepare(uint S){
			size = S;
			index = new uint[size];
		}
		
		// query ---------------
		bool empty() const  { return elems.size() <= 1; }
		uint elements() const { return elems.size() - 1; }
		
		// Insert and extract_top are not supported operations
		
		// extraction ----------
		T top() const       	{ return elems[1]; }
		T key(uint ID) const 	{ return elems[index[ID]];}
		
		// runs in O(NlgN) time due to siftDown		
		std::vector<T> extract(){
			std::vector<T> data = std::vector<T>(); 
			
			std::vector<T> original = elems;
			while(!empty()){
				if (elems.size() <= 1) {break;}
				
				T top {elems[1]};
				std::swap(elems[1], elems.back());
				
				elems.pop_back();
				siftDown(1);
				data.push_back(top);
			}
			
			#ifdef DEBUG_QUEUE
			for(uint i = 1; i < data.size(); i++){
				if(!cmp(data[i-1], data[i])){ std::cout << "A"; }
			}
			#endif
			
			elems = original;
			return data;
		}
		
		std::vector<T> top_level(const colors &coloring, delta* conflicts, color &K){
			std::vector<T> moves = std::vector<T>();
			
			size_t C, L, R;
			T mv, best = elems[index[coloring[0]]];
			
			std::stack<size_t> DFS = std::stack<size_t>();
			DFS.push(1);
			while (!DFS.empty()) {
				C = DFS.top(); 
				DFS.pop();
				if (C >= size){ continue; }
				mv = elems[C];
				if (!cmp(mv.score, best.score)){ continue; }
				const uint sourceID = K * mv.node + coloring[mv.node];
				
				if(conflicts[sourceID] > 0 && coloring[mv.node] != mv.to){
					if(cmp.weak(mv.score, best.score)){
						best = mv;
						moves = std::vector<T>();
					}
					
					if(cmp(mv.score, best.score)){
						moves.push_back(mv);
						if(moves.size() > HEAD_CAPACITY){
							return moves;
						}
					}
				}
				
				L = MovesQueue::left(C), R = MovesQueue::right(C);
				if (L < size && cmp(elems[L].score, best.score)){
					DFS.push(L);
				}
				if(R < size && cmp(elems[R].score, best.score)){
					DFS.push(R);
				}
			}
			
			return moves;
		}
		
		// runs in O(HlgH) time due to DFS
		std::vector<T> head(const colors &coloring, delta* conflicts, color &K){
			if (HEAD_CAPACITY == 1){
				std::vector<T> moves = std::vector<T>();
				T mv = top();
				if(conflicts[K*mv.node + coloring[mv.node]] > 0 && coloring[mv.node] != mv.to){ 
					moves.push_back(mv);
				}
				return moves;
			}
			
			return top_level(coloring, conflicts, K);
		}
		
		// modification ----------
		// O(n) like constructor for all elements
		void build(const std::vector<T> &data) {
			size = data.size();
			index = new uint[size];
			std::memset(index, 0, size);
			
			elems = std::vector<T>();
			elems.reserve(size+1);
			elems.push_back(T());
			
			for(size_t i = 0; i < data.size(); i++){
				index[data[i].ID] = elems.size();
				elems.push_back(data[i]);
			}
			
			// runs in O(n) time, produces max heap from unordered input array
			// second half of elems are leaves, 1 elem is maxheap by default
			for (size_t i = elems.size() >> 1; i != 0; --i){
				siftDown(i);
			} 
			
			#ifdef DEBUG_QUEUE
			std::cout << "MOVES LOADED: " << elements() << " " << is_heap() << " " << correct_index() << std::endl;
			#endif
		}
		
		// O(lgN) like change
		void change(T key) {
			if (cmp(elems[index[key.ID]], key)){
				elems[index[key.ID]] = key;
				siftDown(index[key.ID]);
			} else {
				elems[index[key.ID]] = key;
				siftUp(index[key.ID]);
			}
			
			#ifdef DEBUG_QUEUE
			std::cout << "MOVES UPDATED: " << elements() << " " << is_heap() << " " << correct_index() << std::endl;
			#endif
		}
		
		// tests -----------------
		bool is_heap() const {
			size_t L, R;
			for (size_t i = size >> 1; i != 0; --i){
				L = MovesQueue::left(i);
				R = MovesQueue::right(i);
				if (L < size && !cmp(elems[i], elems[L])){ return false; }
				if (R < size && !cmp(elems[i], elems[R])){ return false; }
			}
			return true;
		}
		
		bool correct_index() const {
			uint skip = 0;
			for(uint i=0;i<size;++i){
				if(index[i] == skip){continue;}
				//std::cout << i << ": " << index[i] << " ";
				//Debugger::print_move(elems[index[i]]);
				if(i != elems[index[i]].ID){return false;}
			}
			return true;
		}
	};
	
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
		 Field: solution
		 The best found so far solution in the current epoche
		 */
		colors solution;
		/*
		 Field: neighbors
		 List of all available one-exchange moves for the current improvement.
		 */
		Moves neighbors;
		/*
		 Field: solution_score
		 The score of the best found so far solution in the current epoche
		 */
		Score solution_score;
		/*
		 Field: conflicts
		 Array with the conflicts with size K * N
		 */
		delta* conflicts;
		/*
		 Field: guidance
		 Array with the guidance with size K * N
		 */
		delta* guidance;
		/*
		 Field: weights
		 Array with the current weights of the edges
		 */
		delta* weights;
		/*
		 Field: Q
		 Priority queue of all movements
		 ORDER BY total ASC    
		*/
		MovesQueue<Move, MoveTotalCompare> Q;
		/*
		 Field: A
		 Priority queue of all movements
		 ORDER BY conflicts ASC    
		*/
		MovesQueue<Move, MoveConflictsCompare> A;
		/*
		Method: build_structs
		Builds the conflicts and guideance structures according to a given coloring of a graph G.
		Additionally, if the CLEAR_WEIGHTS flags is up, the weights are cleaned.
		Uf priority queues are used, then they are also build here.
		*/
		void build_structs(const graph_access &G, const colors &coloring){
			uint ID;
			for(NodeID v = 0; v < N; ++v) {
				for(uint i = 0; i < K; ++i){
					ID = K*v + i;
					conflicts[ID] = 0;
					guidance[ID]  = 0;
				}
				EdgeID until = G.get_first_invalid_edge(v);
				for(EdgeID e = G.get_first_edge(v); e < until; ++e){
					NodeID u = G.getEdgeTarget(e);
					ID = K*v + coloring[u];
					conflicts[ID]++;
					if(RESET_WEIGHTS){
						weights[e] = 0;
					} else {
						guidance[ID] += weights[e];
					}
				}
			}
		}
		/*
		Method: update_weights
		Calculates the utilites of the edges and then increment the value of the weights with 1 for edges having maximal utility.
		Returns the number of edges, which have changed weights.
		Addituionally it updates the score to the new guidance.
		*/
		Moves update_weights(const graph_access &G, const colors &coloring, Score &score){
			std::vector<std::pair<NodeID, EdgeID>> E;// = std::vector<std::pair<NodeID, EdgeID>>();
			std::set<NodeID> refresh = std::set<NodeID>();
			
			float max = 0, utility = 0;
			for(NodeID v = 0; v < N; ++v) {
				EdgeID until = G.get_first_invalid_edge(v);
				for(EdgeID e = G.get_first_edge(v); e < until; ++e){
					NodeID u = G.getEdgeTarget(e);
					if(coloring[v] != coloring[u]){ continue; }
					
					utility = 1.0 / float(1 + weights[e]);
					if(utility > max){
						max = utility;
						E = std::vector<std::pair<NodeID, EdgeID>>();
						refresh = std::set<NodeID>();
					}
					
					if(utility == max){
						std::pair<NodeID, EdgeID> update = std::make_pair(v, e);
						E.push_back(update);
						refresh.insert(v);
					}
				}
			}
			
			score = Score::build(score.conflicts, score.guidance + E.size() / 2);
			for(std::pair<NodeID, EdgeID> edge: E){
				guidance[K*edge.first + coloring[edge.first]]++;
				weights[edge.second]++;
			}
			
			uint src;
			Move mv;
			Moves updates = Moves();
			for(NodeID v: refresh){
				for(color c = 0; c < K; c++){						
					src = K * v + coloring[v];
					mv = Move(v, c, K);
					mv.score = DeltaScore::build(conflicts[mv.ID] - conflicts[src], guidance[mv.ID] - guidance[src]);
					updates.push_back(mv);
				}
			}
			
			return updates;
		}
		/*
		Method: build_score
		Builds the score of a given colorring.
		*/
		Score build_score(const colors &coloring){
			uint c = 0, g = 0;
			for(NodeID v = 0; v < N; v++){
				c += conflicts[K*v + coloring[v]];
				g += guidance[K*v + coloring[v]];
			}
			return Score::build(c / 2, g / 2);
		}
		/*
		Method: make_move
		Applies the next move to a given colring of the graph G with known score.
		Additionally, it applies the move to the conflicts and guidance structures.
		Returns a list with updated moves by applying the move.
		*/
		Moves make_move(const graph_access &G, colors &coloring, Score &score, Move next){
			Moves updates = Moves();
			Move mv;
			
			uint src, dest;
			EdgeID until = G.get_first_invalid_edge(next.node);
			for(EdgeID e = G.get_first_edge(next.node); e < until; e++){
				NodeID u = G.getEdgeTarget(e);
				src = K*u + coloring[next.node], dest = K*u + next.to; 
				
				conflicts[src]--;
				guidance[src] -= weights[e];
				
				conflicts[dest]++;
				guidance[dest] += weights[e];
			}
			
			for(EdgeID e = G.get_first_edge(next.node); e < until; e++){
				NodeID u = G.getEdgeTarget(e);
				for(uint c = 0; c < K; c++){
					mv = Move(u, c, K);
					src = K * u + coloring[u];
					mv.score = DeltaScore::build(conflicts[mv.ID] - conflicts[src], guidance[mv.ID] - guidance[src]);
					updates.push_back(mv);
				}
			}
			
			for(uint c = 0; c < K; c++){
				mv = Move(next.node, c, K);
				mv.score = DeltaScore::build(conflicts[mv.ID] - conflicts[next.ID], guidance[mv.ID] - guidance[next.ID]);
				if(FAST_SEARCH && mv.ID == next.ID){
					mv.score.total = std::numeric_limits<delta>::max();
				}
				updates.push_back(mv);
			}
			
			score.conflicts += next.score.conflicts;
			score.guidance  += next.score.guidance;
			score.total 	+= next.score.total;
			coloring[next.node] = next.to;
			
			return updates;
		}
		/*
		Method: update_neighbors
		Update the neighbors according to a given list with updates moves.
		Record the changes in the priority among the queues.
		*/
		void update_neighbors(Moves &updates){
			for(Move mv: updates){
				if(neighbors[mv.ID].score.conflicts == mv.score.conflicts &&
					neighbors[mv.ID].score.guidance == mv.score.guidance){
					continue;
				}
				
				neighbors[mv.ID].score = mv.score;
				
				if(MOVE_QUEUE){
					Q.change(neighbors[mv.ID]);
					if(ASPIRATION){ A.change(neighbors[mv.ID]); }
				}
			}
		}
		/*
		Method: build_neighbors
		Biuld all available moves in the one-exchange neighborhood.
		*/
		Moves build_neighbors(const colors &coloring){
			Moves moves = Moves();
			moves.reserve(K * N);
			for(NodeID v = 0; v < N; v++){
				const uint sourceID = K*v + coloring[v];
				for(uint c = 0; c < K; c++){
					Move mv = Move(v, c, K);
					if (coloring[v] == c){
						mv.score = DeltaScore::build(0, 0);
					} else {
						mv.score = DeltaScore::build(conflicts[mv.ID] - conflicts[sourceID], guidance[mv.ID] - guidance[sourceID]);
					}
					moves.push_back(mv);
				}
			}
			return moves;
		}
		/*
		Method: load_neighbors
		Builds all neighbors and loads them in the priority queues.
		*/
		void load_neighbors(const colors &coloring){
			neighbors = build_neighbors(solution);
			if(MOVE_QUEUE){
				Q.build(neighbors);
				if(ASPIRATION){ A.build(neighbors); }
			}
		}
		/*
		Method: is_aspiration
		Check if a movement is an aspiration according to a given solution score.
		*/
		bool is_aspiration(const Move &aspiration, const Score &score){
			if(aspiration.score.total <= 0){ return false; }
			if(score.conflicts + aspiration.score.conflicts >= solution_score.conflicts){ return false; }
			report.aspiration();
			return true;
		}
		/*
		 Method: restrict_neighbours
		 Restrict a moves list to the restricted one-exchange neighbourhood of a given coloring with estimated score.
		*/
		Moves restrict_neighbours(const Moves &neighbors, const colors &coloring, Score score){
			Moves moves = Moves();
			
			if(MOVE_QUEUE){
				if(ASPIRATION){
					Moves aspirations = A.head(coloring, conflicts, K);
					if(aspirations.size() > 0){
						Move asp = aspirations.front();
						if(is_aspiration(asp, score)){
							moves.push_back(asp);
							return moves;
						}
					}
				}
				moves = Q.head(coloring, conflicts, K);
			} else {
				MoveTotalCompare tCmp = MoveTotalCompare(); 
				MoveConflictsCompare cCmp = MoveConflictsCompare(); 
				
				DeltaScore best = neighbors[coloring[0]].score;
				DeltaScore asp = neighbors[coloring[0]].score;
				
				Moves aspirations = Moves();
				
				for(Move mv: neighbors){
					const uint sourceID = K*mv.node + coloring[mv.node];
					if(conflicts[sourceID] == 0){ continue; }
					if(coloring[mv.node] == mv.to){ continue; }
					if(tCmp.weak(mv.score, best)){
						best = mv.score;
						moves = Moves();
					}
					if(tCmp(mv.score, best)){
						moves.push_back(mv);
					}
					
					if(ASPIRATION){					
						if(cCmp.weak(mv.score, asp)){
							asp = mv.score;
							aspirations = Moves();
						}
						
						if(cCmp(mv.score, asp) && is_aspiration(mv, score)){
							moves.push_back(mv);
						}
					}
				}				
				if(ASPIRATION && aspirations.size() > 0){
					return aspirations;
				}
			}
			
			return moves;
		}
		/*
		 Method: best_neighbours
		 Full scan search of the restricted one-exchange neighbourhood of a given coloring
		*/
		Moves best_neighbours(const colors coloring){ 
			Move best = Move();
			Moves moves;
			for(NodeID v = 0; v < N; v++){
				const uint sourceID = K*v + coloring[v];
				if(conflicts[sourceID] == 0){ continue; }
				for(uint c = 0; c < K; c++){
					if(coloring[v] == c){ continue; }
					Move mv = Move(v, c, K);
					mv.score = DeltaScore::build(conflicts[mv.ID] - conflicts[sourceID], guidance[mv.ID] - guidance[sourceID]);
					if(mv.score.total < best.score.total){
						best = mv;
						moves = Moves();
					}
					if(mv.score.total == best.score.total){
						moves.push_back(mv);
					}
				}
			}
			return moves;
		}
		
		public:
		
		SolveReport report;
		
		/*
		 Method: ~GuidedLocalSearch
		 Destructs a solver, by freeing the memory for the internal arrays.
		*/
		~GuidedLocalSearch(){
			delete [] conflicts;
			delete [] guidance;
			delete [] weights;
		}
		
		void prepare(const graph_access &G, const colors &coloring, const uint &k){
			N = G.number_of_nodes();
			M = G.number_of_edges();
			K = k;
			
			conflicts = new delta[K * N];
			guidance = new delta[K * N];
			weights = new delta[M];
			for(EdgeID e = 0;e<M;e++){weights[e]=0;}
			
			if(MOVE_QUEUE){
				Q.prepare(N * K);
				if(ASPIRATION){ A.prepare(N * K); }
			}
			
			report.prepare(K);
		}
		
		/*
		 Method: solve
		 Applies the GLS strategy to solve the k-coloring problem for an initial coloring of a graph G. 
		*/
		colors solve(const graph_access &G, const colors &coloring, const uint k){
			K = k;
			build_structs(G, coloring);
			solution = coloring;
			solution_score = build_score(solution);
			if(solution_score.conflicts == 0){
				report.skip_epoch(K);
				return solution;
			}
			
			report.prepare_epoch(K, solution_score);
			
			load_neighbors(solution);
			
			colors improvement = solution;
			Score score = solution_score;
			SolveResolution resolution = SolveResolution::NotFound;
			
			while(resolution == SolveResolution::NotFound){
				report.iteration();
				
				if(solution_score.conflicts == 0){
					resolution = SolveResolution::Solved;
					solution = improvement;
					break;
				}
				
				Moves moves = restrict_neighbours(neighbors, improvement, score);
				
				if(moves.size() == 0){
					resolution = report.minimum();
				} else {
					Move next = moves[rand() % moves.size()];
					resolution = report.check_move(next);
					
					Moves updates = make_move(G, improvement, score, next);
					update_neighbors(updates);
					
					report.report_move(next, score);
					
					if(score.conflicts < solution_score.conflicts){
						solution = improvement;
						solution_score = score;
						report.improvement(solution_score);
					}
				}
				
				if(resolution == SolveResolution::NoImprove || resolution == SolveResolution::LocalMin){
					report.weight_update();
					Moves updates = update_weights(G, improvement, score);
					update_neighbors(updates);
					resolution = SolveResolution::NotFound;
				}
				
				if(solution_score.conflicts == 0){
					resolution = SolveResolution::Solved;
					solution = improvement;
				}
			}
			
			report.finish_epoch(solution_score);
			
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
			
			solver.report.finish_solution();
			
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
