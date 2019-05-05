#ifndef VIDEO_DECODER_H
#define VIDEO_DECODER_H

#include <exception>
#include <string>
#include <vector>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/opt.h>
    #include <libswscale/swscale.h>
    #include <libavutil/pixfmt.h>
    #include <libavutil/timestamp.h>
}

#include "VideoFrame.h"

class VideoDecoderError : public std::exception{

public:
    VideoDecoderError(const std::string& message){
        this->message = message;
    }
    virtual const char* what() const throw() {
        return message.c_str();
    }
    virtual ~VideoDecoderError() throw(){}

private:
    std::string message;

};

class VideoDecoder{

public:
    VideoDecoder();
    ~VideoDecoder();

    int getWidth();
    int getHeight();

    void setDecoderThreads( int num );

    void openFile( std::string fileName );
    void decodeFrame( VideoFrame& frame );

private:
    int width;
    int height;
    AVPacket packet;
    bool has_packet;
    AVFormatContext* format_ctx;
    AVCodecContext* codec_ctx;
    AVCodecParameters* codec_par;
    int videoStreamIndex;
    int frameCount;
    int decoderThreads;
};

#endif // VIDEO_DECODER_H
