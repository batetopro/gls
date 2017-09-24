import os
import sys
from util import read_graph, gls
import matplotlib.pyplot as plt
import math


if len(sys.argv) != 3:
    print("Usage: group graph")

db = os.path.join(os.getcwd(), "db")
graph = read_graph(sys.argv[1], sys.argv[2], db)
if not graph:
    print("Graph does not exist")
    exit(1)

print(graph["source"], graph["graph"])

main = {
    "LOWER_BOUND": graph["x"],
}

show = [
    {
        "name": "clean",
        "title": "Clean GLS",
        "options": {
            "RESET_WEIGHTS": 1,
            "ASPIRATION": 0,
            "DYNAMIC_LAMBDA": 0,
        }
    },
    {
        "name": "aspiration",
        "title": "Aspirations",
        "options": {
            "RESET_WEIGHTS": 1,
            "ASPIRATION": 1,
            "DYNAMIC_LAMBDA": 0,
        }
    },
    {
        "title": "Keep weights",
        "name": "keep",
        "options": {
            "RESET_WEIGHTS": 0,
            "ASPIRATION": 0,
            "DYNAMIC_LAMBDA": 0,
        }
    },
    {
        "title": "Dynamic lambda",
        "name": "dynamic",
        "options": {
            "RESET_WEIGHTS": 1,
            "ASPIRATION": 0,
            "DYNAMIC_LAMBDA": 1,
        }
    },
    {
        "title": "Aspirations and penalty keeping",
        "name": "asp_keep",
        "options": {
            "RESET_WEIGHTS": 0,
            "ASPIRATION": 1,
            "DYNAMIC_LAMBDA": 0,
        }
    },
    {
        "title": "Aspirations and dynamic lambda",
        "name": "asp_dynamic",
        "options": {
            "RESET_WEIGHTS": 1,
            "ASPIRATION": 1,
            "DYNAMIC_LAMBDA": 1,
        }
    }
]


def parse_moves(target, g, s):
    with open(target, "r") as file:
        line = file.readline()

        data = {
            "min": [],
            "best_conflicts": [],
            "best_guidance": [],
            "conflicts": [],
            "guidance": [],
            "score": [],
            "starts": []
        }

        iterations = []
        improves = []
        while line:
            parts = line.strip().split(",")

            for k in range(1, len(parts)):
                parts[k] = int(parts[k])

            if parts[0] == "MOVE":
                if parts[4] > 0:
                    data["conflicts"].append(math.log(parts[4]))
                else:
                    data["conflicts"].append(-1)
                if parts[5] > 0:
                    data["guidance"].append(math.log(parts[5]))
                else:
                    data["guidance"].append(-1)
                if parts[6] > 0:
                    data["score"].append(math.log(parts[6]))
                else:
                    data["score"].append(-1)

                iterations.append(float(parts[1]))

            elif parts[0] == "MIN":
                pass
            elif parts[0] == "IMPROVE":
                if parts[2] > 0:
                    data["best_conflicts"].append(math.log(parts[2]))
                else:
                    data["best_conflicts"].append(-1)

                if parts[3] > 0:
                    data["best_guidance"].append(math.log(parts[3]))
                else:
                    data["best_guidance"].append(-1)

                improves.append(float(parts[1]))
            elif parts[0] == "START":
                data["starts"].append(float(parts[1]))
            line = file.readline()

        plt.clf()
        plt.cla()
        plt.close()
        plt.xlabel('iterations')
        plt.ylabel('log(score)')
        plt.title(s["title"])

        for st in data["starts"]:
            plt.axvline(x=st, color='k', linestyle='--')

        plt.plot(iterations, data["conflicts"], "r", iterations, data["guidance"], "b", improves,
                 data["best_conflicts"], "ko", iterations, data["score"], "g--")
        plt.savefig(target + ".png")


BS = ["0", "1", "2"]
for s in show:
    for k in s["options"]:
        main[k] = s["options"][k]

    for bs in BS:
        main["BUILD_STRATEGY"] = bs

        target = os.path.join(graph["folder"], "moves." + bs + "." + s["name"])
        if os.path.exists(target + ".png"):
            continue

        print(s["name"] + " " + bs)

        gls(graph["path"], target, "moves.ini", db, **main)

        parse_moves(target, graph, s)

        try:
            os.remove(target)
        except:
            pass