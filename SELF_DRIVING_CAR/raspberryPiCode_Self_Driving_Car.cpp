#include <opencv2/opencv.hpp> // include opnCV library
#include <raspicam_cv.h> // include raspicam library
#include <iostream>
#include <chrono> // chrono library used for calculate frame per second(FPS)
#include <ctime> // this library is for calculate total time of execution
#include <wiringPi.h>

using namespace std;
using namespace cv;
using namespace raspicam;

// image peocessing----these are Mat global variables
Mat frame,Matrix,framePers,frameGray,frameThresh,frameEdge,frameFinal,ROILane,ROILaneEnd,frameFinalDuplicate,frameFinalDuplicate1;
int LeftLanePos,RightLanePos,laneCenter,frameCenter,Result,laneEnd;

RaspiCam_Cv Camera; // Camera object created and it's global
stringstream ss;
vector<int> histrogramLane; // dynamic array created 
vector<int> histrogramLaneEnd; // dynamic array created  for lANE END

Point2f Source[] = {Point2f(31,145),Point2f(319,145),Point2f(6,195), Point2f(348,199)}; // 4 regions coordinate points, Stored in Source array with Point2f datatypes
Point2f Destination[] = {Point2f(60,0),Point2f(300,0),Point2f(60,240), Point2f(300,240)};// create 4 perspective wrap points and stored in Destination array 
// basic setup for our project
void Setup ( int argc,char **argv, RaspiCam_Cv &Camera )
{   
    
    Camera.set ( CAP_PROP_FRAME_WIDTH,  ( "-w",argc,argv,360 ) );
    Camera.set ( CAP_PROP_FRAME_HEIGHT,  ( "-h",argc,argv,240 ) );  // resulation = 360*240 , our raspberry pi can not work with high resolation 
    Camera.set ( CAP_PROP_BRIGHTNESS, ( "-br",argc,argv,50 ) );
    Camera.set ( CAP_PROP_CONTRAST ,( "-co",argc,argv,50 ) );
    Camera.set ( CAP_PROP_SATURATION,  ( "-sa",argc,argv,50 ) );
    Camera.set ( CAP_PROP_GAIN,  ( "-g",argc,argv ,50 ) );
    Camera.set ( CAP_PROP_FPS,  ( "-fps",argc,argv,0)); //  Frame per second  count

}

// Perspective Wrap function creation
void Perspective()
{       
         // 4 points for region  ,using line method( image, source,line color,line width)
	line(frame,Source[0], Source[1], Scalar(0,0,255), 2);
	line(frame,Source[1], Source[3], Scalar(0,0,255), 2);
	line(frame,Source[3], Source[2], Scalar(0,0,255), 2);
	line(frame,Source[2], Source[0], Scalar(0,0,255), 2);
	
	// 4 points for perspective wrap test
	//line(frame,Destination[0], Source[1], Scalar(0,255,0), 2);
	//line(frame,Destination[1], Source[3], Scalar(0,255,0), 2);
	//line(frame,Destination[3], Source[2], Scalar(0,255,0), 2);
	//line(frame,Destination[2], Source[0], Scalar(0,255,0), 2);
	
	Matrix = getPerspectiveTransform(Source, Destination);
	warpPerspective(frame, framePers, Matrix, Size(360,240)); // new perspective wrap create
	
}

void Threshold()
{
	cvtColor(framePers, frameGray, COLOR_RGB2GRAY); // RGB color image change to Gray color image
	inRange(frameGray, 50, 255, frameThresh);
	Canny(frameGray,frameEdge,70, 300, 3, false);
	add(frameThresh, frameEdge, frameFinal);
	cvtColor(frameFinal, frameFinal, COLOR_GRAY2RGB);
	cvtColor(frameFinal, frameFinalDuplicate, COLOR_RGB2BGR);   //used in histrogram function only
	cvtColor(frameFinal, frameFinalDuplicate1, COLOR_RGB2BGR);
}

void Histrogram()
{
    histrogramLane.resize(360);
    histrogramLane.clear();
    
    for(int i=0; i<360; i++)       //frame.size().width = 360
    {
	ROILane = frameFinalDuplicate(Rect(i,140,1,100));
	divide(255, ROILane, ROILane);
	histrogramLane.push_back((int)(sum(ROILane)[0])); 
    }
    
    histrogramLaneEnd.resize(360); // laneEnd detect
    histrogramLaneEnd.clear(); 
    
    for(int i=0; i<360; i++)       //frame.size().width = 360
    {
	ROILaneEnd = frameFinalDuplicate1(Rect(i,0,1,240));
	divide(255, ROILaneEnd, ROILaneEnd);
	histrogramLaneEnd.push_back((int)(sum(ROILaneEnd)[0])); 
    }
    
    laneEnd=sum(histrogramLaneEnd)[0];
    cout<<" Lane End = "<<laneEnd<<endl; // laneEnd detect
}


void LaneFinder()
{
    vector<int>:: iterator LeftPtr;
    LeftPtr = max_element(histrogramLane.begin(), histrogramLane.begin() + 130);
    LeftLanePos = distance(histrogramLane.begin(), LeftPtr); // actual distance for our first lane 
    
    vector<int>:: iterator RightPtr;
    RightPtr = max_element(histrogramLane.begin() +230, histrogramLane.end());
    RightLanePos = distance(histrogramLane.begin(), RightPtr); // actual distance for our second lane
    
    // show the left and right position distance in console
    line(frameFinal, Point2f(LeftLanePos, 0), Point2f(LeftLanePos, 240), Scalar(0, 255,0), 2);
    line(frameFinal, Point2f(RightLanePos, 0), Point2f(RightLanePos, 240), Scalar(0,255,0), 2); 
}

