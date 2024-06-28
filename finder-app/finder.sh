#!/bin/sh
# Write a shell script finder-app/finder.sh as described below:
# Accepts the following runtime arguments: the first argument is a path to a directory on the filesystem, referred to below as filesdir; 
# the second argument is a text string which will be searched within these files, referred to below as searchstr
# Exits with return value 1 error and print statements if any of the parameters above were not specified
# Exits with return value 1 error and print statements if filesdir does not represent a directory on the filesystem
# Prints a message "The number of files are X and the number of matching lines are Y" 
# where X is the number of files in the directory and all subdirectories and Y is the number of matching lines found in respective files, 
# where a matching line refers to a line which contains searchstr (and may also contain additional content).
# test string: thinhnt is thinhnt open thinhnt
# echo "finder.sh"
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
    X=0 # number of files 
    Y=0 # number of matching lines 
    # grep -cr $2 $1 | while IFS= read -r line; do # using this is not helpful because read run in different proceess -> changed value of x,y not recorded    
    # output=$(grep -cr $2 $1) # using tempfile for sh (ash)
    # echo "ThinhNT"
    # echo "output=$output"
    tmpFile="tmpFile.txt"
    grep -cr $2 $1 > $tmpFile
    tmpX=$(mktemp)
    echo $X > $tmpX
    tmpY=$(mktemp)
    echo $Y > $tmpY
    # output=$(grep -cr $2 $1) # using tempfile for sh (ash)
    cat $tmpFile | {
        while IFS= read -r line; do 
            # Process each line of the output here
            X=$((X+ 1))
            #<<< $line: redirection here string in bash only (not in sh)
            count=$(echo $line | grep -oE '[0-9]+$') #-o: only matching, -E: interpret patterns as extended regular expressions // $= at the end of line
            Y=$((Y+count))
            # echo "Found $X: $line, Y=$Y"
            # Process Substitution <(...) => provide input for while loop #Chapter 23. Process Substitution (abs-guide)
        done
        # Change to use temp file because /bin/sh doesn't support process substitution like bash
        echo $X > $tmpX
        echo $Y > $tmpY
    }
    X=$(cat "$tmpX")
    Y=$(cat "$tmpY")
    rm "$tmpX"
    rm "$tmpY"
    rm "$tmpFile"
    echo "The number of files are $X and the number of matching lines are $Y"
    # tmp=$2
    # echo $tmp
    else # Exits with return value 1 error and print statements if filesdir does not exists
    echo "Error: $1 is not a directory"
    exit 1
    fi
fi
exit 0

