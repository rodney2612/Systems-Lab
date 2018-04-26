#!/bin/bash

echo "Create script started..."

#Create 6 namespaces ns1 - ns6
echo "Creating namespaces..."
for(( i=1; i<7; i++ ))
do
	ip netns add "ns"$i
done

echo "Current namespace list:"
ip netns list
echo -e "\n"

#Create veth pairs
ip link add veth1_5 type veth peer name veth5_1
ip link add veth2_5 type veth peer name veth5_2
ip link add veth3_6 type veth peer name veth6_3
ip link add veth4_6 type veth peer name veth6_4
ip link add veth5_6 type veth peer name veth6_5

#echo "Current veths in default namespace:"
#ip link show

declare -a ns_array=("ns1" "ns2" "ns3" "ns4" "ns5" "ns5" "ns5" "ns6" "ns6" "ns6")
declare -a veth_array=("veth1_5" "veth2_5" "veth3_6" "veth4_6" "veth5_6" "veth5_1" "veth5_2" "veth6_3" "veth6_4" "veth6_5")

#Place veths in namespaces
for (( i=0; i<10; i++ ))
do
	ip link set ${veth_array[i]} netns ${ns_array[i]}
done

#Set 10 ms delay for each veth using tc
for (( i=0; i<10; i++ ))
do
	ip netns exec ${ns_array[i]} tc qdisc add dev ${veth_array[i]} root netem delay 10ms
	echo "Delay set in "${veth_array[i]}":"
	ip netns exec ${ns_array[i]} tc qdisc show dev ${veth_array[i]}
	echo -e "\n"
done

#Assign the appropriate veths to the bridge interfaces
ip netns exec ns1 ip addr add 10.0.0.1/24 dev veth1_5
ip netns exec ns2 ip addr add 10.0.0.2/24 dev veth2_5
ip netns exec ns3 ip addr add 10.0.0.3/24 dev veth3_6
ip netns exec ns4 ip addr add 10.0.0.4/24 dev veth4_6

#Bring up interfaces
for (( i=0; i<10; i++ ))
do
	ip netns exec ${ns_array[i]} ip link set ${veth_array[i]} up
done

for (( i=1; i<7; i++ ))
do
	echo "Current veths in namespace "$i":"
	ip netns exec "ns"$i ip link show
	echo -e "\n"
done

#Create bridges
#declare -a veths_5=("veth5_1" "veth5_2" "veth5_6")
#declare -a veths_6=("veth6_3" "veth6_4" "veth6_5")
#for(( i=5; i<7; i++ ))
#do
#	ip netns exec "ns"$i brctl addbr "bridge_ns"$i
#	for(( j=1; j<4; j++))
#	do
#		ip netns exec "ns"$i brctl addif "bridge_ns"$i ${"veths_"$i[$j]}
#	done
#	ip netns exec "ns"$i ip link set "bridge_ns"$i up
#done

#Create bridges
ip netns exec ns5 brctl addbr bridge_ns5
ip netns exec ns6 brctl addbr bridge_ns6

#Assign the appropriate veths to the bridge interfaces
ip netns exec ns5 brctl addif bridge_ns5 veth5_1
ip netns exec ns5 brctl addif bridge_ns5 veth5_2
ip netns exec ns5 brctl addif bridge_ns5 veth5_6

ip netns exec ns6 brctl addif bridge_ns6 veth6_3
ip netns exec ns6 brctl addif bridge_ns6 veth6_4
ip netns exec ns6 brctl addif bridge_ns6 veth6_5

#Bring bridges up
ip netns exec ns5 ip link set bridge_ns5 up
ip netns exec ns6 ip link set bridge_ns6 up

echo "Bridge and its interfaces (of namespace 5):"
ip netns exec ns5 brctl show
echo -e "\n"

echo "Bridge and its interfaces (of namespace 6):"
ip netns exec ns6 brctl show
echo -e "\n"

#Ping from one namespace to other and check if they are working fine
ip_prefix="10.0.0."
for(( i=1; i<5; i++ ))
do
	for(( j=1; j<5; j++))
	do
		if [ $i -eq $j ]
		then
			continue
		fi
		echo "Pinging from ns"$i" to ns"$j"..."
		ip netns exec "ns"$i ping -c 2 $ip_prefix$j
		echo -e "\n"
	done
done

echo "Create script done..."
