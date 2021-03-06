#ifndef WORKER_QUEUE_H
#define WORKER_QUEUE_H

#include <vector>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <memory>
#include <queue>
#include <opencv2/opencv.hpp>

#include "Match.h"
#include "VideoFrame.h"

class MatchComparator{
public:
    bool operator() (std::shared_ptr<Match> m1, std::shared_ptr<Match> m2);
};

class VideoFrameComparator{
public:
    bool operator() (std::shared_ptr<VideoFrame> f1, std::shared_ptr<VideoFrame> f2);
};

class WorkerQueue{

public:
    WorkerQueue();
    void terminate();
    bool getTerminate();
    void setMaxLength( size_t len );
    void setImageCount( int num );

    void imageFound();

    std::shared_ptr<VideoFrame> dequeue();
    void enqueue( std::shared_ptr<VideoFrame> frame);
    
    std::shared_ptr<Match> dequeueMatch();
    void enqueueMatch( std::shared_ptr<Match> match);
 
private:
    bool doTerminate;
    std::shared_mutex doTerminateMutex;

    size_t maxLength;

    // prevent starvation with priority queue
    std::priority_queue< std::shared_ptr<VideoFrame>, std::vector<std::shared_ptr<VideoFrame>>, VideoFrameComparator > items;
    std::mutex mutex;
    std::condition_variable condEnq;
    std::condition_variable condDeq;

    long int matchDequeueIndex; // used to track last dequeued frame index
    int matchDequeueImageCount; // used to track if all matches from matchDequeueIndex have been dequeued
    // we use a priority queue sorted first by frame number then by image index
    std::priority_queue< std::shared_ptr<Match>, std::vector<std::shared_ptr<Match>>, MatchComparator > matchItems;
    std::mutex matchMutex;
    std::condition_variable matchCondEnq;
    std::condition_variable matchCondDeq;
    
    int imageCount;
    int imagesFound;

};

#endif // WORKER_QUEUE_H
