This is a process manager, it can read a provide file and store the data as "process" then arrange them with 'SJF'or 'RR'

You should write file with format following:
Arrive Time, Process Name, Service Time, Memory Require

For example:
0, P1, 50, 80

This scheduler have 2048 unit of memory capacity (default).

You should run 'make' command before run scheduler.

You should run the scheduler with command following:
./allocate -f 'text_file' -s 'strategy_you_like' -m 'inifite_or_limit_memory' -q 'quantum_time'

You can arrange the command line arguments in any order.

This is a algorithm-oriented simlulation, doesn't hurt real memory on your device.

To try arrange with real processes, you should have 'process' executable file that compiled by process.c that provid by UniMelb COMP30023.
