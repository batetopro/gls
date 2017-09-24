import os
import sys
from util import read_graph, gls, read_csv
import matplotlib.pyplot as plt


def resolve_ratio(row):
    if float(row["improvements"]) > 0:
        return float(row["conflicts"]) / float(row["improvements"])
    else:
        return 0


def aspiration_ratio(row):
    if float(row["improvements"]) > 0:
        return float(row["aspirations"]) / float(row["improvements"])
    else:
        return 0


def group(data, key):
    result = {}
    for r in data:
        if r[key] not in result:
            result[r[key]] = []
        result[r[key]].append(r)
    return result


if len(sys.argv) != 4:
    print("Usage: group graph tid")
    exit(1)


db = os.path.join(os.getcwd(), "db")
graph = read_graph(sys.argv[1], sys.argv[2], db)
if not graph:
    print("Graph does not exist")
    exit(1)


main = {
    "LOWER_BOUND": graph["x"]
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

keys = ["A", "P", "L", "SW",
        "k", "iterations", "time", "improvements", "minimums",
        "updates", "aspirations", "conflicts", "guidance", "score",
        "final_conflicts", "final_guidance", "final_score"]

result = '"' + '";"'.join(keys) + '"\n'

SW = [1,2,4,8,16,32]

log = os.path.join(graph["folder"], "meta.csv")
if not os.path.exists(log):

    for sw in SW:
        print(sys.argv[3], sw, sys.argv[1], sys.argv[2])
        main["MAX_NO_IMPROVE"] = sw
        for aspiration in range(2):
            main["ASPIRATION"] = aspiration
            for penalty in range(2):
                main["RESET_WEIGHTS"] = penalty

                if penalty == 1:
                    main["DYNAMIC_LAMBDA"] = "1"
                    header = ['"' + str(v) + '"' for v in [aspiration, penalty, "dynamic", sw]]
                    header = ";".join(header)
                    result += header + ';' + gls(graph["path"], sys.argv[3], "meta.ini", db, **main) + "\n"

                main["DYNAMIC_LAMBDA"] = 0
                for l in range(1, 100, 3):
                    main["LAMBDA"] = l
                    header = ['"' + str(v) + '"' for v in [aspiration, penalty, l / 10.0, sw]]
                    header = ";".join(header)
                    result += header + ';' + gls(graph["path"], sys.argv[3], "meta.ini", db, **main) + "\n"

        with open(log, "w") as file:
            file.write(result)

data = read_csv(log)

SW = group(data, "SW")
M = {
    "0": {"0": "keep", "1": "clean" },
    "1": {"0": "asp_keep", "1": "aspiration"},
}

K = {
    "time": {"title": "Time", "f": lambda r: float(r["time"])},
    "k": {"title": "Colors", "f": lambda r: float(r["k"])},
    "conflicts": {"title": "Resolved conflicts", "f": lambda r: float(r["conflicts"])},
    "improvements": {"title": "Improvements", "f": lambda r: float(r["improvements"])},
    "aspirations": {"title": "Aspirations", "f": lambda r: float(r["aspirations"])},
    "minimums": {"title": "Minimums", "f": lambda r: float(r["minimums"])},
    "updates": {"title": "Updates", "f": lambda r: float(r["updates"])},
    "iterations": {"title": "Iterations", "f": lambda r: float(r["iterations"])},
    "resolve_ratio": {"title": "Resolve ratio", "f": resolve_ratio},
    "aspiration_ratio": {"title": "Aspiration ratio", "f": aspiration_ratio},
}

for sw in SW:
    print(sys.argv[3], sw, sys.argv[1], sys.argv[2])
    E = {}
    dyn = None
    dyna = None

    A = group(SW[sw], "A")
    for a in A:
        RP = group(A[a], "P")
        for rp in RP:
            m = M[a][rp]
            for row in RP[rp]:
                if row["L"] == "dynamic":
                    if a == "0":
                        dyn = row
                    else:
                        dyna = row
                else:
                    if m not in E:
                        E[m] = []
                    E[m].append(row)

    for k in K:
        path = os.path.join(graph["folder"], "meta")
        if os.path.exists(path + "." + sw + "." + k + ".png"):
            continue

        L = set()
        D = {}
        for n, g in E.items():
            D[n] = []
            for e in g:
                D[n].append(K[k]["f"](e))
                L.add(float(e["L"]))

        L = sorted(list(L))

        plt.clf()
        plt.cla()
        plt.close()
        plt.xlabel('lambda')
        plt.ylabel(K[k]["title"])
        plt.title(K[k]["title"] + ", sw = " + sw)

        plt.plot(L, D["clean"], "b", L, D["keep"], "b--", L, D["asp_keep"], "r--", L, D["aspiration"], "r")

        plt.axhline(y=K[k]["f"](dyna), color='k')
        plt.axhline(y=K[k]["f"](dyn), color='k', linestyle='--')
        plt.savefig(path + "." + sw + "." + k + ".png")
