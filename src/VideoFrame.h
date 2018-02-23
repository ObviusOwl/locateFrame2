#ifndef VIDEO_FRAME_H
#define VIDEO_FRAME_H

#include <string>
#include <opencv2/opencv.hpp>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/opt.h>
    #include <libswscale/swscale.h>
    #include <libavutil/pixfmt.h>
    #include <libavutil/timestamp.h>
}


class VideoFrame{

public:
    VideoFrame();
    VideoFrame( enum PixelFormat pix_fmt, int width, int height );
    ~VideoFrame();

    AVFrame* getAvFrame();
    int getWidth();
    int getHeight();
    double getTimestamp();
    long int getIndex();
    enum AVPixelFormat getPixelFormat();

    void setDimensions( int width, int height);
    void setTimestamp( double timestamp );
    void setIndex( long int index );
    //void setFrame(AVFrame* frame);
    void setPixelFormat( enum AVPixelFormat pixelFormat );
    
    cv::Mat toMat();
    

private:
    // do not copy VideoFrame
    VideoFrame(const VideoFrame& that);
    VideoFrame& operator=(const VideoFrame& that);

    AVFrame* frame;
    struct SwsContext* sws_ctx;
    int width;
    int height;
    long int index; // frame number
    double timestamp;
    enum AVPixelFormat pixelFormat;
};

#endif // VIDEO_FRAME_H