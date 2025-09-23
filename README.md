# Itrace

itrace is a processor level runtime tracing tool built around the Intel
Processor Trace technology. 

## Setup
```
sudo apt install linux-tools-generic
echo -1 | sudo tee /proc/sys/kernel/perf_event_paranoid
./setup.py
```
