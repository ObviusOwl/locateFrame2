#include <vector>
#include <thread>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <cstdio>
#include <opencv2/opencv.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <math.h>

#include "Worker.h"
#include "VideoFrame.h"
#include "Match.h"
#include "SurfMatcher.h"

Worker::Worker(){
    this->encodeEnabled = false;
    this->outputFile = "";
    this->imageCount = 0;
    this->totalFramesSeen = 0;
}

void Worker::doWork( std::shared_ptr<Worker> self ){
    self->work();
}
void Worker::doEncode( std::shared_ptr<Worker> self ){
    self->encode();
}

void Worker::setQueue( std::shared_ptr<WorkerQueue> queue ){
    this->queue = queue;
}
void Worker::setID( int id ){
    this->ID = id;
}
void Worker::setImageCount( int num ){
    this->imageCount = num;
}

void Worker::setMatcher( SurfMatcher matcher ){
    this->matcher = matcher;
}

void Worker::setOutputFile( std::string fileName ){
    this->outputFile = fileName;
}
void Worker::enableEncode(){
    this->encodeEnabled = true;
}
void Worker::setTerminateMatchRatio( double r ){
    this->terminateMatchRatio = r;
}


void Worker::drawKeyPoints( cv::Mat& mat, std::vector<cv::KeyPoint>& keypoints){
    for( auto& kp : keypoints ){
        cv::Scalar color( 0, 0, 255 );
        cv::circle( mat, kp.pt, std::sqrt(kp.size), color );
    }
}

void Worker::drawKeyPoints( cv::Mat& mat, std::vector<cv::KeyPoint>& keypoints, 
            std::vector<cv::KeyPoint>& matchedKeypoints ){
    cv::Scalar color;
    for( auto& kp : keypoints ){
        color = cv::Scalar( 0, 0, 255);
        for( auto& mkp : matchedKeypoints ){
            if( (int) kp.pt.x == (int) mkp.pt.x && (int) kp.pt.y == (int) mkp.pt.y ){
                color = cv::Scalar( 0, 255, 0);
                break;
            }
        }
        cv::circle( mat, kp.pt, std::sqrt(kp.size), color );
    }
}

void Worker::dumpBestMatch(){
    this->matcher.dumpBestMatch();
}

void Worker::encode(){
    bool videoOpen = false;
    cv::VideoWriter writer;
    for( int i=0; i<this->imageCount; i++){
        // set all images as not found
        this->imagesFound.push_back(-2);
    }

    while( 1 ){
        std::shared_ptr<Match> match = this->queue->dequeueMatch();
        if( match == nullptr ){
            // we want to quit
            break;
        }
        int imageIndex = match->getImageIndex();

        if( imageIndex == 0 && this->encodeEnabled ){
            // write output video for first image (keypoint matches plotted)
            if( ! videoOpen ){
                writer.open( this->outputFile, CV_FOURCC('M','P','E','G'), 100, match->getOutputMat().size(), true );
                videoOpen = true;
            }
            writer.write( match->getOutputMat() );
        }
        // update the images of the matcher of _this_ thread to current best match 
        this->matcher.updateBestMatch( match );
        this->matcher.updateMatchAverages( match );

        // stats line 
        long totalKeypointHit = this->matcher.getTotalKeypointHit( match );
        long totalKeypointMiss = this->matcher.getTotalKeypointMiss( match );
        match->dumpStatus( this->totalFramesSeen, totalKeypointHit, totalKeypointMiss );

        if( imageIndex == 0){
            this->totalFramesSeen++;
        }

        // notify the queue the image has been found and we are ready to terminate
        // videos have streaks of similar images. Once we found a full match 
        // we check the next frames if there may be a even better match.
        int extraFrames = this->imagesFound.at( imageIndex );
        if( this->matcher.isFullMatch(match) ){
            if( extraFrames == -1 ){
                // do nothing, since the queue has been notified
            }else if( extraFrames == -2 ){
                this->imagesFound[ imageIndex ] = 1;
            }else if(extraFrames >= 0 ){
                // for each frame meeting the full match criteria, add a extra frame to check
                // 0 is the edge case: the next not fully matched frame would have notified the queue
                this->imagesFound[ imageIndex ] = extraFrames + 1; // TODO make steps configurable
            }
        }else{
            if(extraFrames > 0 ){
                // for each frame which is not a full match count down 
                // untill we are sure there will be no candidate for a event better match
                this->imagesFound[ imageIndex ] = extraFrames - 1; // TODO make steps configurable
            }else if(extraFrames == 0 ){
                // notify the queue that this image has been found
                this->imagesFound[ imageIndex ] = -1;
                this->queue->imageFound();
            }
        }
    }
}

void Worker::work(){
    while( 1 ){
        std::shared_ptr<VideoFrame> frame = this->queue->dequeue();
        if( frame == nullptr ){
            // we want to quit
            break;
        }
        std::vector<cv::KeyPoint> keypoints;
        std::vector< std::shared_ptr<Match> > matches;
        // get a openCV mat for keypoint calc and keypoint plot
        cv::Mat mat = frame->toMat();
        // detect keypoints of the frame
        this->matcher.calcKeyPoints( mat, keypoints );
        // match keypoints with all images by our copy of the matcher
        matches = this->matcher.matchKeyPoints( keypoints );

        // plot matches
        if( matches.size() > 0 ){
            std::vector<cv::KeyPoint> matchedKeypoints = matches[0]->getMatchedKeypoints();
            this->drawKeyPoints( mat, keypoints, matchedKeypoints);
        }else{
            this->drawKeyPoints( mat, keypoints );
        }

        for( auto& match : matches ){
            // set match infos and enqueue match for checking and video write
            match->setOutputMat( mat );
            match->setFrameTimestamp( frame->getTimestamp() );
            match->setFrameIndex( frame->getIndex() );
            match->setKeypointCount( keypoints.size() );
            this->queue->enqueueMatch( match );
        }
    }
}
