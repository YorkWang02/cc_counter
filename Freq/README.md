# Cuckoo_Counter Frequency Estimation

## How to run

Suppose you've already cloned the repository.

You just need:

```
$ make 
$ ./cuckoo ./XX.data (Memory)
```

`XX.data` is a dataset, Memory is the memory usage (unit is MB).

## Output format

Our program will print the Insert throughput, Query throughput, AAE, ARE of Cuckoo Counter ans seven other sketches.
