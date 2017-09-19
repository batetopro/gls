import os
import subprocess
import configparser
import matplotlib.pyplot as plt
import math
from util import read_csv


def read_experiments():
    result = []

    X = {}
    path = os.path.join(db, "graphs.csv")
    with open(path, "r") as file:
        line = file.readline()
        line = file.readline()
        while line:
            parts = line.strip().strip('"').split('";"')
            if len(parts) == 1:
                line = file.readline()
                continue

            if parts[0] not in X:
                X[parts[0]] = {}

            X[parts[0]][parts[1]] = int(parts[7])

            line = file.readline()

    path = os.path.join(db, "expirements.in")
    with open(path, "r") as file:
        line = file.readline()
        while line:
            parts = line.strip().split(" ")
            if len(parts) != 2:
                line = file.readline()
                continue
            g = {
                "source": parts[0],
                "name": parts[1],
                "path": os.path.join(db, "graphs", parts[0], parts[1]),
                "folder": os.path.join(db, "experiment", parts[0], parts[1])
            }

            if not os.path.exists(g["path"]):
                print("Graph " + g["source"] + " " + g["name"] + " does not exist.")
                line = file.readline()
                continue

            try:
                g["min"] = X[g["source"]][g["name"]]
            except:
                g["min"] = "2"

            result.append(g)
            line = file.readline()

    return result


def gls(graph_path, output_path, config_path, **kwargs):
    config = configparser.ConfigParser()
    config.optionxform = str

    with open(config_path, "r") as fp:
        config.read_file(fp)

    for k in kwargs:
        config.set("gls", k, str(kwargs[k]))

    with open("gls.ini", "w") as fp:
        config.write(fp)

    cmd = os.path.join(db, "bin", "gls.exe") + " " + graph_path

    with open(output_path, "w") as out:
        subprocess.call(cmd, stdout=out)


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


def moves(g):
    main = {
        "BUILD_STRATEGY": 2,
        "UPDATE_STRATEGY": 1,
        "SOURCE_TARGET": 1,
        "DESTIANTION_TARGET": 2,
        "RESET_WEIGHTS": 1,
        "ASPIRATION": 0,
        "FAST_SEARCH": 0,
        "LAMBDA": 10,
        "TIMEOUT": 2,
        "DEBUG": 24,
        "LOWER_BOUND": g["min"],
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

    BS = ["0", "1", "2"]
    for s in show:
        for k in s["options"]:
            main[k] = s["options"][k]

        for bs in BS:
            main["BUILD_STRATEGY"] = bs

            target = os.path.join(g["folder"], "moves." + bs + "." + s["name"])
            if os.path.exists(target + ".png"):
                continue

            print(s["name"] + " " + bs)

            gls(g["path"], target, "moves.ini", **main)
            parse_moves(target, g, s)

            os.remove(target)


def strategies(g):
    log = os.path.join(g["folder"], "strategy")
    if os.path.exists(log + ".csv"):
        return

    main = {
        "BUILD_STRATEGY": 1,
        "UPDATE_STRATEGY": 1,
        "SOURCE_TARGET": 1,
        "DESTIANTION_TARGET": 2,
        "RESET_WEIGHTS": 1,
        "LOWER_BOUND": g["min"],
        "ASPIRATION": 0,
        "FAST_SEARCH": 0,
        "DYNAMIC_LAMBDA": 1,
        "TIMEOUT": 2,
        "LAMBDA": 10,
        "DEBUG": 4,
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
            "updates", "aspirations", "conflicts", "guidance", "score"]

    result = '"' + '";"'.join(keys) + '"\n'

    for bs in BS:
        print("Strategy: " + bs["name"])

        main["BUILD_STRATEGY"] = bs["id"]
        if bs["id"] == "0":
            main["UPDATE_STRATEGY"] = "0"
            gls(g["path"], log, "strategy.ini", **main)
            with open(log, "r") as file:
                data = file.readline().strip()
                data = '"' + data.replace(',', '";"') + '"'
                result += '"Scratch";"";"";' + data + "\n"
        main["UPDATE_STRATEGY"] = "1"
        for src in T:
            main["SOURCE_TARGET"] = src["id"]
            for dest in T:
                main["DESTIANTION_TARGET"] = dest["id"]
                gls(g["path"], log, "strategy.ini", **main)
                with open(log, "r") as file:
                    data = file.readline().strip()
                    data = '"' + data.replace(',','";"') + '"'
                    result += '"' + bs["name"] + '";"' + src["name"] + '";"' + dest["name"] + '";' + data + "\n"

    with open(log + ".csv", "w") as file:
        file.write(result)

    os.remove(log)


def meta(g):
    log = os.path.join(g["folder"], "meta")
    if os.path.exists(log + ".csv"):
        return

    main = {
        "BUILD_STRATEGY": 2,
        "UPDATE_STRATEGY": 1,
        "SOURCE_TARGET": 1,
        "DESTIANTION_TARGET": 2,
        "RESET_WEIGHTS": 1,
        "LOWER_BOUND": g["min"],
        "ASPIRATION": 0,
        "FAST_SEARCH": 0,
        "TIMEOUT": 2,
        "LAMBDA": 10,
        "DEBUG": 4,
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
            "updates", "aspirations", "conflicts", "guidance", "score"]

    result = '"' + '";"'.join(keys) + '"\n'

    SW = [1,2,4,8,16,32]

    for sw in SW:
        main["MAX_NO_IMPROVE"] = sw
        for aspiration in range(2):
            main["ASPIRATION"] = aspiration
            for penalty in range(2):
                main["RESET_WEIGHTS"] = penalty

                if penalty == 1:
                    main["DYNAMIC_LAMBDA"] = "1"

                    header = ['"' + str(v) + '"' for v in [aspiration, penalty, "dynamic", sw]]
                    header = ";".join(header)
                    print(header)

                    gls(g["path"], log, "meta.ini", **main)
                    with open(log, "r") as file:
                        data = file.readline().strip()
                        data = '"' + data.replace(',', '";"') + '"'
                        result += header + ';' + data + "\n"

                main["DYNAMIC_LAMBDA"] = 0
                for l in range(1, 100, 3):
                    main["LAMBDA"] = l

                    header = ['"' + str(v) + '"' for v in [aspiration, penalty, l / 10.0, sw]]
                    header = ";".join(header)
                    print(header)
                    gls(g["path"], log, "meta.ini", **main)
                    with open(log, "r") as file:
                        data = file.readline().strip()
                        data = '"' + data.replace(',', '";"') + '"'
                        result += header + ';' + data + "\n"

    with open(log + ".csv", "w") as file:
        file.write(result)

    os.remove(log)


def group(data, key):
    result = {}
    for r in data:
        if r[key] not in result:
            result[r[key]] = []
        result[r[key]].append(r)
    return result


def meta_images(g):
    path = os.path.join(g["folder"], "meta")
    if not os.path.exists(path + ".csv"):
        return
    data = read_csv(path + ".csv")
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
        "resolve_ratio": {"title": "Resolve ratio", "f": lambda r: float(r["conflicts"]) / float(r["improvements"])},
        "aspiration_ratio": {"title": "Aspiration ratio",
                             "f": lambda r: float(r["aspirations"]) / float(r["improvements"])},
    }

    for sw in SW:
        print("meta", sw)
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


if __name__ == "__main__":
    db = os.path.join(os.getcwd(), "db")

    graphs = read_experiments()
    for g in graphs:
        print(g["source"], g["name"])
        os.makedirs(g["folder"], exist_ok=True)
        moves(g)
        strategies(g)
        meta(g)
        meta_images(g)