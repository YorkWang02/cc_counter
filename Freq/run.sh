set +o posix
rm output.csv
make
echo "MEM,func,throughput(insert),throughput(query),AAE,ARE" >> output.csv
ruleDir='/home/york/dataset/data/1.dat'
# chmod +r /home/york/dataset/data/*.dat
# #ruleDir[40]="/Users/caolu/Downloads/Data/1.dat"
# if [ -z "${ruleDir[0]}" ]; then
#     ruleDir="${ruleDir[40]}"
# fi
echo "ruleDir: $ruleDir"
for ((int i = 1; i <= 1000000; i++))
do
		read -r -n 13 tmp
        tmp="${tmp%'\n'}"
        s[$i]="$tmp"
        ((B[$tmp]++))
done
#for MEM in  $(seq 0.1 0.1 1.0)
for MEM in $(seq 0.01 0.01 0.1)
# do
    #for K in $(seq 10 10 100)
    do
        for ((i = 1; i <= 50; i++)) # 循环运行 30 次
        do
            cmd="./cuckoo $ruleDir $MEM" 
            echo $cmd
            eval $cmd
            pid=$! # 保存进程 ID
            wait $pid
            if ps -p $pid > /dev/null; then
            kill -9 $pid
            fi
            echo "done"
            sleep 1
        done
    done
    python3 figure_aae.py
    python3 figure_are.py
    python3 figure_throughput_insert.py
    python3 figure_throughput_query.py
# done

