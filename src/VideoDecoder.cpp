#include <cstdio>
#include <memory>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/opt.h>
    #include <libswscale/swscale.h>
    #include <libavutil/pixfmt.h>
    #include <libavutil/timestamp.h>
}

#include "VideoDecoder.h"
#include "VideoFrame.h"

VideoDecoder::VideoDecoder(){
    av_register_all();
    this->has_packet = false;
    this->frameCount = 0;
    this->format_ctx = avformat_alloc_context();
    this->codec_ctx = NULL;
    this->codec_par = NULL;
    this->videoStreamIndex = -1;
    this->decoderThreads = -1;
}

VideoDecoder::~VideoDecoder(){
    avcodec_close( this->codec_ctx );
    avformat_close_input( &(this->format_ctx) );
    avformat_free_context( this->format_ctx );
}

void VideoDecoder::setDecoderThreads( int num ){
    this->decoderThreads = num;
    if( this->codec_ctx != NULL && this->decoderThreads > 0 ){
        this->codec_ctx->thread_count = this->decoderThreads;
    }
}


int VideoDecoder::getWidth(){
    return this->width;
}
int VideoDecoder::getHeight(){
    return this->height;
}
double VideoDecoder::getFrameRate(){
    if( this->videoStreamIndex < 0 ){
        throw VideoDecoderError( "video file not opened" );
    }
    AVRational rate = this->format_ctx->streams[this->videoStreamIndex]->avg_frame_rate;
    if( rate.den == 0 ){
        throw VideoDecoderError( "avg_frame_rate not available" );
    }
    return rate.num*1.0 / rate.den;
}

void VideoDecoder::openFile( std::string fileName ){
    int ret;
    AVCodec *codec;
    if( (ret = avformat_open_input(&(this->format_ctx), fileName.c_str(), NULL, NULL)) < 0 ){
        throw VideoDecoderError( "failed to open input video file" );
    }
    if( (ret = avformat_find_stream_info(this->format_ctx, NULL)) < 0 ){
        throw VideoDecoderError( "failed to find stream info" );
    }

    ret = av_find_best_stream(this->format_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
    if( ret < 0) {
        throw VideoDecoderError( "no video stream found" );
    }
    this->codec_par = this->format_ctx->streams[ret]->codecpar;

    this->codec_ctx = avcodec_alloc_context3(codec);
    if( !this->codec_ctx ){
        throw VideoDecoderError( "failed to allocate AVCodecContext" );
    }
    ret = avcodec_parameters_to_context( this->codec_ctx, this->codec_par );
    if( ret < 0 ){
        throw VideoDecoderError( "failed to init AVCodecContext" );
    }
    
    if( this->decoderThreads > 0 ){
        this->codec_ctx->thread_count = this->decoderThreads;
    }

    this->width = this->codec_par->width;
    this->height = this->codec_par->height;
    
    /* init the video decoder */
    if( (ret = avcodec_open2(this->codec_ctx, codec, NULL)) < 0) {
        throw VideoDecoderError( "failed to open the video decoder" );
    }
    this->videoStreamIndex = ret;
}



void VideoDecoder::decodeFrame( VideoFrame& frame ){
    int ret;
    int got_frame = 0;
    AVFrame* avframe = frame.getAvFrame();

    while (1) {
        if( got_frame ){
            // just decode one frame
            break;
        }
        
        if( ! this->has_packet ){
            // only read a new packet if the last packet has been processed
            ret = av_read_frame(this->format_ctx, &(this->packet));
            if( ret == AVERROR_EOF ){
                throw VideoDecoderError( "EOF" );
            }
            if( ret < 0 ){
                throw VideoDecoderError( "reading packed failed" );
            }
            this->has_packet = true;
        }

        if( this->packet.stream_index == this->videoStreamIndex ){
            ret = avcodec_send_packet(this->codec_ctx, &(this->packet) );
            if(ret == AVERROR(EAGAIN) ){
                // pass, receive frame and retry send on the next iteration
            }else if(ret == AVERROR_EOF){
                throw VideoDecoderError( "EOF" );
            }else{
                // packet successfully send or decoding error
                av_packet_unref(&(this->packet));
                this->has_packet = false; 
            }

            ret = avcodec_receive_frame(this->codec_ctx, avframe);
            if(ret == AVERROR(EAGAIN) ){
                continue;
            }else if(ret == AVERROR_EOF){
                throw VideoDecoderError( "EOF" );
            }else if (ret < 0) {
                throw VideoDecoderError( "error during decoding" );
            }else{
                got_frame = 1;
                avframe->pts = av_frame_get_best_effort_timestamp(avframe);
                frame.setIndex( this->frameCount );
                frame.setDimensions( avframe->width, avframe->height );
                frame.setTimestamp( av_q2d( this->format_ctx->streams[this->videoStreamIndex]->time_base )* (avframe->pts) );
                frame.setPixelFormat( this->codec_ctx->pix_fmt );
                (this->frameCount)++;
            }
        }else{
            // discard packets from other streams
            av_packet_unref(&(this->packet));
            this->has_packet = false; 
        }
    }

}
