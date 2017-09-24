import os
import sys
from util import read_graph, gls, read_csv
import random


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

L = {}
N = 10
for s in logs:
    for g in logs[s]:
        log = logs[s][g]
        data = []
        for row in read_csv(log):
            if row["SW"] != "2": continue
            if row["iterations"] == "0": continue
            if row["updates"] == "0": continue
            if row["L"] == "dynamic": continue
            if row["A"] == "0": continue
            if row["P"] == "1": continue

            for k in row:
                if row[k] == "dynamic":
                    continue
                row[k] = float(row[k])
            data.append(row)

        #data = sorted(data, key=lambda row: (row["k"], row["time"], row['iterations']))

        data = sorted(data, key=lambda row: (row["k"], row["time"], row["final_conflicts"], row['iterations']))

        sw = {}
        ctr = 0
        for row in data:
            if row["L"] not in sw:
                sw[row["L"]] = 0
            sw[row["L"]] += 1
            ctr += 1
            if ctr == N: break

        for k in sw:
            if k not in L: L[k] = 0
            L[k] += 1

for k in sorted(L.keys()):
    print(k, L[k])
