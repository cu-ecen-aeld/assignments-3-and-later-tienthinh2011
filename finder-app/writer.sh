#!/bin/bash
# 1. Accepts the following arguments: the first argument is a full path to a file 
# (including filename) on the filesystem, referred to below as writefile; 
# the second argument is a text string which will be written within this file, referred to below as writestr

# 2. Exits with value 1 error and print statements if any of the arguments above were not specified

# 3. Creates a new file with name and path writefile with content writestr, 
# overwriting any existing file and creating the path if it doesn’t exist. 
# Exits with value 1 and error print statement if the file could not be created.

# example: writer.sh /tmp/aesd/assignment1/sample.txt ios
# create /tmp/aesd/assignment1/sample.txt with content ios

if [ $# -lt 2 ]
then 
    echo "number of arguments should be >= 2"
    exit 1
else
    # echo "cmd: echo $2 > $1"
    folder=$(dirname "$1")
    mkdir -p $folder
    if [ $? -ne 0 ] 
    then
        echo "Cannot create folder $folder"
        exit 1
    fi
    echo $2 > $1
    if [ $? -ne 0 ] 
    then
        echo "Cannot create or write on the file"
        exit 1
    fi
fi
exit 0