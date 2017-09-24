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
cnt = 0
for s in os.listdir(exp):
    path = os.path.join(exp, s)
    if os.path.isfile(path):
        continue
    logs[s] = {}

    for g in os.listdir(path):
        if os.path.isfile(os.path.join(path, g)):
            continue

        log = os.path.join(path, g, "meta.csv")

        if not os.path.exists(log):
            continue

        if G[s][g]["x"] == G[s][g]["greedy"]: continue
        cnt += 1
        logs[s][g] = log
print(cnt)

K = {
    "00": "Keep penalties",
    "01": "Clean",
    "10": "Aspirations",
    "11": "Both",
}
L = {}
N = 3
for s in logs:
    for g in logs[s]:
        log = logs[s][g]
        data = []
        for row in read_csv(log):
            if row["SW"] != "2": continue
            if row["L"] != "1.0": continue
            if row["iterations"] == "0": continue
            if row["updates"] == "0": continue
            #if row["aspirations"] == "0" and row["A"] == "1": continue

            if row["L"] == "dynamic": continue

            row["key"] = K[row["A"] + row["P"]]

            for k in row:
                if row[k] == "dynamic" or k == "key":
                    continue
                row[k] = float(row[k])

            data.append(row)

        if len(data) == 0: continue

        data = sorted(data, key=lambda row: (row["k"], row["time"], row['iterations']))

        #data = sorted(data, key=lambda row: (row["k"], row["time"], row["final_conflicts"], row['iterations']))

        sw = {}
        ctr = 0
        for row in data:
            if row["key"] not in sw:
                sw[row["key"]] = 0
            sw[row["key"]] += 1
            ctr += 1
            if ctr == N: break

        for k in sw:
            if k not in L: L[k] = 0
            L[k] += 1

for k in sorted(L.keys()):
    print(k, L[k])
