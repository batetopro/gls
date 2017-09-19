This module contains the logic for importing a new graph to the coloring database.


The import process contains the following steps:
1. add a copy in folder benchmarks // TODO
2. read a file in DIMACS format and converts it to the project's format.
3. runs an upper bound extimation of the graph
4. runs a guided local search on the grpah
Note: estimation of the chromatic number can be added as the field "min"


To add a new graph source
1. add an entry of the source in groups.csv
2. add a folder with the name of the source in folder import
Note: we should keep the convention a source to be three letters long.


To add a new graph
1. copy the graph instance in DIMACS format in its group in folder import
2. run the script "import.py"


To change the GLS import settings edit import.ini


The moves process contains the following steps:
1. reads the moves.in file for which graphs to be processed
2. reads the parsed format of the graoh
3. executes guided local search with move debugging
4. creates an image of the algorithm's progress in the followung cases:
  - clean
  - with aspirations
  - with penalty keeping
  - with both heuristics
5. the images can be found in the folder moves


The strategy process contains the following steps:
1. reads the strategy.in file for which graphs to be processed
2. reads the parsed format of the graoh
3. executes guided local search with epoche debugging
4. creates a csv file of the algorithm's progress in the followung cases with different:
  - initialiazations
  - epoche updates strategies and targets
5. the files can be found in the folder strategies


The lambda process contains the following steps:
1. reads the lambda.in file for which graphs to be processed
2. reads the parsed format of the graoh
3. executes guided local search with solution debugging
4. creates a csv file of the algorithm's progress in the followung cases with different:
  - lambda
  - max no improve
5. the files can be found in the folder lambda

