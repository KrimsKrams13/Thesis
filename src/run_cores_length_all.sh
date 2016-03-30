i=1
while [ "$i" -lt $1 ]    # this is loop1
do
	make clean
	make NUM_THREADS=$i
	./run 2 4
	i=`expr $i + 1`
done