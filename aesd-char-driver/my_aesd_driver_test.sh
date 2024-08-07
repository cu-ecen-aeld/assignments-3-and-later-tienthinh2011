#!/bin/bash
sudo ./aesdchar_unload
sudo ./aesdchar_load
sudo echo 'write1' > /dev/aesdchar
sudo echo 'write2' > /dev/aesdchar
sudo echo 'write3' > /dev/aesdchar
sudo echo 'write4' > /dev/aesdchar
sudo echo 'write5' > /dev/aesdchar
sudo echo 'write6' > /dev/aesdchar
sudo echo 'write7' > /dev/aesdchar
sudo echo 'write8' > /dev/aesdchar
sudo echo 'write9' > /dev/aesdchar
sudo echo 'write10' > /dev/aesdchar
sudo echo 'write11' > /dev/aesdchar
sudo echo 'write12' > /dev/aesdchar
cat /dev/aesdchar 
