#include <stdexcept>
#include <opencv2/opencv.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <algorithm>
#include <stdexcept>
#include <cstdio>
#include <cmath>
#include <limits>
#include <memory>

#include "KdTree.h"


/* Node Constructors */

KdTreeNode::KdTreeNode(){
    // constructor for empty nodes
    this->isEmpty = true;
    this->isLeaf = true;
    this->x = 0;
    this->y = 0;
    this->leftChild = nullptr;
    this->rightChild = nullptr;
}
KdTreeNode::KdTreeNode( int x, int y, KdTreeNode& left, KdTreeNode& right ){
    // not really used
    this->isEmpty = false;
    this->isLeaf = false;
    this->x = x;
    this->y = y;
    this->leftChild = std::shared_ptr<KdTreeNode>( new KdTreeNode(left) );
    this->rightChild = std::shared_ptr<KdTreeNode>( new KdTreeNode(right) );
}
KdTreeNode::KdTreeNode( cv::KeyPoint& keypoint, KdTreeNode& left, KdTreeNode& right ){
    // constructor for regular nodes
    this->isEmpty = false;
    this->isLeaf = false;
    this->keypoint = keypoint;
    this->x = (int) keypoint.pt.x;
    this->y = (int) keypoint.pt.y;
    this->leftChild = std::shared_ptr<KdTreeNode>( new KdTreeNode(left) );
    this->rightChild = std::shared_ptr<KdTreeNode>( new KdTreeNode(right) );
}
KdTreeNode::KdTreeNode( cv::KeyPoint& keypoint ){
    // constructor for leaf node
    this->isEmpty = false;
    this->isLeaf = true;
    this->keypoint = keypoint;
    this->x = (int) keypoint.pt.x;
    this->y = (int) keypoint.pt.y;
}

/* Node Methods */

bool KdTreeNode::empty(){ 
    return this->isEmpty; 
};
bool KdTreeNode::leaf(){ 
    return this->isLeaf; 
};

int KdTreeNode::autoID( int previous ){
    int next = previous+1; 
    this->setID( next );
    if( ! this->leaf() ){
        next = this->leftChild->autoID( next );
        next = this->rightChild->autoID( next );
    }
    return next;
}

void KdTreeNode::dumpDOT( ){
    if( ! this->leaf() ){
        std::printf( "\t\"%d-(%d,%d)\" -> \"%d-(%d,%d)\" [color=blue];\n",
            this->ID, this->x, this->y,
            this->leftChild->getID(), this->leftChild->getX(), this->leftChild->getY() );
        std::printf( "\t\"%d-(%d,%d)\" -> \"%d-(%d,%d)\" [color=red];\n",
            this->ID, this->x, this->y,
            this->rightChild->getID(), this->rightChild->getX(), this->rightChild->getY() );
        // recursion
        this->rightChild->dumpDOT();
        this->leftChild->dumpDOT();
    }
}

/* Node Getters / setters */

double KdTreeNode::getDistanceTo( int x, int y){
    // pythagoras without squareroot = squared distance
    double d = std::pow( this->getX() - x,2) + pow(this->getY() - y,2);
    if( d < 0.0 ){
        // euclidian vector direction does not matter
        return -1.0*d;
    }
    return d;
}

double KdTreeNode::getDistanceTo( KdTreeNode& node){
    return this->getDistanceTo( node.getX(), node.getY() );
}

int KdTreeNode::getID(){ 
    return this->ID; 
}
int KdTreeNode::getX(){ 
    return this->x; 
}
int KdTreeNode::getY(){ 
    return this->y; 
}
std::shared_ptr<KdTreeNode> KdTreeNode::getLeftChild(){ 
    if( this->leaf() ){
        throw std::runtime_error("this node is a leaf");
    }
    return this->leftChild; 
}
std::shared_ptr<KdTreeNode> KdTreeNode::getRightChild(){ 
    if( this->leaf() ){
        throw std::runtime_error("this node is a leaf");
    }
    return this->rightChild; 
}
cv::KeyPoint KdTreeNode::getKeyPoint(){ 
    return this->keypoint; 
}

void KdTreeNode::setID( int id ){ 
    this->ID = id; 
}


/* Tree Constructors */

KdTree::KdTree(){
}
KdTree::KdTree( std::vector<cv::KeyPoint>& keypoints){
    KdTreeNode root = this->build( keypoints, 0 );
    this->root = std::make_shared<KdTreeNode>(std::move(root));;
    if( this->root->empty() ){
        throw std::runtime_error("root node is empty");
    }
}

/* Tree Methods */

void KdTree::dumpDOT( ){
    this->root->autoID(0);
    printf( "digraph graphname {\n" );
    this->root->dumpDOT();
    printf( "}\n" );
}

