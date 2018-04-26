#!/bin/bash

echo "Cleanup script started..."

#Delete the created veths
echo "Before Removing veths"
for (( i=1; i<7; i++ ))
do
	echo "Current veths in namespace "$i":"
	ip netns exec "ns"$i ip link show
	echo -e "\n"
done

echo "Removing veths"
ip netns exec ns1 ip link delete veth1_5
ip netns exec ns2 ip link delete veth2_5
ip netns exec ns3 ip link delete veth3_6
ip netns exec ns4 ip link delete veth4_6
ip netns exec ns5 ip link delete veth5_6

echo "After Removing veths"
for (( i=1; i<7; i++ ))
do
	echo "Current veths in namespace "$i":"
	ip netns exec "ns"$i ip link show
	echo -e "\n"
done

#Delete the created bridges
echo "Before removing bridge of ns5:"
ip netns exec ns5 brctl show
echo -e "\n"

echo "Before removing bridge of ns6:"
ip netns exec ns6 brctl show
echo -e "\n"

echo "Removing bridges"
ip netns exec ns5 ifconfig bridge_ns5 down
ip netns exec ns5 brctl delbr bridge_ns5
ip netns exec ns6 ifconfig bridge_ns6 down
ip netns exec ns6 brctl delbr bridge_ns6

echo "After removing bridge of ns5:"
ip netns exec ns5 brctl show

echo "After removing bridge of ns6:"
ip netns exec ns6 brctl show

echo "Current namespaces:"
ip netns list

#Delete the created namespaces
echo "Removing namespaces"
for(( i=1; i<7; i++ ))
do
	echo "Removing namespace "$i
	ip netns del "ns"$i
done

echo "Current namespaces:"
ip netns list

echo "Cleanup done..."
