#ifndef INPUT_IMAGE_H
#define INPUT_IMAGE_H

#include <string>
#include <opencv2/opencv.hpp>

//#include "SurfMatcher.h"
#include "KdTree.h"
#include "Match.h"

class InputImage{

public:
    InputImage();
    //~InputImage();

    cv::KeyPoint getNearestKeyPoint( int x, int y );

    int getWidth();
    int getHeight();
    int getIndex();
    std::string getFileName();
    int getKeypointCount();

    double getAverageKeypointMiss();
    double getAverageKeypointHit();
    long getTotalKeypointMiss();
    long getTotalKeypointHit();
    long getTotalFramesSeen();
    Match getBestMatch();
    double getMinMatchRatio();
    double getMinSnr();

    void setDimensions( int width, int height);
    void setIndex( int index );
    void setFileName( std::string fileName );
    void setKeypointCount( int num );
    void setKeyPoints( std::vector<cv::KeyPoint>& keypoints );
    void setMinMatchRatio( double r );
    void setMinSnr( double r );

    void setBestMatch( Match match );
    void updateAverages( int hitCount, int missCount );

    double getBestSnr();
    double getBestMatchRatio();
    bool isFound();
    bool isFullMatch( std::shared_ptr<Match> match );

    void dumpBestMatch();

private:
    int width;
    int height;
    int index;
    std::string fileName;
    int keypointCount;
    KdTree database;
    Match bestMatch;
    long totalFramesSeen;
    long totalKeypointMiss;
    long totalKeypointHit;
    
    double minMatchRatio;
    double minSnr;
};

#endif // INPUT_IMAGE_H