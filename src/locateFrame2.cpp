#include <iostream>
#include <cstdio>
#include <thread>
#include <memory>
#include <list>

#include "Arguments.h"
#include "VideoDecoder.h"
#include "VideoFrame.h"
#include "SurfMatcher.h"
#include "WorkerQueue.h"
#include "Worker.h"


int main(int argc, char **argv) {
    /// parse arguments
    Arguments args;
    args.parseArgs( argc, argv );
    args.printArguments();
    // create and configure the decoder
    VideoDecoder dec;
    dec.setDecoderThreads( args.getDecoderThreads() );
    dec.openFile( args.getInputFile() );

    // create and configure the master matcher (the threads will get a copy)
    SurfMatcher matcher;
    matcher.setVideoDimensions( dec.getWidth(), dec.getHeight() );
    if( args.doScale() ){
        matcher.doScaleImages();
    }
    matcher.setHessianThreshold( args.getHessianThreshold() );
    matcher.setKeypointMatchRadius( args.getKeypointMatchRadius() );
    // configure the input images (again, each thread will get a copy of all images)
    std::vector<double> minMatchRatios = args.getMatchRatios();
    std::vector<double> minSnrs = args.getSnrRatios();
    int imageIndex = 0;
    for ( auto &fileName : args.getSearchFiles() ) {
        InputImage img;
        img.setFileName( fileName );
        img.setIndex( imageIndex );
        if( minMatchRatios.size() > imageIndex ){
            // configure minimum match ratios. if the list is too short the last images will keep their default
            img.setMinMatchRatio( minMatchRatios.at(imageIndex) );
        }
        if( minSnrs.size() > imageIndex ){
            // configure minimum SNR.
            img.setMinSnr( minSnrs.at(imageIndex) );
        }
        matcher.addImage( img );
        imageIndex++;
    }
    int imageCount = imageIndex; // imageCount is used further down

    // create and configure the queue used by the workers to communicate
    std::shared_ptr<WorkerQueue> queue = std::make_shared<WorkerQueue>();
    queue->setImageCount( imageCount );
    queue->setMaxLength( args.getQueueSize() );
    
    // create the workers and their threads
    std::list< std::shared_ptr<Worker> > workers;
    int i;
    for( i=0; i< args.getMatcherThreads(); i++){
        std::shared_ptr<MatchWorker> worker = std::make_shared<MatchWorker>();
        worker->setQueue( queue );
        worker->setID( i );
        worker->setMatcher( matcher );
        worker->start(); // start thread
        workers.push_back( worker );
    }
    // create the encode worker responsible for finding the maximum match and encoding the output video
    // this is done sequencially, so all frames are in the correct order again
    std::shared_ptr<EncodeWorker> encodeWorker = std::make_shared<EncodeWorker>();
    encodeWorker->setQueue( queue );
    encodeWorker->setID( i++ );
    encodeWorker->setImageCount( imageCount );
    // the InputImages of the matcher of the encode worker 
    // will be the only ones storing the current best match
    encodeWorker->setMatcher( matcher );
    workers.push_back( encodeWorker );

    if( args.getOutputFile() != "" ){
        // enable encoding only if requested
        encodeWorker->enableEncode();
        encodeWorker->setOutputFile( args.getOutputFile() );
    }
    encodeWorker->start(); // start thread

    int minFrame = args.getMinFrame();
    int maxFrame = args.getMaxFrame();
    while( 1 ){
        // main loop decodign the frames
        std::shared_ptr<VideoFrame> frame = std::make_shared<VideoFrame>();
        dec.decodeFrame( *frame );
        if( frame->getIndex() >= minFrame ){
            // skip the first decoded frames until minFrame is reached
            queue->enqueue( frame );
        }
        if( frame->getIndex() >= maxFrame ){
            // signal the worker threads to terminate, we are done!
            queue->terminate();
            break;
        }
    }
    
    for ( auto &worker : workers ) {
        // wait for all threads to terminate, especially the wencoder thread
        worker->join();
    }
    // output the finalt sumary with all best matches
    encodeWorker->dumpBestMatch();
}