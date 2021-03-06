#!/bin/bash
	if [ testResults.csv ]; then
    		rm testResults.csv 
	fi
	for file in data/*.mtx
	do
	 # do something on "$file"
		#file name is now like data/Matrix.mtx
		#the following lines are used to remove .mtx and /data	  
		#split file name using '.' delimeter 
		IFS='.'
		read -a direc <<< "${file}"
		#split file name using '.' delimeter 
		IFS='/'
		read -a name <<< "${direc}"

		#Put gcolor result and profilings in appropriate folders
		examples/gcolor -i "$file" -m slo >> testResults.csv
		examples/gcolor -i "$file" -m ido >> testResults.csv
		examples/gcolor -i "$file" -m rlf >> testResults.csv
		examples/gcolor -i "$file" -m lfo >> testResults.csv
		examples/gcolor -i "$file" -m sdo >> testResults.csv
	done
	echo "Done!!"
