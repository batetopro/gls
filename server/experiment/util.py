import os
import configparser
import subprocess
import shutil


def gls(graph_path, tid, config_path, db, **kwargs):
    config = configparser.ConfigParser()
    config.optionxform = str

    with open(config_path, "r") as fp:
        config.read_file(fp)

    for k in kwargs:
        config.set("gls", k, str(kwargs[k]))

    path = os.path.join(db, "exec", tid)
    exe = os.path.join(path, "gls.exe")
    ini = os.path.join(path, "gls.ini")
    cmd = exe + " " + graph_path

    if not os.path.exists(path):
        os.makedirs(path)
        shutil.copyfile(os.path.join(db, "bin", "gls.exe"), exe)

    with open(ini, "w") as fp:
        config.write(fp)

    cwd = os.getcwd()
    os.chdir(path)
    data = subprocess.getoutput(cmd)
    os.chdir(cwd)
    
    result = []
    for line in data.split("\n"):
        result.append('"' + line.strip().replace(',', '";"') + '"')

    return "\n".join(result)


def read_graph(group, graph, base_path):
    graph_path = os.path.join(base_path, "graphs", group, graph)
    if not os.path.exists(graph_path):
        return False

    with open(os.path.join(base_path, "graphs.csv"), "r") as file:
        header = file.readline().strip('"').split('";"')
        line = file.readline()
        while line:
            parts = line.strip().strip().split('";"')
            for k,p in enumerate(parts):
                parts[k] = p.strip('"')
            if len(parts) == 1:
                line = file.readline()
                continue

            E = {}
            for k, v in enumerate(header):
                E[v] = parts[k]

            if E["source"] == group and E["graph"] == graph:
                E["path"] = graph_path
                E["folder"] = os.path.join(base_path, "experiment", group, graph)

                if not os.path.exists(E["folder"]):
                    os.makedirs(E["folder"])

                return E

            line = file.readline()
    return False


def strip_csv(line, sep=";", quote='"'):
    return line.strip().strip(quote).split(quote+sep+quote)


def read_csv(path, sep=";", quote='"'):
    result = []
    with open(path, "r", encoding="utf8") as file:
        header = strip_csv(file.readline(), sep, quote)
        line = file.readline()

        entry = {}
        while line:
            row = strip_csv(line)

            if len(row) == 1:
                result[-1][header[-1]] += "\n" + row[0]
                line = file.readline()
                continue

            for k, h in enumerate(header):
                if k >= len(row):
                    entry[h] = ""
                else:
                    entry[h] = row[k]

            result.append(entry)
            entry = {}

            line = file.readline()
    return result

