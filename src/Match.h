#ifndef MATCH_H
#define MATCH_H

#include <string>
#include <opencv2/opencv.hpp>

class Match{

public:
    Match();
    void setOutputMat( cv::Mat mat );
    void setFrameTimestamp( double ts );
    void setFrameIndex( long int idx );
    void setKeypointCount( int nkp);
    void setImageKeypointCount( int nkp);
    void setKeypointMatchCount( int nkpm);
    void setImageIndex( int idx );
    void addMatchedKeypoint( cv::KeyPoint kp );

    cv::Mat getOutputMat();
    double getFrameTimestamp();
    long int getFrameIndex();
    int getKeypointCount();
    int getImageKeypointCount();
    int getKeypointMatchCount();
    int getImageIndex();
    std::vector< cv::KeyPoint > getMatchedKeypoints();

    double getSnr();
    double getMatchRatio();
    
    void dumpStatus( long totalFramesSeen, long totalKeypointHit, long totalKeypointMiss );

private:
    cv::Mat outputMat;
    double frameTimestamp;
    long int frameIndex;
    int keypointCount;
    int keypointMatchCount;
    int imageIndex;
    int imageKeypointCount;
    std::vector< cv::KeyPoint > matchedKeypoints;
};

#endif // MATCH_H