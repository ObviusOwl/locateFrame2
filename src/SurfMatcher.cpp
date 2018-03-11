#include <vector>
#include <cstdio>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <opencv2/nonfree/features2d.hpp>

#include "VideoFrame.h"
#include "InputImage.h"
#include "Match.h"
#include "SurfMatcher.h"


SurfMatcher::SurfMatcher(){
    this->hessianThreshold = 500;
    this->keypointMatchRadius = 5.0;
    this->videoWidth = 0;
    this->videoHeight = 0;
    this->scaleImages = false;
}

void SurfMatcher::setVideoDimensions( int width, int height){
    this->videoWidth = width;
    this->videoHeight = height;
}

void SurfMatcher::doScaleImages(){
    this->scaleImages = true;
}


void SurfMatcher::calcKeyPoints( cv::Mat& mat, std::vector<cv::KeyPoint>& keypoints ){
    cv::SurfFeatureDetector surf(this->hessianThreshold);
    surf.detect(mat, keypoints);
}

void SurfMatcher::setHessianThreshold( int thres ){
    this->hessianThreshold = thres;
}
void SurfMatcher::setKeypointMatchRadius( double r){
    this->keypointMatchRadius = r;
}

void SurfMatcher::addImage( InputImage& img ){
    std::vector<cv::KeyPoint> keypoints;
    cv::Mat mat = cv::imread( img.getFileName() );

    // scale image to match video size for better matching
    if( this->videoWidth != 0 && this->videoHeight != 0 && this->scaleImages){
        cv::Mat mat2;
        cv::resize( mat, mat2, cv::Size(this->videoWidth, this->videoHeight),0,0, cv::INTER_CUBIC );
        this->calcKeyPoints( mat2, keypoints );
    }else{
        this->calcKeyPoints( mat, keypoints );
    }

    img.setKeyPoints( keypoints );

    // makes a copy of the image object
    this->images.push_back( img );
}

double SurfMatcher::getKeyPointDistance( cv::KeyPoint& kp1, cv::KeyPoint& kp2 ){
    double d = std::pow( kp1.pt.x - kp2.pt.x ,2) + pow(kp1.pt.y - kp2.pt.y ,2);
    if( d < 0.0 ){
        return -1.0*d;
    }
    return std::sqrt( d );
}

std::vector< std::shared_ptr<Match> >  SurfMatcher::matchKeyPoints( std::vector<cv::KeyPoint>& keypoints ){
    int hits;
    std::vector<cv::KeyPoint> nearest;
    std::shared_ptr<Match> match;
    std::vector< std::shared_ptr<Match> > matches;

    int imageIndex = 0;
    for( auto& img : this->images ){
        // create a match object per image
        match = std::make_shared<Match>();
        matches.push_back( match );

        hits = 0;
        nearest.clear(); // per image
        for( auto& kp : keypoints ){
            // per image per keypoint nearest neighbor search
            cv::KeyPoint neighbor = img.getNearestKeyPoint( kp.pt.x, kp.pt.y );
            nearest.push_back( neighbor );
            // match the nearest to the keypoint, discard nearest not near enough to be a hit
            double dist = SurfMatcher::getKeyPointDistance( kp, neighbor );
            if( dist < this->keypointMatchRadius ){
                ++hits;
                match->addMatchedKeypoint( kp );
            }
        }
        
        match->setImageIndex( imageIndex );
        match->setKeypointMatchCount( hits );
        match->setImageKeypointCount( img.getKeypointCount() );
        imageIndex++;
    }
    return matches;
}

void SurfMatcher::updateBestMatch( std::shared_ptr<Match> match ){
    InputImage& img = this->images.at( match->getImageIndex() );
    Match currentBest = img.getBestMatch();
    if( match->getKeypointMatchCount() > currentBest.getKeypointMatchCount() ){
        img.setBestMatch( *match );
    }
}

void SurfMatcher::updateBestMatches( std::vector< std::shared_ptr<Match> > matches ){
    for( auto& match : matches ){
        this->updateBestMatch( match );
    }
}

void SurfMatcher::updateMatchAverages( std::shared_ptr<Match> match ){
    InputImage& img = this->images.at( match->getImageIndex() );
    long missCount = match->getKeypointCount() - match->getKeypointMatchCount();
    img.updateAverages( match->getKeypointMatchCount(), missCount );
}
long SurfMatcher::getTotalKeypointMiss( std::shared_ptr<Match> match ){
    InputImage& img = this->images.at( match->getImageIndex() );
    return img.getTotalKeypointMiss();
}
long SurfMatcher::getTotalKeypointHit( std::shared_ptr<Match> match ){
    InputImage& img = this->images.at( match->getImageIndex() );
    return img.getTotalKeypointHit();
}

bool SurfMatcher::isImageFound( std::shared_ptr<Match> match ){
    InputImage& img = this->images.at( match->getImageIndex() );
    return img.isFound();
}
bool SurfMatcher::isFullMatch( std::shared_ptr<Match> match ){
    InputImage& img = this->images.at( match->getImageIndex() );
    return img.isFullMatch( match );
}


void SurfMatcher::dumpBestMatch(){
    for( auto& img : this->images ){
        img.dumpBestMatch();
    }
}

