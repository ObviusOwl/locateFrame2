#ifndef SURF_MATCHER_H
#define SURF_MATCHER_H

#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/nonfree/features2d.hpp>

#include "VideoFrame.h"
#include "InputImage.h"
#include "Match.h"

class SurfMatcher{

public:
    SurfMatcher();
    //~SurfMatcher();

    void setHessianThreshold( int thres );
    void setKeypointMatchRadius( double r);

    void addImage( InputImage& img );

    void calcKeyPoints( cv::Mat& mat, std::vector<cv::KeyPoint>& keypoints );
    std::vector< std::shared_ptr<Match> > matchKeyPoints( std::vector<cv::KeyPoint>& keypoints );
    void updateBestMatches( std::vector< std::shared_ptr<Match> > matches );
    void updateBestMatch( std::shared_ptr<Match> match );
    void dumpBestMatch();

    static double getKeyPointDistance( cv::KeyPoint& kp1, cv::KeyPoint& kp2 );

    void updateMatchAverages( std::shared_ptr<Match> match );
    long getTotalKeypointMiss( std::shared_ptr<Match> match );
    long getTotalKeypointHit( std::shared_ptr<Match> match );

    bool isImageFound( std::shared_ptr<Match> match );
    bool isFullMatch( std::shared_ptr<Match> match );


private:
    int hessianThreshold;
    double keypointMatchRadius;
    std::vector< InputImage > images;
};

#endif // SURF_MATCHER_H