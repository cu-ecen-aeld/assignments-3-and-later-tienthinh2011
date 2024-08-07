#!/bin/bash
sudo ./aesdchar_unload
sudo ./aesdchar_load
sudo echo 'write1' > /dev/aesdchar
sudo echo 'write2' > /dev/aesdchar
cat /dev/aesdchar 