cv::KeyPoint KdTree::nearestNeighborSearch( int x, int y){
    KdTreeNode nullNode;
    KdTreeNode result;
    result = this->nearestNeighborSearchRec( this->root, x, y, 0, nullNode, std::numeric_limits<double>::infinity());
    if( result.empty() ){
        throw std::runtime_error("no result found");
    }
    return result.getKeyPoint();
}

/* Treee Helpers */

bool KdTree::compareKeyPointByX( cv::KeyPoint& lhs, cv::KeyPoint& rhs ){
    return (int) lhs.pt.x < (int) rhs.pt.x;
}
bool KdTree::compareKeyPointByY( cv::KeyPoint& lhs, cv::KeyPoint& rhs ){
    return (int) lhs.pt.y < (int) rhs.pt.y;
}


/* Tree KD Datastructure */

KdTreeNode KdTree::build( std::vector<cv::KeyPoint> keypoints, int depth ){
    if( keypoints.size() == 0 ){
        KdTreeNode node;
        return node;
    }
    if( keypoints.size() == 1 ){
        KdTreeNode node( keypoints[0] );
        return node;
    }
    // cycle axis
    int axis = depth % 2;
    std::vector<cv::KeyPoint> leftPoints;
    std::vector<cv::KeyPoint> rightPoints;
    
    // sort by axis. ok since keypoints passed by value
    if( axis == 0 ){
        std::sort( keypoints.begin(), keypoints.end(), KdTree::compareKeyPointByX );
    }else{
        std::sort( keypoints.begin(), keypoints.end(), KdTree::compareKeyPointByY );
    }
    int medianIdx = keypoints.size() / 2;

    // sort keypints into left and right side of median
    int i;
    for(i = 0; i < medianIdx; i++) {
        leftPoints.push_back( keypoints[i] );
    }
    for(i = medianIdx+1; i < keypoints.size(); i++) {
        rightPoints.push_back( keypoints[i] );
    }
    if( leftPoints.size() + rightPoints.size() + 1 != keypoints.size() ){
        throw std::runtime_error("size missmatch of left+right");
    }
    
    // recursion
    KdTreeNode leftNode = this->build( leftPoints, depth+1 );
    KdTreeNode rightNode = this->build( rightPoints, depth+1 );
    // fill node
    KdTreeNode node( keypoints[medianIdx], leftNode, rightNode );
    return node;
}

KdTreeNode KdTree::nearestNeighborSearchRec( std::shared_ptr<KdTreeNode> currentNode, int searchX, int searchY, 
    int depth, KdTreeNode bestNode, double bestDistance ){

    // check if this node is better than the others
    double currentDistance = currentNode->getDistanceTo( searchX, searchY );
    if( currentDistance < bestDistance ){
        bestDistance = currentDistance;
        bestNode = *currentNode;
    }

    // leaf nodes
    if( currentNode->leaf() ){
        return bestNode;
    }
    
    if( currentNode->getLeftChild()->leaf() && currentNode->getRightChild()->leaf() ){
        // cannot descend into children of leaf nodes
        return bestNode;
    }
    std::shared_ptr<KdTreeNode> visitFirst;
    std::shared_ptr<KdTreeNode> visitLast;
    // visit first the side where the search point would be inserted
    if( depth % 2 == 0 ){
        if( searchX > currentNode->getX() ){
            visitFirst = currentNode->getRightChild();
            visitLast = currentNode->getLeftChild();
        }else{
            visitFirst = currentNode->getLeftChild();
            visitLast = currentNode->getRightChild();
        }
    }else{
        if( searchY > currentNode->getY() ){
            visitFirst = currentNode->getRightChild();
            visitLast = currentNode->getLeftChild();
        }else{
            visitFirst = currentNode->getLeftChild();
            visitLast = currentNode->getRightChild();
        }
    }
    // recursion: visit first child
    bestNode = this->nearestNeighborSearchRec( visitFirst, searchX, searchY, 
        depth+1, bestNode, bestDistance);

    // check if there may be a closer point on the other side
    // create a circle around the search point with currentNode on that circle
    double radius = sqrt(bestNode.getDistanceTo( searchX, searchY ));

    double distanceAxis; // distance from current node to the splitting axis
    if( depth % 2 == 0 ){
        distanceAxis = (double) searchX - currentNode->getX();
    }else{
        distanceAxis = (double) searchY - currentNode->getY();
    }
    if( distanceAxis < 0 ){
        // ignore direction
        distanceAxis = -1.0*distanceAxis;
    }

    // check if the circle crosses the boundary of the current splitting axis
    if( distanceAxis < radius ){
        // visit other side if there is doubt: recursion
        bestNode = this->nearestNeighborSearchRec( visitLast, searchX, searchY, 
            depth+1, bestNode, bestDistance);
    }
    return bestNode;
}
