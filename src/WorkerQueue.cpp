#include <list>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <queue>
#include <opencv2/opencv.hpp>
#include <opencv2/nonfree/features2d.hpp>

#include "WorkerQueue.h"
#include "VideoFrame.h"
#include "Match.h"

bool MatchComparator::operator() (std::shared_ptr<Match> m1, std::shared_ptr<Match> m2) {
    bool ret = false;
    long int frame1 = m1->getFrameIndex();
    long int frame2 = m2->getFrameIndex();
    if( frame1 == frame2 ){
        if( m1->getImageIndex() > m2->getImageIndex() ){
            ret = true;
        }
    }else if(frame1 > frame2) {
        ret = true;
    }
    return !ret; // the priority queue sorts the largest first, we want the smallest
}


WorkerQueue::WorkerQueue(){
    this->maxLength = 5;
    this->imageCount = 0;
    this->imagesFound = 0;
    this->doTerminate = false;
    this->matchEnqueueIndex = 0;
    this->matchDequeueIndex = -1;
}

void WorkerQueue::setMaxLength( size_t len ){
    std::unique_lock<std::mutex> mlock( this->maxLengthMutex );
    this->maxLength = len;
}

void WorkerQueue::setImageCount( int num ){
    this->imageCount = num;
}

void WorkerQueue::terminate(){
    // exclusive access
    std::unique_lock<std::mutex> mlock( this->doTerminateMutex );
    this->doTerminate = true;
    // release the lock to prevent deadlock
    mlock.unlock();
    // notify all consumer blocking on dequeue()
    this->condDeq.notify_all();
    this->matchCondDeq.notify_all();
}
void WorkerQueue::imageFound(){
    // exclusive access
    std::unique_lock<std::mutex> mlock( this->doTerminateMutex );
    // terminate if all images have been found
    this->imagesFound++;
    if( this->imagesFound == this->imageCount ){
        this->doTerminate = true;
    }
    mlock.unlock();
    // notify all consumer blocking on dequeue()
    this->condDeq.notify_all();
    this->matchCondDeq.notify_all();
}

std::shared_ptr<VideoFrame> WorkerQueue::dequeue(){
    // exclusive access
    std::unique_lock<std::mutex> mlock( this->mutex );
    std::unique_lock<std::mutex> doTerminateLock( this->doTerminateMutex );

    while( this->items.empty() && ! this->doTerminate){
        doTerminateLock.unlock();
        // wait until item arrives
        this->condDeq.wait(mlock);
        doTerminateLock.lock();
    }

    if( this->doTerminate ){
        // release the locks in order to prevent deadlock
        doTerminateLock.unlock();
        mlock.unlock();
        // notify all producer blocking on enqueue() to ensure termination
        this->condEnq.notify_all();
        return nullptr;
    }else{
        // get item and delete from list
        std::shared_ptr<VideoFrame> item = this->items.front();
        this->items.pop_front();
        // release the lock in order to prevent deadlock
        doTerminateLock.unlock();
        mlock.unlock();
        // notify producer blocking on enqueue()
        this->condEnq.notify_one();
        return item;
    }
}

void WorkerQueue::enqueue( std::shared_ptr<VideoFrame> frame){
    // exclusive access
    std::unique_lock<std::mutex> mlock( this->mutex );
    std::unique_lock<std::mutex> maxLengthLock( this->maxLengthMutex );
    std::unique_lock<std::mutex> doTerminateLock( this->doTerminateMutex );

    while( this->items.size() == this->maxLength  && ! this->doTerminate ){
        // wait until item arrives & unlock in correct order
        doTerminateLock.unlock();
        maxLengthLock.unlock();
        this->condEnq.wait(mlock);
        // lock for next iteration
        maxLengthLock.lock();
        doTerminateLock.lock();
    }
    doTerminateLock.unlock();
    maxLengthLock.unlock();

    this->items.push_back( frame );

    mlock.unlock();
    doTerminateLock.lock();
    if( this->doTerminate ){
        // notify all consumer blocking on dequeue() to ensure termination
        doTerminateLock.unlock();
        this->condDeq.notify_all();
    }else{
        // notify one consumer blocking on dequeue()
        doTerminateLock.unlock();
        this->condDeq.notify_one();
    }
}



std::shared_ptr<Match> WorkerQueue::dequeueMatch(){
    // output the frames in sequencial order.
    std::unique_lock<std::mutex> mlock( this->matchMutex );
    std::unique_lock<std::mutex> doTerminateLock( this->doTerminateMutex );

    int i = 0;
    std::list< std::shared_ptr<Match> >::iterator index;
    bool found = false;
    while( this->matchItems.empty() && ! this->doTerminate ){
        doTerminateLock.unlock();
        // wait until item arrives
        this->matchCondDeq.wait(mlock);
        // lock for next iteration
        doTerminateLock.lock();
    }

    if( this->doTerminate ){
        // release the locks in order to prevent deadlock
        doTerminateLock.unlock();
        mlock.unlock();
        // notify all producer blocking on enqueue() to ensure termination
        this->matchCondEnq.notify_all();
        return nullptr;
    }else{
        // get item and delete from list
        std::shared_ptr<Match> item = this->matchItems.top();
        this->matchItems.pop();
        // release the lock in order to prevent deadlock
        doTerminateLock.unlock();
        mlock.unlock();
        // notify producer blocking on enqueue()
        this->matchCondEnq.notify_all();
        return item;
    }
}


void WorkerQueue::enqueueMatch( std::shared_ptr<Match> match){
    // exclusive access
    std::unique_lock<std::mutex> mlock( this->matchMutex );
    // no guard against a full buffer.
    this->matchItems.push( match );

    mlock.unlock();
    this->matchCondDeq.notify_all();
}

