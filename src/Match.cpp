#include <cstdio>
#include <cmath>
#include <opencv2/opencv.hpp>

#include "Match.h"


Match::Match(){
    this->frameTimestamp = 0.0;
    this->frameIndex = 0;
    this->keypointCount = 0;
    this->imageKeypointCount = 0;
    this->keypointMatchCount = 0;
    this->imageIndex = -1;
}


/* Setters */

void Match::setOutputMat( cv::Mat mat ){
    this->outputMat = mat;
}
void Match::setFrameTimestamp( double ts ){
    this->frameTimestamp = ts;
}
void Match::setFrameIndex( long int idx ){
    this->frameIndex = idx;
}
void Match::setKeypointCount( int nkp){
    this->keypointCount = nkp;
}
void Match::setImageKeypointCount( int nkp){
    this->imageKeypointCount = nkp;
}
void Match::setKeypointMatchCount( int nkpm){
    this->keypointMatchCount = nkpm;
}
void Match::setImageIndex( int idx ){
    this->imageIndex = idx;
}
void Match::addMatchedKeypoint( cv::KeyPoint kp ){
    this->matchedKeypoints.push_back( kp );
}


/* Getters */

cv::Mat Match::getOutputMat(){
    return this->outputMat;
}
double Match::getFrameTimestamp(){
    return this->frameTimestamp;
}
long int Match::getFrameIndex(){
    return this->frameIndex;
}
int Match::getKeypointCount(){
    return this->keypointCount;
}
int Match::getImageKeypointCount(){
    return this->imageKeypointCount;
}
int Match::getKeypointMatchCount(){
    return this->keypointMatchCount;
}
int Match::getImageIndex(){
    return this->imageIndex;
}
std::vector< cv::KeyPoint > Match::getMatchedKeypoints(){
    return this->matchedKeypoints;
}

double Match::getSnr(){
    double commonMiss = this->imageKeypointCount + this->keypointCount - (2*this->keypointMatchCount);
    return (this->keypointMatchCount*2.0)/( commonMiss );
}
double Match::getMatchRatio(){
    return (this->keypointMatchCount*1.0)/ this->imageKeypointCount;
}


void Match::dumpStatus( long totalFramesSeen, long totalKeypointHit, long totalKeypointMiss ){
    double commonMiss = this->imageKeypointCount + this->keypointCount - (2*this->keypointMatchCount);
    double snr = (this->keypointMatchCount*2.0)/( commonMiss );
    double avgSnr = (totalKeypointHit*2.0)/( totalKeypointMiss + this->imageKeypointCount*totalFramesSeen );
    std::fprintf( stderr,"arg %d, frame %ld: keypts %d, "
            "match %3.3f%%, SNR %3.3f, avg SNR %3.3f, rel SNR %3.3f\n",
            this->imageIndex, this->frameIndex, this->keypointCount, 
            ( (this->keypointMatchCount*1.0)/ this->imageKeypointCount )*100,
            /*10*std::log10(snr),
            10*std::log10(avgSnr),*/
            snr,
            avgSnr,
            snr-avgSnr 
    );

}

