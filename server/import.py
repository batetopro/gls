import os
import re
import time
import subprocess
import shutil
import configparser


def remove_extension(path):
    return os.path.splitext(path)[0]


def parse_graph(G):
    with open(G["source"], "r") as f:
        data = {}
        n, m = 0, 0

        line = f.readline()
        while line:
            line = line.strip()
            line = re.sub(' +', ' ', line)

            if line.startswith("min "):
                parts = line.split(" ")
                G["min"] = parts[1]

            if line.startswith("c "):
                G["comment"] += line[2: ] + "\n"

            if line.startswith("e "):
                parts = line.split(" ")
                l, r = int(parts[1]), int(parts[2])
                if l > n: n = l
                if r > n: n = r

                if l not in data: data[l] = set()
                if r not in data: data[r] = set()

                data[l].add(r)
                data[r].add(l)

            line = f.readline()

        m = sum([len(data[k]) for k in data])
        m = int(m / 2)

        min_deg = m
        max_deg = 0
        sum_deg = 0.0

        with open(G["target"], "w") as f:
            f.write("{0} {1}\n".format(n, m))
            for i in range(1, n + 1):
                if i in data:
                    f.write("{0}\n".format(" ".join([str(o) for o in data[i]])))

                    if len(data[i]) > max_deg:
                        max_deg = len(data[i])

                    if len(data[i]) < min_deg:
                        min_deg = len(data[i])

                    sum_deg += len(data[i])
                else:
                    min_deg = 0
                    f.write("\n")

        G["N"] = n
        G["M"] = m
        G["deg_min"] = min_deg
        G["deg_max"] = max_deg
        G["deg_avg"] = sum_deg / float(n)


def stat_graph(G):
    with open(G["source"]) as file:
        line = file.readline().strip().split(" ")
        G["N"] = int(line[0])
        G["M"] = int(line[1])
        
        L = {}
        line = file.readline().strip().split(" ")
        while line[0] != '':
            l = len(line)
            if l not in L:
                L[l] = 0
            L[l] += 1
            line = file.readline().strip().split(" ")
      
    G["deg_min"] = min(L)
    G["deg_max"] = max(L)
    G["deg_avg"] = sum([L[k] for k in L]) / len(L)
    shutil.copy(G["source"], G["target"])
    
        
def upper_bounds(G):
    cmd = os.path.join(db, "bin","upper.exe") + " " + G["target"]
    upper = os.popen(cmd).read().strip().split(",")
    G["d"] = upper[0]
    G["t2"] = upper[1]
    G["t3"] = upper[2]
    G["greedy"] = upper[3]


def get_name(G):
    p = G["target"].split(os.path.sep)
    G["name"] = p[-1]
    G["group"] = p[-2]


def gls_graph(G):
    config = configparser.ConfigParser()
    config.optionxform = str

    with open("import.ini", "r") as fp:
        config.read_file(fp)

    config.set("gls", "LOWER_BOUND", str(int(G["min"])))

    with open("gls.ini", "w") as fp:
        config.write(fp)

    cmd = os.path.join(db, "bin", "gls.exe") + " " + G["target"]

    start = time.time()
    with open(G["color"], "w") as out:
        subprocess.call(cmd, stdout=out)
    G["gls_time"] = time.time() - start

    C = set()
    with open(G["color"], "r") as inp:
        line = inp.readline()
        line = inp.readline()
        while line:
            C.add(int(line.split(" ")[0]))
            line = inp.readline()

    G["gls"] = max(C) + 1


def append_graph(G, path, columns):
    quote = '"'
    sep = ";"
    row = [quote + str(G[c]) + quote for c in columns]
    with open(path, "a", encoding="utf8") as file:
        file.write(sep.join(row) + "\n")


def read_no_parse():
    path = os.path.join(db, "noparse.in")
    result = set()
    
    if not os.path.exists(path):
        return result
    
    with open(path) as file:
        line = file.readline().strip()
        while line:
            result.add(line)            
            line = file.readline().strip()
    
    return result
        
if __name__ == "__main__":
    db = os.path.join(os.getcwd(), "db")
    Q = [{"target": "graphs", "source": "import", "color": "coloring"}]
    csv = os.path.join(db, "graphs.csv")
    
    NP = read_no_parse()
    
    while len(Q) > 0:
        e = Q.pop(0)
        s = os.path.join(db, e["source"])
        t = os.path.join(db, e["target"])
        c = os.path.join(db, e["color"])

        if os.path.isfile(s):
            G = {"source": s, "target": remove_extension(t), "color": remove_extension(c), "min": 2, "comment": ""}
            get_name(G)
            print(len(Q), G["group"], G["name"])
            if G["group"] in NP:
                stat_graph(G)
            else:
                parse_graph(G)
            upper_bounds(G)
            gls_graph(G)
            append_graph(G, csv, ["group", "name", "N", "M", "deg_min", "deg_max", "deg_avg", "min", "d", "t2", "t3", "greedy", "gls", "gls_time", "comment"])
            os.remove(G["source"])
            continue

        if not os.path.exists(s):
            os.mkdir(t)
            print("Folder " + s + " was empty.")

        if not os.path.exists(t):
            os.mkdir(t)

        if not os.path.exists(c):
            os.mkdir(c)

        for f in os.listdir(s):
            Q.append({
                "target": os.path.join(e["target"], f),
                "source": os.path.join(e["source"], f),
                "color": os.path.join(e["color"], f)
            })