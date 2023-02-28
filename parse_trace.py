import subprocess
import json
import copy

def get_func_name(exec_path, addr):
    if exec_path == "" or exec_path == "(null)" in exec_path:
        exec_path = "main"
    process = subprocess.Popen("addr2line --demangle -f -e " + exec_path + " " + addr, shell=True,
                           stdout=subprocess.PIPE, 
                           stderr=subprocess.PIPE)
    out, err = process.communicate()
    out = out.decode()
    out = out.split('\n')

    return {"fname": out[0], "exec": exec_path, "file": out[1].split(":")[0], "line": out[1].split(":")[1]}


def main():
    outdata = []
    linecache = {}
    with open("trace_dump.txt", 'r') as fr:
        for line in fr.readlines():
            sline = line[:-1].split(',')
            timestamp = int(sline[0].split("ts: ")[-1])
            curr_tid = sline[1].split("tid: ")[-1]
            curr_pid = sline[2].split("pid: ")[-1]
            callee = sline[3].split("f: ")[-1]
            callee_dl = sline[4].split("l: ")[-1]
            caller = sline[5].split("c: ")[-1]
            caller_dl = sline[6].split("l: ")[-1]
            if curr_pid not in linecache:
                linecache[curr_pid] = {}
            if curr_tid not in linecache[curr_pid]:
                linecache[curr_pid][curr_tid] = []
            linecache[curr_pid][curr_tid] += [(callee, callee_dl, caller, caller_dl, timestamp)]
    # print(linecache)

    parsed_func = {}
    for pid in linecache:
        parsed_func[pid] = {}
        for tid in linecache[pid]:
            parsed_func[pid][tid] = []
            for x in linecache[pid][tid]:
                parsed_func[pid][tid] += [{"callee": get_func_name(x[1], x[0]), "caller": get_func_name(x[3], x[2]), "timestamp": x[4]}]
    # print(json.dumps(parsed_func, indent=2))

    func_call_dict = {}
    for pid in parsed_func:
        func_call_dict[pid] = {}
        for tid in parsed_func[pid]:
            func_call_dict[pid][tid] = {}
        for x in parsed_func[pid][tid]:
            callee_name = x["callee"]["fname"]
            caller_name = x["caller"]["fname"]
            if not callee_name in func_call_dict[pid][tid]:
                func_call_dict[pid][tid][callee_name] = {}
            if "called_by" not in func_call_dict[pid][tid][callee_name]:
                func_call_dict[pid][tid][callee_name]["called_by"] = {}
            if caller_name not in func_call_dict[pid][tid][callee_name]["called_by"]:
                func_call_dict[pid][tid][callee_name]["called_by"][caller_name] = {"count": 0, "all_calls": []}
            func_call_dict[pid][tid][callee_name]["called_by"][caller_name]["count"] += 1
            func_call_dict[pid][tid][callee_name]["called_by"][caller_name]["all_calls"] += [{"exec": x["caller"]["exec"], "file": x["caller"]["file"], "line": x["caller"]["line"], "timestamp": x["timestamp"]}]

            if not caller_name in func_call_dict[pid][tid]:
                func_call_dict[pid][tid][caller_name] = {}
            if "called" not in func_call_dict[pid][tid][caller_name]:
                func_call_dict[pid][tid][caller_name]["called"] = {}
            if callee_name not in func_call_dict[pid][tid][caller_name]["called"]:
                func_call_dict[pid][tid][caller_name]["called"][callee_name] = {"count": 0, "all_calls": []}
            func_call_dict[pid][tid][caller_name]["called"][callee_name]["count"] += 1
            func_call_dict[pid][tid][caller_name]["called"][callee_name]["all_calls"] += [{"exec": x["callee"]["exec"], "file": x["callee"]["file"], "line": x["callee"]["line"], "timestamp": x["timestamp"]}]
    print(func_call_dict)
    with open("parsed_trace.json", "w") as fw:
        fw.write(json.dumps(func_call_dict, indent=2))


if __name__ == "__main__":
    main()
