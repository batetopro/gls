import os
import sys
from util import read_graph, gls, read_csv


db = os.path.join(os.getcwd(), "db")
exp = os.path.join(db, "experiment")

G = {}
for g in read_csv(os.path.join(db, "graphs.csv")):
    if g["source"] not in G:
        G[g["source"]] = {}
    if g["graph"] not in G[g["source"]]:
        G[g["source"]][g["graph"]] = g

logs = {}
for s in os.listdir(exp):
    path = os.path.join(exp, s)
    if os.path.isfile(path):
        continue
    logs[s] = {}

    for g in os.listdir(path):
        log = os.path.join(path, g, "meta.csv")
        if not os.path.exists(log):
            continue
        if G[s][g]["x"] == G[s][g]["greedy"]:
            continue
        logs[s][g] = log

U = {}
N = 5
for s in logs:
    for g in logs[s]:
        log = logs[s][g]
        data = []
        for row in read_csv(log):
            if row["iterations"] == "0": continue
            if row["L"] == "dynamic": continue
            if row["A"] == "1": continue
            if row["P"] == "0": continue

            for k in row:
                if row[k] == "dynamic":
                    continue
                row[k] = float(row[k])
            data.append(row)

        data = sorted(data, key=lambda row: (row["k"], row["time"], row['iterations']))

        for row in data:
            if row["SW"] not in U:
                U[row["SW"]] = 0
            U[row["SW"]] += row["updates"] - row["minimums"]

for k in sorted(U.keys()):
    print(k, U[k])
