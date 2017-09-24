import os
import sys
from util import read_graph, gls

if len(sys.argv) != 4:
    print("Usage: group graph tid")
    exit(1)

db = os.path.join(os.getcwd(), "db")
graph = read_graph(sys.argv[1], sys.argv[2], db)
if not graph:
    print("Graph does not exist")
    exit(1)

log = os.path.join(graph["folder"], "strategy.csv")
if os.path.exists(log):
    exit(0)

main = {
    "LOWER_BOUND": graph["x"],
}

BS = [
    {
        "name": "Random",
        "id": "0"
    },
    {
        "name": "Greedy",
        "id": "1"
    },
    {
        "name": "Bipartite",
        "id": "2"
    }
]
T = [
    {
        "name": "Minimal",
        "id": "1"
    },
    {
        "name": "Maximum",
        "id": "2"
    },
    {
        "name": "Median",
        "id": "3"
    }
]

keys = ["init", "source", "destination",
        "k", "iterations", "time", "improvements", "minimums",
        "updates", "aspirations", "conflicts", "guidance", "score",
        "final_conflicts", "final_guidance", "final_score"]

result = '"' + '";"'.join(keys) + '"\n'

for bs in BS:
    print(sys.argv[3], sys.argv[1], sys.argv[2], bs["name"])

    main["BUILD_STRATEGY"] = bs["id"]
    if bs["id"] == "0":
        main["UPDATE_STRATEGY"] = "0"
        result += '"Scratch";"";"";' + gls(graph["path"], sys.argv[3], "strategy.ini", db, **main) + "\n"

    main["UPDATE_STRATEGY"] = "1"
    for src in T:
        main["SOURCE_TARGET"] = src["id"]
        for dest in T:
            main["DESTINATION_TARGET"] = dest["id"]
            header = '"' + bs["name"] + '";"' + src["name"] + '";"' + dest["name"]
            result += header + '";' + gls(graph["path"], sys.argv[3], "strategy.ini", db, **main) + "\n"

with open(log, "w") as file:
    file.write(result)
