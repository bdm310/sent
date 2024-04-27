
import os.path

# https://arduino.github.io/arduino-cli/0.35/installation/#latest-release
CMD_ARDUINO_CLI = os.path.expanduser("~/Downloads/arduino-cli/arduino-cli")

ROOT = os.path.join(os.path.dirname(__file__), "../../")
BOARDS = os.path.join(ROOT, "test/boards.yaml")
OUTDIR = os.path.join(ROOT, "out")

import subprocess
from pprint import pprint
import yaml

def compile(board):
    sketch = os.path.join(ROOT, board["sketch"])
    cmd = [CMD_ARDUINO_CLI, "compile", "--build-path", OUTDIR, "--fqbn", board["fqbn"], sketch]
    p = subprocess.run(cmd)
    return p.returncode

def main():
    ret = dict()
    with open(BOARDS, "rb") as f:
        y = yaml.load(f, Loader=yaml.Loader)
        for board in y["boards"]:
            print(f"Compiling: {board['name']}")
            ret[board["name"]] = compile(board)
    
    pprint(ret)

if __name__ == "__main__":
    main()
