rm result.csv
make
echo "MEM,func,k,AAE,ARE,_sum,throughput(insert),throughput(query)" >> result.csv

ruleDir1='/home/york/dataset/data'
for MEM in $(seq 100 100 1000)
do
    for K in 10 50 $(seq 100 100 1000)
    do
        for file in $ruleDir1/*.dat
        do
            cmd="./cuckoo -d $file -m $MEM -k $K"
            echo $cmd
            eval $cmd
            pid=$!
            wait $pid
            if ps -p $pid > /dev/null; then
                kill -9 $pid
            fi
            echo "done"
            sleep 1
        done
    done
done

ruleDir2='/home/york/dataset/zipf'
for MEM in $(seq 100 100 1000)
do
    for K in 10 50 $(seq 100 100 1000)
    do
        for file in $ruleDir2/zipf_*.dat
        do
            cmd="./cuckoo -d $file -m $MEM -k $K"
            echo $cmd
            eval $cmd
            pid=$!
            wait $pid
            if ps -p $pid > /dev/null; then
                kill -9 $pid
            fi
            echo "done"
            sleep 1
        done
    done
done
