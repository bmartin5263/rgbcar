#!/bin/sh -e

apk add curl
apk add python3
apk add gcompat
apk add git

curl -fsSL -o get-platformio.py https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py
python3 get-platformio.py
export PATH=$PATH:$HOME/.local/bin

mkdir -p /usr/local/bin
ln -s ~/.platformio/penv/bin/platformio /usr/local/bin/platformio
ln -s ~/.platformio/penv/bin/pio /usr/local/bin/pio
ln -s ~/.platformio/penv/bin/piodebuggdb /usr/local/bin/piodebuggdb

cd rgbcar

pio account login -u "${PIO_USERNAME}" -p "${PIO_PASSWORD}"

./scripts/package.sh
./scripts/deploy.sh

cd ..
cd version

./scripts/increment-version.sh

git config --global user.email "bsm2112@yahoo.com"
git config --global user.name "Brandon Martin"
git add ./library.json
git commit -m "UpdateVersion"