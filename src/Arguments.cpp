#include <stdlib.h>
#include <errno.h>
#include <argp.h>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstdio>

#include "Arguments.h"

char Arguments::prog_doc[] = "Find frames in a video file";
char Arguments::args_doc[] = "-i VIDEO IMAGE [IMAGE ...]";

struct argp_option Arguments::options[] = {
    { "min-frame",  'm',    "number",   0,  "Skip the first m frames", 0},
    { "max-frame",  'M',    "number",   0,  "Search up to the M'th frame. Default until the end of the file.", 0},
    { "thres",      'H',    "number",   0,  "Ignored.",0},
    { "scale",      'S',      NULL,    0,  "Scale the input images to the video's dimensions. Default false.",0},
    { "match-ratio",'r',    "float",    0,  "Minimum percentage [0-100] of image keypoint matches required to consider a frame as fully matched. Can be combined with -s. Default 100%. Repeat for each input image.", 0},
    { "snr",        's',    "float",    0,  "Minimum Signal to noise ratio [ >= 0] (keypoint matches / keypoint misses) required to consider a frame as fully matched. Can be combined with -r. Default infinity. Repeat for each input image.", 0},
    { "radius",     'R',    "float",    0,  "Radius of the circle around a keypoint of the image in which a keypoint of the frame must be to be considered a keypoint match. Default 5",0},
    { "threads",    't',    "number",   0,  "Number of threads to start for the matching task, default 1",0},
    { "ff-threads", 'T',    "number",   0,  "Number of threads for video decoding, default auto",0},
    { "queue",      'q',    "number",   0,  "Length of the frame queue between decoder and matcher threads. Default 5.",0 },
    { NULL,         'i',    "FILE",     0,  "Input video file",0 },
    { "output",     'o',    "FILE.mpg", 0,  "Output a video with the keypoints drawn onto it. The keypoint matches for the first input image are colored in green.",0 },
    { 0 }
};

// pointers to static functions are valid C function pointers
struct argp Arguments::argp = { Arguments::options, Arguments::argp_parse_opt, Arguments::args_doc, Arguments::prog_doc, NULL, NULL, NULL };


Arguments::Arguments(){
    this->minFrame = 0;
    this->maxFrame = -1;
    this->hessianThreshold = 300;
    this->argpState = NULL;
    this->matcherThreads = 1;
    this->decoderThreads = -1;
    this->queueSize = 5;
    this->outputFile = "";
    this->scale = false;
}

int Arguments::parseArgs( int argc, char **argv ){
    argp_parse(&(this->argp), argc, argv, 0, 0, this);
    return 0;
}

/* Setters */

void Arguments::setArgpState( struct argp_state *state){
    this->argpState = state;
}

void Arguments::setMinFrame( int frameNumber ){
    this->minFrame = frameNumber;
}

void Arguments::setDoScale(){
    this->scale = true;
}

void Arguments::setMaxFrame( int frameNumber ){
    this->maxFrame = frameNumber;
}
void Arguments::setHessianThreshold( int thres ){
    this->hessianThreshold = thres;
}
void Arguments::setKeypointMatchRadius( double r ){
    this->keypointMatchRadius = r;
}

void Arguments::setMatcherThreads( int count ){
    this->matcherThreads = count;
}
void Arguments::setDecoderThreads( int count ){
    this->decoderThreads = count;
}
void Arguments::setQueueSize( int count ){
    this->queueSize = count;
}

void Arguments::addSearchFile( std::string fileName ){
    this->searchFiles.push_back( fileName );
}

void Arguments::setInputFile( std::string fileName ){
    this->inputFile = fileName;
}

void Arguments::setOutputFile( std::string fileName ){
    this->outputFile = fileName;
}

void Arguments::addMatchRatio( double r ){
    this->matchRatios.push_back(r);
}
void Arguments::addSnrRatio( double r ){
    this->snrRatios.push_back(r);
}



/* Getters */

int Arguments::getMinFrame(){
    return this->minFrame;
}
int Arguments::getMaxFrame(){
    return this->maxFrame;
}
bool Arguments::doScale(){
    return this->scale;
}
int Arguments::getHessianThreshold(){
    return this->hessianThreshold;
}
double Arguments::getKeypointMatchRadius(){
    return this->keypointMatchRadius;
}

int Arguments::getMatcherThreads(){
    return this->matcherThreads;
}
int Arguments::getDecoderThreads(){
    return this->decoderThreads;
}
int Arguments::getQueueSize(){
    return this->queueSize;
}

std::vector<std::string> Arguments::getSearchFiles(){
    return this->searchFiles;
}

std::string Arguments::getInputFile(){
    return this->inputFile;
}

std::string Arguments::getOutputFile(){
    return this->outputFile;
}

