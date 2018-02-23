# LocateFrame2

# Usage

# Build instructions

LocateFrame2 depends on OpenCV 2.4 nonfree module for SURF detection. This module is
not included on ubuntu distributions. OpenCV ist only a build dependency since we link 
the libraries statically.

We also need libAV:

```
sudo apt install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev
```

Download opencv source code from https://opencv.org/releases.html and extract it. 

Pick a install prefix and build & install OpenCV. 

```
cd opencv
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -D CMAKE_INSTALL_PREFIX=/media/files/Software/opencv ..
make -j4
make install
```

Then build LocateFrame2

```
cd locateframe2
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH=/media/files/Software/opencv/share/OpenCV/ ..
make locateFrame2
```

The binary can then be found in the root of the build dir.