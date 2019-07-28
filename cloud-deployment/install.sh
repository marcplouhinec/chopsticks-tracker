#!/usr/bin/env bash

# Update the system
export DEBIAN_FRONTEND=noninteractive
apt-get -y update
apt-get -y upgrade

# Install core development tools
apt-get -y install git cmake

# Install CUDA
cd /tmp
wget https://developer.nvidia.com/compute/cuda/10.1/Prod/local_installers/cuda-repo-ubuntu1804-10-1-local-10.1.168-418.67_1.0-1_amd64.deb
dpkg -i cuda-repo-ubuntu1804-10-1-local-10.1.168-418.67_1.0-1_amd64.deb
apt-key add /var/cuda-repo-10-1-local-10.1.168-418.67/7fa2af80.pub
apt-get -y update
apt-get -y install cuda
nvidia-smi
export PATH=$PATH:/usr/local/cuda/bin

# Boost compilation and installation
cd ~/projects
wget https://dl.bintray.com/boostorg/release/1.70.0/source/boost_1_70_0.tar.gz
tar -xzf boost_1_70_0.tar.gz
rm -f boost_1_70_0.tar.gz
cd boost_1_70_0

./bootstrap.sh --prefix=/usr/local/
./b2
./b2 headers
sudo ./b2 install

# RapidJSON compilation and installation
cd ~/projects
git clone -b v1.1.0 --single-branch https://github.com/Tencent/rapidjson.git
cd rapidjson

mkdir -p build
cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE \
    -D CMAKE_INSTALL_PREFIX=/usr/local \
    -D RAPIDJSON_BUILD_DOC=OFF \
    -D RAPIDJSON_BUILD_EXAMPLES=OFF \
    -D RAPIDJSON_BUILD_TESTS=OFF \
    -D RAPIDJSON_BUILD_THIRDPARTY_GTEST=OFF \
    -D RAPIDJSON_BUILD_CXX11=ON \
    ..
make
sudo make install

# FFMpeg compilation and installation
sudo apt-get -y install nasm pkg-config

cd ~/projects
git clone -b release/4.1 --single-branch https://git.ffmpeg.org/ffmpeg.git
cd ffmpeg

./configure \
    --prefix=/usr/local \
    --enable-gpl --enable-version3 \
    --enable-shared \
    --disable-programs --disable-doc

make -j$(nproc)
sudo make install

# OpenCV compilation and installation
cd ~/projects
git clone -b 4.1.0 --single-branch https://github.com/opencv/opencv.git
git clone -b 4.1.0 --single-branch https://github.com/opencv/opencv_contrib.git
cd opencv

mkdir -p build
cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE \
    -D CMAKE_INSTALL_PREFIX=/usr/local \
    -D OPENCV_EXTRA_MODULES_PATH=~/projects/opencv_contrib/modules \
    -D BUILD_opencv_python2=OFF \
    -D BUILD_opencv_python3=OFF \
    -D INSTALL_PYTHON_EXAMPLES=OFF \
    -D INSTALL_C_EXAMPLES=OFF \
    -D OPENCV_ENABLE_NONFREE=OFF \
    -D WITH_FFMPEG=1 \
    -D BUILD_opencv_dnn=ON \
    -D BUILD_EXAMPLES=OFF ..
make -j4
sudo make install

# Darknet compilation and installation
cd ~/projects
git clone https://github.com/AlexeyAB/darknet.git
cd darknet

mkdir -p build
cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE \
    -D CMAKE_INSTALL_PREFIX=/usr/local \
    -D ENABLE_OPENCV=FALSE \
    -D ENABLE_CUDA=TRUE \
    ..
make
sudo make install
sudo cp libdark.* /usr/local/lib/

# Chopstick tracker compilation
cd ~/projects/chopsticks-tracker
rm -rf build
mkdir -p build
cd build
cmake -G "Unix Makefiles" ..
make -j$(nproc)


echo "Chopstick tracker and its dependencies are installed with success!"