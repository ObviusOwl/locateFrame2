#ifndef KD_TREE_H
#define KD_TREE_H

#include <string>
#include <memory>
#include <opencv2/opencv.hpp>


class KdTreeNode{

public:
    KdTreeNode();
    KdTreeNode( cv::KeyPoint& keypoint );
    KdTreeNode( cv::KeyPoint& keypoint,  KdTreeNode& left, KdTreeNode& right );
    KdTreeNode( int x, int y, KdTreeNode& left, KdTreeNode& right );

    int autoID( int previous );
    void dumpDOT( );

    double getDistanceTo( int x, int y );
    double getDistanceTo( KdTreeNode& node );
    bool empty();
    bool leaf();

    int getID();
    int getX();
    int getY();
    std::shared_ptr<KdTreeNode> getLeftChild();
    std::shared_ptr<KdTreeNode> getRightChild();
    cv::KeyPoint getKeyPoint();

    void setID( int id );

private:
    int x;
    int y;
    int ID;
    bool isEmpty;
    bool isLeaf;
    cv::KeyPoint keypoint;
    std::shared_ptr<KdTreeNode> leftChild;
    std::shared_ptr<KdTreeNode> rightChild;
};


class KdTree{

public:
    KdTree();
    KdTree( std::vector<cv::KeyPoint>& keypoints);

    void dumpDOT( );
    cv::KeyPoint nearestNeighborSearch( int x, int y);


    static bool compareKeyPointByX( cv::KeyPoint& lhs, cv::KeyPoint& rhs );
    static bool compareKeyPointByY( cv::KeyPoint& lhs, cv::KeyPoint& rhs );

    KdTreeNode build( std::vector<cv::KeyPoint> keypoints, int depth );
    KdTreeNode nearestNeighborSearchRec( std::shared_ptr<KdTreeNode> currentNode, int searchX, int searchY, 
        int depth, KdTreeNode bestNode, double bestDistance );

private:
    std::shared_ptr<KdTreeNode> root;
};


#endif // KD_TREE_H