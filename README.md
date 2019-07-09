# Chopstick Tracker
The goal of this project is to track chopsticks in a video by using
[YOLO v3](https://pjreddie.com/darknet/yolo/) to detect objects.

## Compilation
```bash
# Boost compilation and installation
mkdir -p ~/projects
cd ~/projects
wget https://dl.bintray.com/boostorg/release/1.70.0/source/boost_1_70_0.tar.gz
tar -xzf boost_1_70_0.tar.gz
rm -f boost_1_70_0.tar.gz
cd boost_1_70_0

./bootstrap.sh --prefix=/usr/local/
./b2
./b2 headers
sudo ./b2 install

# FFMpeg compilation and installation
brew install automake fdk-aac git lame libass libtool libvorbis libvpx \
    opus sdl shtool texi2html theora wget x264 x265 xvid nasm pkg-config

cd ~/projects
git clone https://git.ffmpeg.org/ffmpeg.git
cd ffmpeg

./configure \
    --cc=/usr/bin/clang --prefix=/usr/local \
    --enable-gpl --enable-version3 \
    --enable-shared \
    --disable-programs --disable-doc \
    --enable-libx264

make -j4
sudo make install

# OpenCV compilation and installation
brew install cmake pkg-config jpeg libpng libtiff openexr eigen tbb wget

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
    -D ENABLE_CUDA=FALSE \
    ..
make -j4
sudo make install
sudo cp libdark.dylib /usr/local/lib/

# Project compilation
cd ~/projects
cd chopsticks-tracker

mkdir -p build
cd build
cmake -G "Unix Makefiles" ..
make
```

## Execution
```bash
cd ~/projects/chopsticks-tracker/build
./ChopsticksTracker --config-path=../config.ini
```
