#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <queue>
#include <opencv2/opencv.hpp>

#include "WorkerQueue.h"
#include "VideoFrame.h"
#include "Match.h"

bool MatchComparator::operator() (std::shared_ptr<Match> m1, std::shared_ptr<Match> m2) {
    long int frame1 = m1->getFrameIndex();
    long int frame2 = m2->getFrameIndex();
    if( frame1 > frame2 ){
        return true;
    }else if( frame1 < frame2 ){
        return false;
    }else{
        return ( m1->getImageIndex() < m2->getImageIndex() );
    }
}

bool VideoFrameComparator::operator() (std::shared_ptr<VideoFrame> f1, std::shared_ptr<VideoFrame> f2) {
    return (f1->getIndex() > f2->getIndex()); // sort smallest index first
}

WorkerQueue::WorkerQueue(){
    this->maxLength = 5;
    this->imageCount = 0;
    this->imagesFound = 0;
    this->doTerminate = false;
    this->matchDequeueIndex = -1;
    this->matchDequeueImageCount = 0;
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

bool WorkerQueue::getTerminate(){
    std::unique_lock<std::mutex> mlock( this->doTerminateMutex );
    bool term = this->doTerminate;
    mlock.unlock();
    return term;
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
        std::shared_ptr<VideoFrame> item = this->items.top();
        this->items.pop();
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
    std::unique_lock<std::mutex> doTerminateLock( this->doTerminateMutex );

    while( this->items.size() == this->maxLength  && ! this->doTerminate ){
        // wait until item arrives & unlock in correct order
        doTerminateLock.unlock();
        this->condEnq.wait(mlock);
        // lock for next iteration
        doTerminateLock.lock();
    }
    doTerminateLock.unlock();

    this->items.push( frame );
    int s = this->items.size();

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

    bool found = true;
    while( true ){
        if( this->doTerminate && this->matchItems.empty() ){
            // empty the queue before terminate
            break;
        }
        doTerminateLock.unlock();

        if( ! this->matchItems.empty() ){
            std::shared_ptr<Match> item = this->matchItems.top();
            if( this->matchDequeueIndex < 0 ){
                // init -> set up like we just dequeued the last match from the preceeding frame
                // there is a chance we do not get the really first frame -> handle special later
                this->matchDequeueIndex = item->getFrameIndex()-1;
                this->matchDequeueImageCount = this->imageCount;
            }
            long int dqIdx = this->matchDequeueIndex;
            long int frameIdx = item->getFrameIndex();

            if( dqIdx+1 == frameIdx || frameIdx <= dqIdx ){
                if( frameIdx < dqIdx+1 ){
                    // frame too late -> pretend it has never existed
                }else{
                    if( this->matchDequeueImageCount == this->imageCount ){
                        this->matchDequeueImageCount = 0;
                        this->matchDequeueIndex = frameIdx; // max(dqIdx,frameIdx)==frameIdx at this point
                    }
                    this->matchDequeueImageCount = this->matchDequeueImageCount +1;
                }
                // dequeue the match
                (this->matchItems).pop();
                mlock.unlock();
                // notify producer blocking on enqueue()
                this->matchCondEnq.notify_all();
                found = true;
                return item;
            }else{
                found = false;
            }
        }

        if( this->matchItems.empty() || ! found ){
            // wait until item arrives
            this->matchCondDeq.wait(mlock);
        }

        // lock for next iteration
        doTerminateLock.lock();
    }

    // release the locks in order to prevent deadlock
    doTerminateLock.unlock();
    mlock.unlock();
    // notify all producer blocking on enqueue() to ensure termination
    this->matchCondEnq.notify_all();
    return nullptr;
}


void WorkerQueue::enqueueMatch( std::shared_ptr<Match> match){
    // exclusive access
    std::unique_lock<std::mutex> mlock( this->matchMutex );
    // no guard against a full buffer.
    this->matchItems.push( match );

    mlock.unlock();
    this->matchCondDeq.notify_all();
}

