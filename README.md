# Itrace

I trace is a processor level runtime tracing tool built around the Intel
Processor Trace technology. 

## Install Dependencies

Install uv
```
curl -LsSf https://astral.sh/uv/install.sh | sh
```

Install perf
```
sudo apt install linux-perf
```

## Setup
Make Intel PT available for non root users by appending the following to `/etc/sysctl.conf`
```
echo "kernel.perf_event_paranoid=-1" | sudo tee -a /etc/sysctl.conf
```


