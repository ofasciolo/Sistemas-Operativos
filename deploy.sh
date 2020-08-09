#!/bin/bash

length=$(($#-1))
OPTIONS=${@:1:$length}
REPONAME="${!#}"
CWD=$PWD

echo -e "\n\nInstalling commons libraries...\n\n"

COMMONS="so-commons-library"
git clone "https://github.com/sisoputnfrba/${COMMONS}.git" $COMMONS
cd $COMMONS
sudo make uninstall
make all
sudo make install
cd $CWD

echo -e "\n\nBuilding projects...\n\n"

make -C ./broker
make -C ./gameboy
make -C ./gamecard
make -C ./team

git config --global user.email "dariokozicki@gmail.com"
git config --global user.name "Dario"

git add .
git stash

echo -e "\n\nDeploy done!\n\n"