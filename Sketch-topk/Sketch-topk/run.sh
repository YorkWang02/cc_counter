set +o posix
rm result.csv
make
echo "MEM,func,k,AAE,ARE,_sum,throughput" >> result.csv
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
for MEM in  $(seq 50 10 150)
#for MEM in $(seq 100 100 1000)
# do
    #for K in $(seq 100 100 1000)
    do
        #cmd="./cuckoo -d $ruleDir -m 100 -k $K"
        cmd="./cuckoo -d $ruleDir -m $MEM -k 800" 
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
# done

