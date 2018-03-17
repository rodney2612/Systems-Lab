if (echo $1 | grep -Eqx "((\*)|([0-9]+,)*[0-9]+)|([0-9]+-[0-9]+)|([0-9]+)|")
then
	echo "true" 
else 
	echo "false" 
fi
