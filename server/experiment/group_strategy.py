import os
import sys
from util import read_csv, read_graph


if len(sys.argv) != 3:
    print("Usage: group graph")

db = os.path.join(os.getcwd(), "db")
source = os.path.join(db, "experiment", sys.argv[1])

if not os.path.exists(source):
    print("The source group does not exist")
    exit(1)

target = os.path.join(source, "strategy.csv")

if os.path.exists(target):
    exit(0)

try:
    with open(target, "w") as output:
        H = ["init", "source", "destination"]
        B = ["k", "time", "iterations", "conflicts"]

        header = ['"' + v + '"' for v in H + B + ["hits"]]
        header = ";".join(header) + "\n"
        output.write(header)

        data = {}

        graphs = []
        for g in os.listdir(source):
            log = os.path.join(source, g)
            if os.path.isfile(log):
                continue

            log = os.path.join(log, "strategy.csv")
            if not os.path.exists(log) or not os.path.isfile(log):
                continue

            graphs.append(g)

        data = {}
        for g in graphs:
            log = os.path.join(source, g, "strategy.csv")

            G = read_graph(sys.argv[1], g, db)
            if not G: continue

            D = []
            maxD = {}
            for raw in read_csv(log):
                if int(raw["iterations"]) == 0:
                    continue
                row = {}

                for k in raw:
                    if k in H:
                        row[k] = raw[k]
                    if k in B:
                        row[k] = float(raw[k])
                    if k == "k":
                        row[k] -= float(G["x"])
                D.append(row)

            for row in D:
                for k in B:
                    if k not in maxD:
                        maxD[k] = row[k]
                    elif maxD[k] < row[k]:
                        maxD[k] = row[k]

            for row in D:
                entry = {}
                for k in B:
                    if maxD[k] != 0:
                        row[k] /= maxD[k]

                    if row["init"] not in data:
                        data[row["init"]] = {}

                    if row["source"] not in data[row["init"]]:
                        data[row["init"]][row["source"]] = {}

                    if row["destination"] not in data[row["init"]][row["source"]]:
                        data[row["init"]][row["source"]][row["destination"]] = {}

                    if k not in data[row["init"]][row["source"]][row["destination"]]:
                        data[row["init"]][row["source"]][row["destination"]][k] = []

                    data[row["init"]][row["source"]][row["destination"]][k].append(row[k])

        for i in data:
            for s in data[i]:
                for d in data[i][s]:
                    line = [i, s, d]
                    for k in B:
                        if len(data[i][s][d][k]) == 0:
                            continue
                        line.append(sum(data[i][s][d][k]) / len(data[i][s][d][k]))
                    line.append(len(data[i][s][d][B[0]]))
                    line = ['"' + str(v) + '"' for v in line]
                    line = ";".join(line) + "\n"
                    output.write(line)
except:
    exit(0)
