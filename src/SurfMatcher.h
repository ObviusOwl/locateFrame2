#ifndef SURF_MATCHER_H
#define SURF_MATCHER_H

#include <vector>
#include <tuple>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>

#include "VideoFrame.h"
#include "InputImage.h"
#include "Match.h"

class SurfMatcher{

public:
    SurfMatcher();
    //~SurfMatcher();

    void setHessianThreshold( int thres );
    void setKeypointMatchRadius( double r);
    void setVideoDimensions( int width, int height);
    void doScaleImages();

    void addImage( InputImage& img );

    void calcKeyPoints( cv::Mat& mat, std::vector<cv::KeyPoint>& keypoints );
    std::vector< std::shared_ptr<Match> > matchKeyPoints( std::vector<cv::KeyPoint>& keypoints );
    void updateBestMatches( std::vector< std::shared_ptr<Match> > matches );
    void updateBestMatch( std::shared_ptr<Match> match );
    void dumpBestMatch();

    static double getKeyPointDistance( cv::KeyPoint& kp1, cv::KeyPoint& kp2 );
    int getBestTranslation( std::vector<cv::KeyPoint>& keypoints, 
            std::vector<cv::KeyPoint>& nearest, int votes_init, std::vector<int>& best_trans);

    void updateMatchAverages( std::shared_ptr<Match> match );
    long getTotalKeypointMiss( std::shared_ptr<Match> match );
    long getTotalKeypointHit( std::shared_ptr<Match> match );

    bool isImageFound( std::shared_ptr<Match> match );
    bool isFullMatch( std::shared_ptr<Match> match );


private:
    int hessianThreshold;
    double keypointMatchRadius;
    std::vector< InputImage > images;
    int videoWidth;
    int videoHeight;
    bool scaleImages;
};

#endif // SURF_MATCHER_H