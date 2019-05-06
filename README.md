# LocateFrame2

# Usage

Run time library dependencies:

```sh
sudo apt-get install --no-install-recommends --no-install-suggests libopencv-core3.2 libopencv-features2d3.2
```

Note the `--no-install-recommends` to avoid installing a complete desktop in containers. Also libav gets installed as a dependency of opencv.


# Build Instructions

LocateFrame2 depends on OpenCV 3.2.0 and libav (ffmpeg). Install the build dependencies:

```sh
sudo apt install build-essential cmake pkg-config
sudo apt install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev
sudo apt install libopencv-dev
```

Then build LocateFrame2

```sh
mkdir build
cd build
cmake ..
make locateFrame2
```

The binary can then be found in the root of the build dir.