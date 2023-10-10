rm result_cc3_1.csv
make
echo "MEM,func,k,AAE,ARE,throughput" >> result_cc3_1.csv
ruleDir='/home/york/dataset/temp'
for MEM in $(seq 100 100 1000)
do
    for file in $ruleDir/*.dat
    do
        cmd="./cuckoo -d $file -m $MEM -k 1000"
        echo $cmd
        eval $cmd
        pid = $!
        wait $pid
        if ps -p $pid > /dev/null; then
            kill -9 $pid
        fi
        echo "done"
        sleep 1
    done
done

