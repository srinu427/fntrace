import subprocess
import json

def get_func_name(exec_path, addr):
    process = subprocess.Popen("addr2line --demangle -f -e " + exec_path + " " + addr, shell=True,
                           stdout=subprocess.PIPE, 
                           stderr=subprocess.PIPE)
    out, err = process.communicate()
    out = out.decode()
    out = out.split('\n')
    return {"fname": out[0], "file": out[1].split(":")[0], "line": out[1].split(":")[1]}


def main():
    outdata = []
    linecache = {}
    with open("trace_dump.txt", 'r') as fr:
        curr_tid = None
        for line in fr.readlines():
            if line.startswith("Thread ID: "):
                curr_tid = int(line.split("Thread ID: ")[-1][:-1])
                linecache[curr_tid] = []
            elif curr_tid is not None and line.startswith("func: "):
                sline = line[:-1].split(', ')
                func = sline[0].split("func: ")[-1]
                caller = sline[1].split("caller: ")[-1]
                caller_func = sline[2].split("caller_func: ")[-1]
                linecache[curr_tid] += [(func, caller, caller_func)]
    print(linecache)
    parsed_func = {}
    for tid in linecache:
        parsed_func[tid] = []
        for x in linecache[tid]:
            parsed_func[tid] += [{"func": get_func_name("main", x[0]), "caller": get_func_name("main", x[1]), "caller_func": get_func_name("main", x[2])}]
    print(json.dumps(parsed_func, indent=2))


if __name__ == "__main__":
    main()
