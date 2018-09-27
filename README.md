# rubbis_exp
## usage

First, In *exp_pattern.sh*

- change *vmssh* to your own
- change *lc* to your own
- change the ip address in *url* to your own ip address

Then, run *exp_preparedata.sh*

Data will be writen in *internal_data.txt* and *external_data.txt* with the following format:

​	**preference_order, CPU_cap, concurrency, init_latency, weights** in internal_data.txt

​	**init_latency, init_serviceLevel, new_serviceLevel** in external_data.txt


