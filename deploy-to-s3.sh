#!/bin/sh

print_usage() {
	echo "$0 -b [bucket] -r [region]"
	echo "    -b [bucket]     S3 bucket name"
	echo "    -r [region]     S3 region"
	echo "  Note: You can use environment variables to pass settings."
	echo "For example, if \$bucket is set, that value will be honored."
	echo "Arguments override environment variables, so if you specify"
	echo "\$bucket and -b, the -b value will be used."
}

while getopts "r:b:" opt
do
	case $opt in
	r)
		region=$OPTARG
		;;
	b)
		bucket=$OPTARG
		;;
	h)
		print_usage
		exit 0
		;;
	\?)
		echo "Unrecognized option: -$OPTARG"
		print_usage
		exit 1
		;;
	esac
done

if [ -z ${region} ]
then
	print_usage
	exit 1
fi

if [ -z ${bucket} ]
then
	print_usage
	exit 1
fi
exit 0

# Figure out a serial number based on today's date.
# The first invokation of the day should result in -01, and
# every subsequent run should increase that by 1.
dateval=$(date +'%Y%m%d')
serial=1
if grep -q ${dateval} .deploydata 2> /dev/null
then
	serial=$(($(grep ${dateval} .deploydata | awk '{print $2}')+1))
fi
echo "# This file contains serial deployment data, as used by deploy-to-s3.sh" > .deploydata
printf "${dateval} ${serial}" >> .deploydata
fileprefix=$(printf "%s-%02d" ${dateval} ${serial})

s3-deploy './build/**' --cwd ./build/ --region ${region} --bucket ${bucket} --filePrefix ${fileprefix}

echo "File prefix: ${fileprefix}"
echo "Region: ${region}"
echo "Bucket: ${bucket}"
