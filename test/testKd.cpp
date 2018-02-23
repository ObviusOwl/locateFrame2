#include "gtest/gtest.h"

#include <vector>
#include <cstdio>
#include <opencv2/opencv.hpp>
#include <opencv2/nonfree/features2d.hpp>

#include "../src/KdTree.h"

struct point{
    int x;
    int y;
};

class KdTest : public ::testing::Test {

protected:

    virtual void SetUp(){
        //struct point coord[15] = { {23,28},{9,26},{10,8},{6,22},{16,22},{5,26},{29,14},{11,29},{27,25},{28,9},{18,10},{7,4},{13,25},{12,4},{14,6} };
        struct point coord[50] ={{8,42},{48,11},{6,25},{46,24},{20,28},{19,12},{11,46},{8,43},{15,41},{5,23},
                                {14,25},{1,43},{7,24},{5,22},{39,11},{48,22},{48,11},{6,44},{35,11},{21,14},
                                {48,13},{34,19},{12,0},{12,26},{22,37},{40,42},{45,40},{47,47},{38,7},{32,10},
                                {11,10},{41,33},{11,4},{7,18},{9,11},{31,49},{47,4},{2,31},{41,17},{23,12},
                                {40,16},{4,25},{16,1},{39,29},{43,13},{47,44},{47,14},{20,8},{32,7},{32,36}};

        cv::KeyPoint kp;
        int n = sizeof(coord) / sizeof(coord[0]);
        for( int i=0; i< n; i++ ){
            kp.pt.x = coord[i].x;
            kp.pt.y = coord[i].y;
            this->keypoints.push_back( kp );
        }
    }

    std::vector<cv::KeyPoint> keypoints;

};

TEST_F(KdTest, nearestNeighborSearch) {
    struct point s = {11,9};
    struct point nnExp = {11,10};

    KdTree tree = KdTree( this->keypoints );
    cv::KeyPoint nn = tree.nearestNeighborSearch( s.x, s.y );

    //tree.dumpDOT();
    //std::printf( "neares neighbor of (%d,%d) is (%d,%d)\n", s.x, s.y, (int) nn.pt.x, (int) nn.pt.y );
    EXPECT_EQ( nnExp.x , nn.pt.x);
    EXPECT_EQ( nnExp.y , nn.pt.y);
}

TEST_F(KdTest, compareKeyPointByX) {
    cv::KeyPoint a;
    cv::KeyPoint b;
    a.pt.x = 10;
    a.pt.y = 15;
    b.pt.x = 20;
    b.pt.y = 35;
    
    EXPECT_TRUE( KdTree::compareKeyPointByX( a, b ) );
    EXPECT_FALSE( KdTree::compareKeyPointByX( b, a ) );
}

TEST_F(KdTest, compareKeyPointByY) {
    cv::KeyPoint a;
    cv::KeyPoint b;
    a.pt.x = 10;
    a.pt.y = 15;
    b.pt.x = 20;
    b.pt.y = 35;
    
    EXPECT_TRUE( KdTree::compareKeyPointByY( a, b ) );
    EXPECT_FALSE( KdTree::compareKeyPointByY( b, a ) );
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
