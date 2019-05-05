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
    virtual void work() = 0;

    void setID( int id );
    void setQueue( std::shared_ptr<WorkerQueue> queue );

    std::shared_ptr<std::thread> start();
    void join();
    std::shared_ptr<std::thread> getThread();
    static void doWork( Worker* self );

protected:
    int ID;
    std::shared_ptr<WorkerQueue> queue;
    std::shared_ptr<std::thread> thread;
};


class MatchWorker : public Worker{

public:
    MatchWorker();
    void work();

    void setMatcher( SurfMatcher matcher );

    void drawKeyPoints( cv::Mat& mat, std::vector<cv::KeyPoint>& keypoints);
    void drawKeyPoints( cv::Mat& mat, std::vector<cv::KeyPoint>& keypoints, 
                std::vector<cv::KeyPoint>& matchedKeypoints );


private:
    long totalFramesSeen;
    SurfMatcher matcher;
};

class EncodeWorker : public Worker{

public:
    EncodeWorker();
    void work();

    void setImageCount( int num );
    void setOutputFile( std::string fileName );
    void enableEncode();
    void setFrameRate( double rate );
    
    void setMatcher( SurfMatcher matcher );
    void dumpBestMatch();

private:
    long totalFramesSeen;
    SurfMatcher matcher;

    bool encodeEnabled;
    std::string outputFile;
    double frameRate;

    std::vector<int> imagesFound; // -2 => not found , -1 => queue notified, >= 0 => extra frames
    int imageCount;
};

#endif // WORKER_H