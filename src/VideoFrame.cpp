#include <opencv2/opencv.hpp>
#include <stdexcept>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/opt.h>
    #include <libswscale/swscale.h>
    #include <libavutil/pixfmt.h>
    #include <libavutil/timestamp.h>
}

#include "VideoFrame.h"

VideoFrame::VideoFrame(){
    this->frame = av_frame_alloc();
    if( this->frame == NULL ){
        throw std::runtime_error("avframe is NULL");
    }
    this->sws_ctx = NULL;
}

VideoFrame::VideoFrame( enum AVPixelFormat pix_fmt, int width, int height ){
    this->setDimensions( width, height );
    this->setPixelFormat( pix_fmt );
    this->sws_ctx = NULL;
    this->frame = av_frame_alloc();
    if( this->frame == NULL ){
        throw std::runtime_error("avframe is NULL");
    }
    this->frame->format = pix_fmt;
    this->frame->width = width;
    this->frame->height = height;
    // allocate buffer for image data, 
    // this also increments the reference counter, so that av_frame_free frees  data too
    if( av_frame_get_buffer( this->frame, 32 ) < 0 ){
        throw std::runtime_error("failed to aquire frame buffer");
    }
}


VideoFrame::~VideoFrame(){
    av_frame_free( &(this->frame) );
    sws_freeContext( this->sws_ctx );
}

cv::Mat VideoFrame::toMat(){
    // better for Frame reuse
    this->sws_ctx = sws_getCachedContext( this->sws_ctx, 
        this->frame->width, this->frame->height, this->pixelFormat, 
        this->frame->width, this->frame->height, AV_PIX_FMT_BGR24,
        SWS_BICUBIC,NULL,NULL,0 );

    if( this->sws_ctx == NULL ){
        throw std::runtime_error("sws_ctx is NULL");
    }

    // get a temprary frame for format conversion
    VideoFrame tempFrame( AV_PIX_FMT_BGR24, this->frame->width, this->frame->height);
    AVFrame* frame2 = tempFrame.getAvFrame();

    // convert to BGR24 for openCV
    sws_scale( this->sws_ctx,  this->frame->data, 
        this->frame->linesize, 0, this->frame->height, 
        frame2->data, frame2->linesize);
    
    // create the Mat from pixel data
    cv::Mat img(this->frame->height, this->frame->width, CV_8UC3, 
        frame2->data[0], frame2->linesize[0]);
    return img.clone();
}

/* Setters */

void VideoFrame::setDimensions( int width, int height){
    this->width = width;
    this->height = height;
}
void VideoFrame::setTimestamp( double timestamp ){
    this->timestamp = timestamp;
}
void VideoFrame::setIndex( long int index ){
    this->index = index;
}

void VideoFrame::setPixelFormat( enum AVPixelFormat pixelFormat ){
    this->pixelFormat = pixelFormat;
}


/* Getters */

AVFrame* VideoFrame::getAvFrame(){
    return this->frame;
}

int VideoFrame::getWidth(){
    return this->width;
}
int VideoFrame::getHeight(){
    return this->height;
}
double VideoFrame::getTimestamp(){
    return this->timestamp;
}
long int VideoFrame::getIndex(){
    return this->index;
}
enum AVPixelFormat VideoFrame::getPixelFormat(){
    return this->pixelFormat;
}

