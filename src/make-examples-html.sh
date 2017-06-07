#!/bin/bash

example_categories=0
current_category=0

echo "<ol>"

for i in $(find examples-ltc | sort -n)
do
	if [ -d ${i} ]
	then
		if [ $(echo ${i} | tr '/' '\n' | wc -l) -eq 2 ]
		then
			id=$(echo $i | cut -d. -f2 | tr '[A-Z]' '[a-z]')
			name=$(echo $i | cut -d/ -f2)
			tmp1=$(echo ${name} | cut -d. -f1)
			tmp2=$(echo ${name} | cut -d. -f2)
			name="${tmp2}"
			if [ ${example_categories} -ne 0 ]
			then
				echo "  </ul>"
			fi
			example_categories=$((${example_categories} + 1))
			echo '  <li class="ExampleCategory">'${name}'</li>'
			echo '  <ul>'
			current_category=0
		fi
	elif [ -f ${i} ]
	then
		if echo "${i}" | grep -q '\.ino$'
		then
			id=$(echo $i | cut -d/ -f3 | tr '[A-Z]' '[a-z]')
			name=$(echo $i | rev | cut -d. -f2 | cut -d/ -f1 | rev | sed 's/\([a-z]\)\([A-Z]\)/\1 \2/g')
			current_category=$((${current_category} + 1))
			echo '    <li class="ExampleItem"><a href="'${i}'">'${name}'</a></li>'
		fi
	else
		echo "Other: $i"
	fi
done

echo "  </ul>"
echo "</ol>"
