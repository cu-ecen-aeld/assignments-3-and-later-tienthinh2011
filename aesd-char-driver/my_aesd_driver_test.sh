#!/bin/bash
sudo ./aesdchar_unload
sudo ./aesdchar_load
sudo echo -n 'w' > /dev/aesdchar
sudo echo -n 'r' > /dev/aesdchar
sudo echo -n 'it' > /dev/aesdchar
sudo echo 'e1' > /dev/aesdchar
# sudo echo 'write1' > /dev/aesdchar
sudo echo 'write2' > /dev/aesdchar
sudo echo 'write3' > /dev/aesdchar
echo "----read after write1, write2, write3 ------"
cat /dev/aesdchar
sudo echo 'write4' > /dev/aesdchar
sudo echo 'write5' > /dev/aesdchar
sudo echo 'write6' > /dev/aesdchar
sudo echo 'write7' > /dev/aesdchar
sudo echo 'write8' > /dev/aesdchar
sudo echo 'write9' > /dev/aesdchar
sudo echo 'write10' > /dev/aesdchar
sudo echo 'write11' > /dev/aesdchar
sudo echo -n 'w' > /dev/aesdchar
sudo echo -n 'r' > /dev/aesdchar
sudo echo -n 'it' > /dev/aesdchar
sudo echo 'e12' > /dev/aesdchar
echo "----read after write4, ..., write12 -------"
cat /dev/aesdchar 
echo ""
