# Chopstick Tracker

## Summary
1. [Introduction](#introduction)
2. [Compilation](#compilation)
3. [Usage](#usage)
4. DNN model training
5. Running this application in the cloud
6. External links

## Introduction
The goal of this project is to help me to learn about how to use a
[deep neural network](https://en.wikipedia.org/wiki/Deep_learning) to detect and track objects in
a video. For that I have chosen the [YOLO v3](https://pjreddie.com/darknet/yolo/) model, because it has
good performance, can work in real-time and is easy to use.

This application is able to detect and track [chopsticks](https://en.wikipedia.org/wiki/Chopsticks).
The idea is to combine it later with a [robotic arm](https://en.wikipedia.org/wiki/Robotic_arm) that
will automatically recognize where chopsticks are located, and then move them to a specific
position according a user-defined pattern (for example a grid). The robotic arm would also be able
to detect when somebody "breaks" the pattern, so it can fix it automatically.

You can see the resulting video by clicking on the following image:
[![Watch video](images/example.jpg)](https://youtu.be/d3EM2Zqqtio)

## Compilation
The following instructions describes how to compile this project and its dependencies on MacOS Mojave
and Ubuntu Linux 18.04 64-bit.

Before stating, make sure your system respects the following prerequisites:
* If you use MacOS, make sure you have installed the
  [Xcode Command Line Tools](https://apple.stackexchange.com/questions/337744/installing-xcode-command-line-tools/33901),
  [Homebrew](https://brew.sh/) and [cmake](https://formulae.brew.sh/formula/cmake).
* If you use Ubuntu, please make sure you have installed the
  [binutils](https://packages.ubuntu.com/bionic/binutils),
  [git](https://packages.ubuntu.com/bionic/git) and
  [cmake](https://packages.ubuntu.com/bionic/cmake) packages.
* If your machine has a CUDA-compatible GPU, make sure you have installed the
  [CUDA Toolkit 10.1](https://developer.nvidia.com/cuda-toolkit).
* In order to checkout this project with git, you need to install
  [git-lfs](https://github.com/git-lfs/git-lfs/wiki/Installation). This allows you to download
  big files such as [data/input-video/VID_20181231_133114.mp4](data/input-video/VID_20181231_133114.mp4)
  or [data/yolo-model/yolov3.weights](data/yolo-model/yolov3.weights).

The first step is to install the dependencies for this project. Open a terminal to your machine and
execute the following commands:
```bash
# Run the following command if you use MacOS
NB_PROCESSORS=$(sysctl -n hw.ncpu)

# Run the following command if you use Ubuntu Linux
NB_PROCESSORS=$(nproc)

#
# Boost compilation and installation
#
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

#
# RapidJSON compilation and installation
#
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

#
# FFMpeg compilation and installation
#

# Run the following command if you use MacOS
brew install automake fdk-aac git lame libass libtool libvorbis libvpx \
    opus sdl shtool texi2html theora wget x264 x265 xvid nasm pkg-config

# Run the following command if you use Ubuntu Linux
sudo apt-get -y install nasm pkg-config

cd ~/projects
git clone -b release/4.1 --single-branch https://git.ffmpeg.org/ffmpeg.git
cd ffmpeg

./configure \
    --prefix=/usr/local \
    --enable-gpl --enable-version3 \
    --enable-shared \
    --disable-programs --disable-doc # add "--cc=/usr/bin/clang" if you are using MacOS

make -j${NB_PROCESSORS}
sudo make install

#
# OpenCV compilation and installation
#

# Run the following command if you use MacOS
brew install cmake jpeg libpng libtiff openexr eigen tbb wget

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
make -j4 # Warning: "make -j${NB_PROCESSORS}" may take too much memory on your system
sudo make install

# Darknet compilation and installation
cd ~/projects
git clone https://github.com/AlexeyAB/darknet.git
cd darknet

mkdir -p build
cd build
# Note: change to "-D ENABLE_CUDA=TRUE" if your machine supports CUDA
cmake -D CMAKE_BUILD_TYPE=RELEASE \
    -D CMAKE_INSTALL_PREFIX=/usr/local \
    -D ENABLE_OPENCV=FALSE \
    -D ENABLE_CUDA=FALSE \
    ..
make
sudo make install
sudo cp libdark.* /usr/local/lib/
```

We can now compile this project. Please run the following commands in the same terminal:
```bash
cd ~/projects
git clone https://github.com/marcplouhinec/chopsticks-tracker.git # Make sure git-lfs is installed beforehand!
cd chopsticks-tracker

mkdir -p build
cd build
cmake -G "Unix Makefiles" ..
make -j${NB_PROCESSORS}
```

## Usage
The application takes two parameters:
* The path to the [configuration file](config.ini).
* The path to the [video file](data/input-video/VID_20181231_133114.mp4).

If you open the [configuration file](config.ini), you can see that most of the parameters are
self-descriptive or documented.
The following parameters are the most important:
* Under the `[objectDetection]` section, `implementation` can take two values: `opencvdnn` or `darknet`.
  `darknet` is much faster if you have compiled Darknet with CUDA support and your machine has a strong GPU.
  However, the `opencvdnn` implementation is faster if you can't use CUDA.
* Under the `[rendering]` section, the parameters starting from `detectedObjectsPainter_show` and
  `trackedObjectsPainter_show` allow us to show or hide detected or tracked objects in the output images.
* Under the `[rendering]` section, `writerImplementation` can take two values: `mjpeg` or `multijpeg`.
  If `mjpeg` is set, then a video file (with the .avi extension) is generated in the output folder
  (defined by `outputpath`). If `multijpeg` is set, then each frame from the input video is rendered
  into a seperate JPEG file, which makes debugging easier.

> Note: all the file paths in the [configuration file](config.ini) must be relative to this configuration file.

In order to run this project, open a terminal to your machine and run the following commands:
```bash
export LD_LIBRARY_PATH=/usr/local/lib

cd ~/projects/chopsticks-tracker/build

./ChopsticksTracker \
    --config-path=../config.ini \
    --video-path=../data/input-video/VID_20181231_133114.mp4
```

The result is generated in the folder defined by the `outputpath` configuration parameter. With the default
configuration, the generated file is located at `~/projects/chopsticks-tracker/output/result/VID_20181231_133114.avi`.
