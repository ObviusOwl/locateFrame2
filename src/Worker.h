#ifndef WORKER_H
#define WORKER_H

#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "VideoFrame.h"
#include "SurfMatcher.h"
#include "WorkerQueue.h"

class Worker{

public:
    Worker();
    //~Worker();
    void work();
    void encode();
    static void doWork( std::shared_ptr<Worker> self );
    static void doEncode( std::shared_ptr<Worker> self );
    void dumpBestMatch();

    void drawKeyPoints( cv::Mat& mat, std::vector<cv::KeyPoint>& keypoints);
    void drawKeyPoints( cv::Mat& mat, std::vector<cv::KeyPoint>& keypoints, 
                std::vector<cv::KeyPoint>& matchedKeypoints );

    void setQueue( std::shared_ptr<WorkerQueue> queue );
    void setID( int id );
    void setImageCount( int num );
    void setMatcher( SurfMatcher matcher );
    void setOutputFile( std::string fileName );
    void setTerminateMatchRatio( double r );
    void enableEncode();
    

private:
    std::shared_ptr<WorkerQueue> queue;
    int ID;
    SurfMatcher matcher;
    bool encodeEnabled;
    std::string outputFile;
    double terminateMatchRatio;
    long totalFramesSeen;
    std::vector<int> imagesFound; // -2 => not found , -1 => queue notified, >= 0 => extra frames
    int imageCount;
};

#endif // WORKER_H