#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxGui.h"
#include "ofxAssimpModelLoader.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
	
	//Computer Vision Stuff
		ofVideoGrabber grabber; //VideoGrabber for cam input
		ofxCvColorImage image;      //CV ColorImage for our cam input
		ofxCvColorImage imageColor;	//Second color image, currently unused?
		ofxCvGrayscaleImage greyImage;  //CV GreyscaleImage for conversion of our cam
		ofxCvGrayscaleImage greyBackground; //CV GreyscaleImage to store the background
		ofxCvGrayscaleImage greyDiff;       //CV GreyscaleImage to store the difference between the greyImage and greyBackground aka bk removal
		ofxCvGrayscaleImage greyProcessed;  //CV GreyscaleImage that is preprocessed prior to blob detection

		bool learnBackground;   //bool to set whether or not to look for a new background plate
		void bkToggler();       //function to toggle the background
   
		ofxCvContourFinder contourFinder;   //object for handling contour finding aka blobs

//////////////////////////////////////////////////////////////////////////////////


	//ColorGrabbing
		ofxCvGrayscaleImage hueSave;	//image for our hue
		ofxCvGrayscaleImage satSave;	//for our saturation
		ofxCvGrayscaleImage briSave;	//and for our brightness

		int maxPixels;	//maximum pixel color threshold.
		bool grabRed;	//when changed, sets the texture image
		
		int getHue;	//gets hue of one specified pixle point
		int holdMax;	//maximum pixel count
		int tempMax;	//temporary maximum pixel count
		string currentMax;	//Which color is the most prominent?

		//Keeps track of how many pixels are in the grabber. May need to alter hue values based on things.
		int pixelCount[5]; 
		int greenPixelCount;	//green
		int redPixelCount;		//red
		int bluePixelCount;		//blue
		int brownPixelCount;	//brown
		int brownOffset;	 //Raised saturation threshold for brown
		int whitePixelCount;	//white

		bool lightSetting;

		int redThreshold;	//default color range for our red
		int greenThreshold;	//for green
		int blueThreshold;	//for blue
		int brownThreshold;	//and brown

		ofRectangle boundingBox;	//Rectangular limitations for our color scan


//////////////////////////////////////////////////////////////////////////////////

	//Array for storing our models
		string modelArray[6];		
		ofxAssimpModelLoader model;

		bool bAnimate; //Placeholder for actual animations
		bool autoRotate;	//Determines whether model rotates on its own, or if it is based on mouse value
		float rotAngle;		//Angle of our rotation.
		bool showSkeleton;	//Determines whether or not we display our skeleton
	
		ofMesh mesh;	//The actual mesh of the model
		ofLight light;	//Light up our scene, or we can't see anything!
		int currentModel;	//The number of our displayed model
		string captureSource;	//String for displaying the way we captured our models

		void valueCheck();	//Function for scanning/checking color values and loading models
		

//////////////////////////////////////////////////////////////////////////////////

    //GUI Stuff
		ofxPanel gui ;			    //ofxPanel object for gui objects
		ofxButton lBKButton;	    //learn background Button
		ofxToggle showProcessed;    //toggle to show our RGB or Processed Image
		ofxToggle blurToggle;		//toggle for blur
		ofxToggle showBG;			//toggle for displaying our BG image
		ofxToggle threshToggle;     //thresholding toggle
		ofxIntSlider threshSlider;  //slider for setting threshold amount
		ofxIntSlider colorFinder;	//slider for determining which color to find

		ofxIntSlider satFinder;		//slider for determing
		ofxFloatSlider meshTestSlider;

		bool guiStatus;     //bool to toggle the gui draw
		ofImage backgroundImage;
		ofSoundPlayer sound;
};
