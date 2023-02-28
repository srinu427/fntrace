import addr2line


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


if __name__ == "__main__":
    main()
