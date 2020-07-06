#!/bin/bash
sudo apt-get update 
sudo apt-get install git -y
git clone https://github.com/eclipse/paho.mqtt.c.git mqtt
cd mqtt 
make
sudo make install
sudo apt-get install openssl -y
sudo apt-get install libssl-dev -y
sudo apt-get install -y libcurl-dev
sudo apt-get install -y libjson-c-dev