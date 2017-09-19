def strip_csv(line, sep=";", quote='"'):
    return line.strip().strip(quote).split(quote+sep+quote)


def read_csv(path, sep=";", quote='"'):
    result = []
    with open(path, "r") as file:
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

