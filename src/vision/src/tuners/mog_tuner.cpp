//sg: found this here: http://docs.opencv.org/3.1.0/d1/dc5/tutorial_background_subtraction.html

#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/bgsegm.hpp"

//C
#include <stdio.h>
//C++
#include <iostream>
#include <sstream>
using namespace cv;
using namespace std;
// Global variables
Mat frame; //current frame
Mat fgMaskMOG; //fg mask fg mask generated by MOG method
Mat fgMaskMOG2; //fg mask fg mask generated by MOG2 method
Ptr<BackgroundSubtractor> pMOG; //MOG Background subtractor
Ptr<BackgroundSubtractor> pMOG2; //MOG2 Background subtractor
int slider_area, slider_circularity, slider_ratio, slider_convexity;
int MAX_area = 1000, MAX_circularity = 1000, MAX_ratio = 1000, MAX_convexity = 1000;

// Setup SimpleBlobDetector parameters.
SimpleBlobDetector::Params params;
     
//opencv
int keyboard; //input from keyboard

void help();
void processVideo(char* videoFilename);
void help()
{
    cout
    << "--------------------------------------------------------------------------" << endl
    << "This program shows how to use background subtraction methods provided by "  << endl
    << " OpenCV. You can process videos."             << endl
                                                                                    << endl
    << "Usage:"                                                                     << endl
    << "./bs  <video filename>"                                                     << endl
    << "for example: ./bs  video.avi"                                               << endl
    << " file = cam for using webcam"                                               << endl
    << "--------------------------------------------------------------------------" << endl
    << endl;
}

void area( int, void* )
{
    //Filter by Area.
    params.filterByArea = true;
    params.minArea = slider_area;
}

void circularity( int, void* )
{
    //Filter by Circularity.
    params.filterByCircularity = true; 
    params.minCircularity = (float) slider_circularity / (float) MAX_circularity;
}

void convexity( int, void* )
{
    //Filter by Convexity.
    params.filterByConvexity = true; 
    params.minConvexity = (float) slider_convexity / (float) MAX_convexity;
}

void inertia( int, void* )
{
    //Filter by ratio of the inertia.
    params.filterByInertia = true; 
    params.minInertiaRatio = (float) slider_ratio / (float) MAX_ratio;
}

int main(int argc, char* argv[])
{
    //print help information
    help();
    //check for the input parameter correctness
    if(argc != 2){
        cerr <<"Incorrect input list" << endl;
        cerr <<"exiting..." << endl;
        return EXIT_FAILURE;
    }
    

    slider_area = 0;
    slider_circularity = 0;
    slider_convexity = 0;
    slider_ratio = 0;

    //create GUI window for the keypoints
    namedWindow("keypoints");

    //create all the trackbars
    createTrackbar( "area", "keypoints", &slider_area, MAX_area, area); 
    createTrackbar( "circularity", "keypoints", &slider_circularity, MAX_circularity, circularity);
    createTrackbar( "convexity", "keypoints", &slider_convexity, MAX_convexity, convexity);
    createTrackbar( "inertia ratio", "keypoints", &slider_ratio, MAX_ratio, inertia);
    
    //create Background Subtractor objects
    pMOG = bgsegm::createBackgroundSubtractorMOG(1000,5,.7,0);
    pMOG2 = createBackgroundSubtractorMOG2(1000,16,false);

    waitKey(0); 
    processVideo(argv[1]);
    
    //destroy GUI windows
    destroyAllWindows();
    return EXIT_SUCCESS;
}
void processVideo(char* videoFilename) {
    //All the frames needed 
    Mat copy, thresh, invert, gauss, mask;
   
    //create the capture object
    VideoCapture capture;   
    if(strcmp(videoFilename, "cam") == 0){
       capture.open(0);
    }
    else{
        capture.open(videoFilename);
    }       

    if(!capture.isOpened()){
        //error in opening the video input
        cerr << "Unable to open video file: " << videoFilename << endl;
        exit(EXIT_FAILURE);
    }

    VideoWriter outputVideo; //output video object
    Size S = Size((int) capture.get(CV_CAP_PROP_FRAME_WIDTH),    //Acquire input size
                  (int) capture.get(CV_CAP_PROP_FRAME_HEIGHT));

    int ex = static_cast<int>(capture.get(CV_CAP_PROP_FOURCC));     // Get Codec Type- Int form

    //    outputVideo.open("/home/sgillen/Desktop/log.avi" , ex , capture.get(CV_CAP_PROP_FPS),S,true);
   //outputVideo.open("/home/dlinko/Desktop/log1.avi" , ex , capture.get(CV_CAP_PROP_FPS),S,true);


    if(!outputVideo.isOpened()){
        cout << "something went wrong with opening the output video" << endl;
        cout << ex << endl;
        cout << S << endl;        
        cout << capture.get(CV_CAP_PROP_FPS) << endl;
    }

    //read input data. ESC or 'q' for quitting
    while( (char)keyboard != 'q' && (char)keyboard != 27 ){
         
        
        //read the current frame
        if(!capture.read(frame)) {
            cerr << "Unable to read next frame." << endl;
            cerr << "Exiting..." << endl;
            exit(EXIT_FAILURE);
        }

        //update the background model
        pMOG2->apply(frame, fgMaskMOG2);
        pMOG->apply(frame, fgMaskMOG);

        //get the frame number and write it on the current frame
        stringstream ss;
        rectangle(frame, cv::Point(10, 2), cv::Point(100,20),
                  cv::Scalar(255,255,255), -1);
        ss << capture.get(CAP_PROP_POS_FRAMES);
        string frameNumberString = ss.str();
        putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
                FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));
        //show the current frame and the fg masks

        imshow("FG Mask MOG", fgMaskMOG);
        imshow("FG Mask MOG 2", fgMaskMOG2);

        //blurs the image uses the MOG background subrtaction
        GaussianBlur(fgMaskMOG, gauss, Size(3,3), 0,0);

        //blurs the image uses the MOG2 background subtraction 
        //GaussianBlur(fgMaskMOG2, gauss, Size(3,3), 0,0);

        // Define the structuring elements to be used in eroding and dilating the image 
        Mat se1 = getStructuringElement(MORPH_RECT, Size(5, 5));
        Mat se2 = getStructuringElement(MORPH_RECT, Size(5, 5));

        // Perform dialting and eroding helps to elminate background noise 
        morphologyEx(gauss, mask, MORPH_CLOSE, se1);
        morphologyEx(gauss, mask, MORPH_OPEN, se2);

        //inverts the colors 
        bitwise_not(mask, invert, noArray());
        imshow("invert", invert);
        
        // Change thresholds
        params.minThreshold = 0;
        params.maxThreshold = 256;

        params.filterByColor = true;
        params.blobColor= 0;
         
        // Storage for blobs
        vector<KeyPoint> keypoints;
         
        // Set up detector with params
        Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create(params);
        
        // SimpleBlobDetector::create creates a smart pointer. 
        // So you need to use arrow ( ->) instead of dot ( . )
        detector->detect( invert, keypoints);
          
        // Draw detected blobs as red circles.
        // DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures
        // the size of the circle corresponds to the size of blob

        Mat im_with_keypoints;
        drawKeypoints(frame, keypoints, im_with_keypoints, Scalar(0,0,255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS );

        // Shows blobs
        imshow("keypoints", im_with_keypoints );            
        
        //outputVideo.write(im_with_keypoints);

        //get the input from the keyboard
        keyboard = waitKey( 60 );
    }
    //delete capture object
    capture.release();
}
