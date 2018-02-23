#include <stdexcept>
#include <opencv2/opencv.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <cstdio>
#include <limits>

#include "InputImage.h"
#include "SurfMatcher.h"
#include "KdTree.h"

InputImage::InputImage(){
    //this->database = KdTree;
    this->totalFramesSeen = 0;
    this->totalKeypointMiss = 0;
    this->totalKeypointHit = 0;
    this->minMatchRatio = 1.0;
    this->minSnr = std::numeric_limits<double>::infinity();
}

/*InputImage::~InputImage(){
}*/

cv::KeyPoint InputImage::getNearestKeyPoint( int x, int y ){
    return this->database.nearestNeighborSearch( x, y );
}


/* Setters */

void InputImage::setDimensions( int width, int height){
    this->width = width;
    this->height = height;
}
void InputImage::setIndex( int index ){
    this->index = index;
}
void InputImage::setFileName( std::string fileName ){
    this->fileName = fileName;
}
void InputImage::setKeypointCount( int num ){
    this->keypointCount = num;
}
void InputImage::setKeyPoints( std::vector<cv::KeyPoint>& keypoints ){
    this->database = KdTree( keypoints );
    this->setKeypointCount( keypoints.size() );
    //this->database.dumpDOT();
}
void InputImage::setBestMatch( Match match ){
    this->bestMatch = match;
}

void InputImage::updateAverages( int hitCount, int missCount ){
    this->totalFramesSeen++;
    this->totalKeypointMiss = this->totalKeypointMiss + missCount;
    this->totalKeypointHit = this->totalKeypointHit + hitCount;
}

void InputImage::setMinMatchRatio( double r ){
    this->minMatchRatio = r;
}
void InputImage::setMinSnr( double r ){
    this->minSnr = r;
}



/* Getters */

int InputImage::getWidth(){
    return this->width;
}
int InputImage::getHeight(){
    return this->height;
}
int InputImage::getIndex(){
    return this->index;
}
std::string InputImage::getFileName(){
    return this->fileName;
}
int InputImage::getKeypointCount(){
    return this->keypointCount;
}

double InputImage::getMinMatchRatio(){
    return this->minMatchRatio;
}
double InputImage::getMinSnr(){
    return this->minSnr;
}

Match InputImage::getBestMatch(){
    return this->bestMatch;
}
double InputImage::getAverageKeypointMiss(){
    return (double) ( this->totalKeypointMiss / this->totalFramesSeen);
}
double InputImage::getAverageKeypointHit(){
    return (double) ( this->totalKeypointHit / this->totalFramesSeen);
}
long InputImage::getTotalKeypointMiss(){
    return this->totalKeypointMiss;
}
long InputImage::getTotalKeypointHit(){
    return this->totalKeypointHit;
}
long InputImage::getTotalFramesSeen(){
    return this->totalFramesSeen;
}

double InputImage::getBestSnr(){
    return this->bestMatch.getSnr();
}
double InputImage::getBestMatchRatio(){
    return (this->bestMatch.getKeypointMatchCount()*1.0)/ this->keypointCount;
}

bool InputImage::isFound(){
    double r = this->getBestMatchRatio();
    double snr = this->getBestSnr();
    if( r >= this->minMatchRatio && snr >= this->minSnr ){
        return true;
    }
    return false;
}
bool InputImage::isFullMatch( std::shared_ptr<Match> match ){
    double r = match->getMatchRatio();
    double snr = match->getSnr();
    if( r >= this->minMatchRatio && snr >= this->minSnr ){
        return true;
    }
    return false;
}


void InputImage::dumpBestMatch(){
    double matchPer = this->getBestMatchRatio()*100;
    double snr = this->getBestSnr();
    double avgSnr = (this->totalKeypointHit*2.0)/( this->totalKeypointMiss + this->keypointCount*this->totalFramesSeen );

    std::printf( "Best match img%d: frame=%ld, ts=%f, hits=%.2f%% (%d/%d ~ %d/%d), snr=%.3f, avg_snr=%.3f, rel_snr=%.3f\n", 
        this->index, this->bestMatch.getFrameIndex(), this->bestMatch.getFrameTimestamp(), matchPer,
        this->bestMatch.getKeypointMatchCount(), this->keypointCount, 
        this->bestMatch.getKeypointMatchCount(), this->bestMatch.getKeypointCount(), 
        snr, avgSnr, snr - avgSnr
    );
}


