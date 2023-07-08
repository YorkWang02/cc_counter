# Cuckoo Counter Heavy Hitter Detection

## How to run

Suppose you've already cloned the repository.

You just need:

```
$ make 
$ ./cuckoo ./XX.data (Memory)
```

`XX.data` is a dataset, Memory is the memory usage (unit is MB).

## Output format

Our program will print the AAE, ARE, Recall, Precision, F1-score of Cuckoo Counter and other seven sketches.
