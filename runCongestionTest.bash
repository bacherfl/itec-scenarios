#!/bin/bash
#compile
./waf

#run test
./waf --run congestiontest 2> out.txt
grep "stats(" out.txt > out2.txt

#throw away first 2501 lines (= first 5 seconds)
tail -n+2501  out2.txt > out3.txt

# get all lines for client 1
awk 'NR % 5 == 0' out3.txt > client1.txt
# get all lines for client 2
awk 'NR % 5 == 1' out3.txt > client2.txt
awk 'NR % 5 == 2' out3.txt > client3.txt
awk 'NR % 5 == 3' out3.txt > client4.txt
awk 'NR % 5 == 4' out3.txt > client5.txt


./extractCwnd.bash client1.txt > client1_new.txt
./extractCwnd.bash client2.txt > client2_new.txt
./extractCwnd.bash client3.txt > client3_new.txt
./extractCwnd.bash client4.txt > client4_new.txt
./extractCwnd.bash client5.txt > client5_new.txt


gnuplot << EOF
set terminal png size 20000,500
set output "output.png"
plot 'client1_new.txt' with lines, \
'client2_new.txt' with lines, \
'client3_new.txt' with lines, \
'client4_new.txt' with lines, \
'client5_new.txt' with lines
EOF


