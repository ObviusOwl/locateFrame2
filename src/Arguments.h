#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <exception>
#include <string>
#include <argp.h>
#include <vector>

class Arguments{
public:
    Arguments();

    void setMinFrame( int frameNumber );
    void setMaxFrame( int frameNumber );
    void setDoScale();
    void setHessianThreshold( int thres );
    void setKeypointMatchRadius( double r );
    void setMatcherThreads( int count );
    void setDecoderThreads( int count );
    void setQueueSize( int count );
    void setInputFile( std::string fileName );
    void setOutputFile( std::string fileName );
    void addSearchFile( std::string fileName );
    void addMatchRatio( double r );
    void addSnrRatio( double r );

    int getMinFrame();
    int getMaxFrame();
    bool doScale();
    int getHessianThreshold();
    double getKeypointMatchRadius();
    int getMatcherThreads();
    int getDecoderThreads();
    int getQueueSize();
    std::string getInputFile();
    std::string getOutputFile();
    std::vector<std::string> getSearchFiles();
    std::vector<double> getMatchRatios();
    std::vector<double> getSnrRatios();

    int parseArgs( int argc, char **argv );
    void setArgpState( struct argp_state *state);
    void exitErrorHelp( const std::string& msg );
    long int parseIntNumber( const std::string& arg );
    double parseDoubleNumber( const std::string& arg );
    double parsePercentToRatio( const std::string& arg );

    void printArguments();

    /* argp stuff */
    static char prog_doc[];
    static char args_doc[];
    static struct argp_option options[];
    static struct argp argp;
    struct argp_state *argpState;
    static error_t argp_parse_opt (int key, char *arg, struct argp_state *state);

private:
    /* config options*/
    int minFrame;
    int maxFrame;
    int hessianThreshold;
    double keypointMatchRadius;
    int matcherThreads;
    int decoderThreads;
    int queueSize;
    bool scale;
    std::vector<std::string> searchFiles;
    std::string inputFile;
    std::string outputFile;
    std::vector<double> matchRatios;
    std::vector<double> snrRatios;
};

#endif // ARGUMENTS_H
