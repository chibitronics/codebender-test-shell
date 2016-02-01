#!/bin/bash

example_categories=0
current_category=0

echo "["

for i in $(find examples | sort -n)
do
	if [ -d ${i} ]
	then
		if [ $(echo ${i} | tr '/' '\n' | wc -l) -eq 2 ]
		then
			id=$(echo $i | cut -d. -f2 | tr '[A-Z]' '[a-z]')
			name=$(echo $i | cut -d/ -f2)
			tmp1=$(echo ${name} | cut -d. -f1)
			tmp2=$(echo ${name} | cut -d. -f2)
			name="${tmp1}. ${tmp2}"
			if [ ${example_categories} -ne 0 ]
			then
				echo '      }'
				echo '    ]'
				echo "  },"
			fi
			example_categories=$((${example_categories} + 1))
			echo "  {"
			echo '    "id": "'${id}'",'
			echo '    "text": "'${name}'",'
			echo '    "children": ['
			current_category=0
		fi
	elif [ -f ${i} ]
	then
		if echo "${i}" | grep -q '\.ino$'
		then
			id=$(echo $i | cut -d/ -f3 | tr '[A-Z]' '[a-z]')
			name=$(echo $i | cut -d/ -f3)
			if [ ${current_category} -ne 0 ]
			then
				echo '      },'
			fi
			current_category=$((${current_category} + 1))
			echo '      {'
			echo '        "id": "'${id}'",'
			echo '        "text": "'${name}'",'
			echo '        "data": "'${i}'",'
			echo '        "type": "file"'
		fi
	else
		echo "Other: $i"
	fi
done

echo "      }"
echo "    ]"
echo "  }"
echo "]"