std::vector<double> Arguments::getMatchRatios(){
    return this->matchRatios;
}
std::vector<double> Arguments::getSnrRatios(){
    return this->snrRatios;
}


/* Parsing */

void Arguments::exitErrorHelp( const std::string& msg ){
    std::printf( "%s\n\n", msg.c_str() );
    if( this->argpState != NULL ){
        argp_usage( this->argpState );
    }
    std::exit( 1 );
}

long int Arguments::parseIntNumber( const std::string& arg ){
    try{
        std::size_t pos = 0;
        long int val = std::stol( arg, &pos, 10 );
        if( pos < arg.length() ){
            this->exitErrorHelp( "Not a valid number." );
        }
        return val;
    }catch( const std::invalid_argument& e ){
        this->exitErrorHelp( "Invalid argument" );
    }catch( const std::out_of_range& e ){
        this->exitErrorHelp( "Number out of range" );
    }
}

double Arguments::parseDoubleNumber( const std::string& arg ){
    try{
        std::size_t pos = 0;
        double val = std::stod( arg, &pos );
        if( pos < arg.length() ){
            this->exitErrorHelp( "Not a valid floating point number." );
        }
        return val;
    }catch( const std::invalid_argument& e ){
        this->exitErrorHelp( "Invalid argument" );
    }catch( const std::out_of_range& e ){
        this->exitErrorHelp( "Number out of range" );
    }
}

double Arguments::parsePercentToRatio( const std::string& arg ){
    double val = this->parseDoubleNumber( arg ) / 100;
    if( val < 0.0 ){
        this->exitErrorHelp( "No negative value allowed" );
    }
    if( val > 1.0 ){
        this->exitErrorHelp( "Number must be in between 0 and 100" );
    }
    return val;
}

error_t Arguments::argp_parse_opt (int key, char *arg, struct argp_state *state){
    // cast back our instance passed to argp in Arguments::parseArgs
    Arguments * self = static_cast<Arguments *>(state->input);
    self->setArgpState( state );

    // 0-length args (switches)
    switch (key){
    case 'S': ;
        self->setDoScale();
        return 0;
    }

    // args with a value
    if( arg == NULL ){
        // require a value
        return ARGP_ERR_UNKNOWN;
    }
    std::string argstr(arg);

    switch (key){
    case 'm': ;
        self->setMinFrame( self->parseIntNumber( argstr ) );
        break;
    case 'M': ;
        self->setMaxFrame( self->parseIntNumber( argstr ) );
        break;
    case 'H': ;
        self->setHessianThreshold( self->parseIntNumber( argstr ) );
        break;
    case 'R': ;
        self->setKeypointMatchRadius( self->parseDoubleNumber( argstr ) );
        break;
    case 't': ;
        self->setMatcherThreads( self->parseIntNumber( argstr ) );
        break;
    case 'T': ;
        self->setDecoderThreads( self->parseIntNumber( argstr ) );
        break;
    case 'q': ;
        self->setQueueSize( self->parseIntNumber( argstr ) );
        break;
    case 'r': ;
        self->addMatchRatio( self->parsePercentToRatio( argstr ) );
        break;
    case 's': ;
        self->addSnrRatio( self->parseDoubleNumber( argstr ) );
        break;
    case 'i': ;
        self->setInputFile( argstr );
        break;
    case 'o': ;
        self->setOutputFile( argstr );
        break;
    case ARGP_KEY_ARG:
        self->addSearchFile( argstr );
        break;
    case ARGP_KEY_END:
        if( self->getSearchFiles().empty() ){
            self->exitErrorHelp( "no input images specified" );
        }
        if( self->getInputFile().empty() ){
            self->exitErrorHelp( "no input video specified (-i)" );
        }
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

void Arguments::printArguments(){
    std::printf( "minFrame: %d\n", this->getMinFrame() );
    std::printf( "maxFrame: %d\n", this->getMaxFrame() );
    std::printf( "scale: %d\n", this->doScale() );
    std::printf( "matcherThreads: %d\n", this->getMatcherThreads() );
    std::printf( "decoderThreads: %d\n", this->getDecoderThreads() );
    std::printf( "queueSize: %d\n", this->getQueueSize() );
    std::printf( "keypointMatchRadius: %f\n", this->getKeypointMatchRadius() );

    std::printf( "inputFile: %s\n", this->getInputFile().c_str() );
    if( this->getOutputFile() != "" ){
        std::printf( "outputFile: %s\n", this->getOutputFile().c_str() );
    }

    for ( auto &sFile : this->getSearchFiles() ) {
        std::printf( "searchFile: %s\n", sFile.c_str() );
    }
    for ( auto &r : this->getMatchRatios() ) {
        std::printf( "matchRatio: %f\n", r );
    }
    for ( auto &r : this->getSnrRatios() ) {
        std::printf( "snrRatio: %f\n", r );
    }
}

