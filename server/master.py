import os
import threading
import subprocess
import time
import sys
import random


class Slave(threading.Thread):
    def __init__(self, group, graph, tid):
        threading.Thread.__init__(self)
        self.group = group
        self.graph = graph
        self.tid = str(tid)
        self.finished = False
        self.started = False

    def run(self):
        global command
        self.started = True
        cmd = "python " + os.path.join(exp, command + ".py") + " " + self.group + " " + self.graph + " " + self.tid
        subprocess.call(cmd)
        self.finished = True


exp = os.path.join(os.getcwd(), "experiment")
db = os.path.join(os.getcwd(), "db")

command = "strategy"
if len(sys.argv) > 1:
    command = sys.argv[1]

if not os.path.exists(os.path.join(exp, command + ".py")):
    print("Experiment not found")
    exit(1)


K = 6
graphs = []
path = os.path.join(db, "expirements.in")
with open(path, "r") as file:
    line = file.readline()
    while line:
        parts = line.strip().split(" ")
        if len(parts) != 2:
            line = file.readline()
            continue
        graphs.append({"group": parts[0], "graph": parts[1]})
        line = file.readline()

random.shuffle(graphs)

N = len(graphs)
M = 0
threads = []
for k in range(K):
    g = graphs.pop(0)
    threads.append(Slave(g["group"], g["graph"], command + str(k)))

while True:
    for k, t in enumerate(threads):
        if not t.started:
            t.start()
        if t.finished:
            if len(graphs) == 0: break
            g = graphs.pop(0)
            threads[k] = Slave(g["group"], g["graph"], command + str(k))
    if len(graphs) == 0: break
    time.sleep(0.1)