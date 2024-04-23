#!/bin/bash 
# Write a shell script finder-app/finder.sh as described below:

# Accepts the following runtime arguments: the first argument is a path to a directory on the filesystem, referred to below as filesdir; the second argument is a text string which will be searched within these files, referred to below as searchstr

# Exits with return value 1 error and print statements if any of the parameters above were not specified

# Exits with return value 1 error and print statements if filesdir does not represent a directory on the filesystem

# Prints a message "The number of files are X and the number of matching lines are Y" where X is the number of files in the directory and all subdirectories and Y is the number of matching lines found in respective files, where a matching line refers to a line which contains searchstr (and may also contain additional content).

# test string: thinhnt is thinhnt open thinhnt

if [ $# -lt 2 ]
then 
    echo "number of arguments should be >= 2"
    exit 1
else
    # Check arg#1 (should be an directory)
    # echo "1st argugment: " $1  # debug
    if [ -d $1 ]
    then
    # check arg#2 (should be a text string)
    # echo "2nd argugment: " $2 #debug
    # echo "cmd to run: grep -cr $2 $1"
    # For example, searching for lines containing the word "error" in a log file
    X=0
    Y=0
    # grep -cr $2 $1 | while IFS= read -r line; do # using this is not helpful because read run in different proceess -> changed value of x,y not recorded
    while IFS= read -r line; do 
        # Process each line of the output here
        ((X+= 1))
        count=$(grep -oE '[0-9]+$' <<< $line)
        ((Y+= count))
        # echo "Found $X: $line, Y=$Y"
    done < <(grep -cr $2 $1)
    echo "The number of files are $X and the number of matching lines are $Y"
    # tmp=$2
    # echo $tmp
    else # Exits with return value 1 error and print statements if filesdir does not exists
    echo "Error: $1 is not a directory"
    exit 1
    fi
fi
exit 0

