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

SW = {"a": {"dyn": 0, "const": 0, "equal": 0}, "c": {"dyn": 0, "const": 0, "equal": 0}}

N = 5
for s in logs:
    for g in logs[s]:
        log = logs[s][g]
        data = []
        dyn = {"a": None, "c": None}
        for row in read_csv(log):
            if row["SW"] != "16": continue
            if row["iterations"] == "0": continue
            if row["updates"] == "0": continue

            for k in row:
                if row[k] == "dynamic":
                    continue
                row[k] = float(row[k])

            if row["L"] == "dynamic":
                if row["A"] == 1:
                    dyn["a"] = row
                else:
                    dyn["c"] = row
            else:
                data.append(row)

        data = sorted(data, key=lambda row: (row["k"], row["time"], row['iterations']))

        sw = {"a": {"dyn": 0, "const": 0, "equal": 0}, "c": {"dyn": 0, "const": 0, "equal": 0}}
        K = ["k", "time", "iterations"]
        ctr = 0

        for row in data:
            for d in dyn:
                found = False
                for k in K:
                    if dyn[d][k] > row[k]:
                        found = True
                        sw[d]["dyn"]+=1
                    elif dyn[d][k] < row[k]:
                        sw[d]["const"] += 1
                        found = True
                    if found: break
                if not found:
                    sw[d]["equal"] += 1

        for k in sw:
            for k2 in sw[k]:
                SW[k][k2] += sw[k][k2]

for k in sorted(SW.keys()):
    for k2 in sorted(SW[k].keys()):
        print(k, k2, SW[k][k2])
