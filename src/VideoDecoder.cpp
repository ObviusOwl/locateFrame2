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
    this->frameCount = 0;
    this->format_ctx = avformat_alloc_context();
    this->codec_ctx = NULL;
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


void VideoDecoder::openFile( std::string fileName ){
    int ret;
    AVCodec *dec;
    if( (ret = avformat_open_input(&(this->format_ctx), fileName.c_str(), NULL, NULL)) < 0 ){
        throw VideoDecoderError( "failed to open input video file" );
    }
    if( (ret = avformat_find_stream_info(this->format_ctx, NULL)) < 0 ){
        throw VideoDecoderError( "failed to find stream info" );
    }

    ret = av_find_best_stream(this->format_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
    if( ret < 0) {
        throw VideoDecoderError( "no video stream found" );
    }
    this->codec_ctx = this->format_ctx->streams[ret]->codec;
    this->codec_ctx->refcounted_frames = 1;
    if( this->decoderThreads > 0 ){
        this->codec_ctx->thread_count = this->decoderThreads;
    }
    
    /* init the video decoder */
    if( (ret = avcodec_open2(this->codec_ctx, dec, NULL)) < 0) {
        throw VideoDecoderError( "failed to open the video decoder" );
    }
    this->videoStreamIndex = ret;
}



void VideoDecoder::decodeFrame( VideoFrame& frame ){
    AVPacket packet; // re-use?
    int ret;
    int got_frame = 0;
    AVFrame* avframe = frame.getAvFrame();

    while (1) {
        if( got_frame ){
            // just decode one frame
            break;
        }

        ret = av_read_frame(this->format_ctx, &packet);
        if( ret == AVERROR_EOF ){
            throw VideoDecoderError( "EOF" );
        }
        if( ret < 0 ){
            throw VideoDecoderError( "reading packed failed" );
        }

        if( packet.stream_index == this->videoStreamIndex ){
            got_frame = 0;
            ret = avcodec_decode_video2(this->codec_ctx, avframe, &got_frame, &packet);
            if (ret < 0) {
                throw VideoDecoderError( "error while decoding video" );
            }
            if( got_frame ){
                avframe->pts = av_frame_get_best_effort_timestamp(avframe);
                frame.setIndex( this->frameCount );
                frame.setDimensions( avframe->width, avframe->height );
                frame.setTimestamp( av_q2d( this->format_ctx->streams[this->videoStreamIndex]->time_base )* (avframe->pts) );
                frame.setPixelFormat( this->codec_ctx->pix_fmt );
                (this->frameCount)++;
            }
            
        }
        av_free_packet(&packet);
    }

}
