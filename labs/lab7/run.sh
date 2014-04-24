
for ((i=1;i<10;i++))
do echo "Running $i:"
./matmul_mp$((2**i))
done
