import os
import sys
import time
import configparser

while True:

    os.system('cls' if os.name == 'nt' else 'clear')

    for folder in os.listdir("."):
        if os.path.isfile(folder): continue

        config = configparser.ConfigParser()
        config.optionxform = str
        config_path = os.path.join(folder, "gls.ini")


        print(folder)

        try:
            with open(config_path, "r") as fp:
                config.read_file(fp)
            print(config.get("gls", "MAX_NO_IMPROVE") + " " + config.get("gls", "ASPIRATION") + " " + config.get("gls", "RESET_WEIGHTS") + " " + config.get("gls", "LAMBDA"))
        except:
            pass
    time.sleep(2)