void LaneCenter()
{
    laneCenter = (RightLanePos-LeftLanePos)/2 +LeftLanePos;
    frameCenter = 180;
    
    line(frameFinal, Point2f(laneCenter,0), Point2f(laneCenter,240), Scalar(0,255,0), 3); // draw line in console
    line(frameFinal, Point2f(frameCenter,0), Point2f(frameCenter,240), Scalar(255,0,0), 3);// draw line in console

    Result = laneCenter-frameCenter;
}

// Video capture  function
void Capture()
{
    Camera.grab();
    Camera.retrieve( frame);
    cvtColor(frame, frame, COLOR_BGR2RGB); // convert Our BGR image to RGB color
}

int main(int argc,char **argv)
{
        wiringPiSetup();
        pinMode(21,OUTPUT);
        pinMode(22,OUTPUT);
        pinMode(23,OUTPUT);
        pinMode(24,OUTPUT);

   
    Setup(argc,argv,Camera); // argc and argc are the command line argument
    cout<<"Connecting to camera"<<endl;
    if (!Camera.open())
    {
	 // if camera is not working then , below message will be print	
         cout<<"Failed to Connect"<<endl;
	 return -1;
    }
     
    cout<<"Camera Id = "<<Camera.getId()<<endl;
    while(1)
    {   
       auto start = std::chrono::system_clock::now(); // excution start and stored the value in start variable
       
       Capture();
       Perspective();
       Threshold();
       Histrogram();
       LaneFinder();
       LaneCenter();
       
    //uturn implemented 
    
    if (laneEnd > 15000)
     {
       	digitalWrite(21, 1);
	digitalWrite(22, 1);    //decimal = 7
	digitalWrite(23, 1);
	digitalWrite(24, 0);
	cout<<"Lane End"<<endl;
     }
    
    //uturn close
       
    if(Result==0)
    {
         digitalWrite(21,0);
         digitalWrite(22,0); // decimal =0
         digitalWrite(23,0);
         digitalWrite(24,0);
         cout<<"Go Forward"<<endl;
    }
    else if(Result>0 && Result <10)
    {
         digitalWrite(21,1);
         digitalWrite(22,0); // decimal =1
         digitalWrite(23,0);
         digitalWrite(24,0);
         cout<<"Go Right1"<<endl;
    }
    else if(Result>=10 && Result <20)
    {
         digitalWrite(21,0);
         digitalWrite(22,1); // decimal =2
         digitalWrite(23,0);
         digitalWrite(24,0);
         cout<<"Go Right2"<<endl;
    }
    else if(Result>20)
    {
         digitalWrite(21,1);
         digitalWrite(22,1); // decimal =3
         digitalWrite(23,0);
         digitalWrite(24,0);
         cout<<"Go Right3"<<endl;
    }



    else if(Result<0 && Result >-10)
    {
         digitalWrite(21,0);
         digitalWrite(22,0); // decimal =4
         digitalWrite(23,1);
         digitalWrite(24,0);
         cout<<"Go Left1"<<endl;
    }
    else if(Result<=-10 && Result >-20)
    {
         digitalWrite(21,1);
         digitalWrite(22,0); // decimal =5
         digitalWrite(23,1);
         digitalWrite(24,0);
         cout<<"Go Left2"<<endl;
    }
    else if(Result<-20)
    {
         digitalWrite(21,0);
         digitalWrite(22,1); // decimal =6
         digitalWrite(23,1);
         digitalWrite(24,0);
         cout<<"Go Left3"<<endl;
    }
    
    // laneEnd program
    if(laneEnd>15000)
    {
       ss.str(" ");
       ss.clear();
       ss<<" Lane End ";
       putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(0,255,255), 2);
	
    }
    
    else if(Result==0)
    {
       ss.str(" ");
       ss.clear();
       ss<<"Result = "<<Result<<"  (Move Forward )";
       putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(0,0,255), 2);
	
    }
    else if(Result> 0)
    {
       ss.str(" ");
       ss.clear();
       ss<<"Result = "<<Result<<"  (Move Right )";
       putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(0,0,255), 2);
	
    }
    
    else if(Result< 0)
    {
       ss.str(" ");
       ss.clear();
       ss<<"Result = "<<Result<<"  (Move Left )";
       putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(0,0,255), 2);
	
    }
       
       
      
     //LaneEnd program finished  
     
     
     
       // original image
       namedWindow("Original RGB",WINDOW_KEEPRATIO);
       moveWindow("Original RGB",0,100); // window size
       resizeWindow("Original RGB",640,480); // resolation size
       imshow("Original RGB",frame);// show the stored value in console
       
       
       // perspective image
       namedWindow("Perspective",WINDOW_KEEPRATIO);
       moveWindow("Perspective",640,100); // window size
       resizeWindow("Perspective",640,480); // resolation size
       imshow("Perspective",framePers);//   (show the stored value in console
       
       // Grapy scale image
       namedWindow("Final",WINDOW_KEEPRATIO);
       moveWindow("Final",1280,100); // window size
       resizeWindow("Final",640,480); // resolation size
       imshow("Final",frameFinal);// show the stored value in console
       
       
       
       waitKey(1);// delay for 1 ms
       
       auto end = std::chrono::system_clock::now(); // execution end and stored the value in end variable
       std::chrono::duration<double> elapsed_seconds = end-start; // total executaion time stored in elapsed_seconds variable
    
       float t = elapsed_seconds.count(); // execution time conver to user readable format and t variable contains total execution time 
       int FPS = 1/t; // FPS- frame per second variable contains total frame 
       cout<<"FPS = "<<FPS<<endl; // print total frame
       
       
    }
    return 0;
}
