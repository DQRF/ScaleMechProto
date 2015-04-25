/*

Written by Marcus Murray - December 2nd, 2014

Proof of concept for larger project: ScaleMech
Uses pixel color data to recognize most prominent pixels within bounding box
Then loads appropriate model when user presses load key ('f')

Uses AssimpModelLaoder, ofxGUI, and ofxOpenCV addons.
*ofxOBJModel is a dated addon which is not required for this project.

*/


#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetWindowShape(1420, 800);
    ofSetFrameRate(60);
    ofSetBackgroundAuto(true);
    ofBackground(0);
	
	ofSetLogLevel(OF_LOG_VERBOSE);
    ofBackground(50, 0);


	//////////////////////////////////////////////////////////////////////////////////////////
	//Model Setup
	//////////////////////////////////////////////////////////////////////////////////////////

		ofDisableArbTex(); // we need GL_TEXTURE_2D for our models coords.
		bAnimate = false; //DON'T PLAY ANIMATIONS FROM THE START

		model.loadModel("Takanuva.dae", true);	//Starting Model
		currentModel = 5;	//value for displaying the name of our current model

		model.setPosition(ofGetWidth() * .65, (float)ofGetHeight() * .9, 0);
			//Initial positions of mesh, right side of screen, near bottom, 0 depth;
		model.setLoopStateForAllAnimations(OF_LOOP_NONE);
			//turns off looping of animations for now. Will activate on button press
		if(!bAnimate)	model.setPausedForAllAnimations(true);
			//pauses all our animations
		autoRotate = true;
			//automatically rotates models at start

		//Array of our models
		modelArray[0] = "gangnam_style.dae";			//Green, Lewa
		modelArray[1] = "back_flip_to_uppercut_1.dae";	//Red, Tahu
		modelArray[2] = "Pohatu.dae";					//Brown, Pohatu. Not animated
		modelArray[3] = "Nokama@idle.dae";				//Blue, Nokama
		modelArray[4] = "Takanuva.dae";					//White, Takanuva. Not animated. Default model, cannot load after loading other model.
		modelArray[5] = "astroBoy_walk.dae";			//None, Default. Not currently loadable

	//////////////////////////////////////////////////////////////////////////////////////////
	//CV Setup
	//////////////////////////////////////////////////////////////////////////////////////////

		grabber.setDeviceID(0);             //grab our first cam device
		grabber.setDesiredFrameRate(30);    //set framerate for cam
		grabber.initGrabber(640, 360);      //set our incoming size, shrunken to accomodate for meshes.

		boundingBox.setWidth(grabber.width *.5); //Define dimensions for our bounding box
		boundingBox.setHeight(grabber.height);	//Designed for reading the area specified
		boundingBox.setX(grabber.width * .5);	//Within our grabber window.

		maxPixels = boundingBox.width * boundingBox.height;

		image.allocate(grabber.width, grabber.height);  //allocate our CV ColorImage with w/h based off cam
			image.setROI(boundingBox); //Changes Range of Interest
		greyImage.allocate(grabber.width, grabber.height);  //same for other CV Images
			greyImage.setROI(boundingBox);	//Changes Range of Interest
			greyImage.setROI(boundingBox);	//Changes Range of Interest
		greyBackground.allocate(grabber.width, grabber.height); //same
			greyBackground.setROI(boundingBox);	//Changes Range of Interest
		greyProcessed.allocate(grabber.width,grabber.height);   //same
			greyProcessed.setROI(boundingBox);	//Changes Range of Interest

		hueSave.allocate(grabber.width, grabber.height);	//Allocation for HueImage
			hueSave.setROI(boundingBox);					//Changes Range of Interest
		satSave.allocate(grabber.width, grabber.height);	//Allocation for SaturationImage
			satSave.setROI(boundingBox);					//Changes Range of Interest
		briSave.allocate(grabber.width, grabber.height);	//Allocation for BrightinessImage, might not be used
			briSave.setROI(boundingBox);					//Changes Range of Interest

		learnBackground = false;    //saves background for processed image display
   
	/////////////////////////////////////////////////////////////////////////
	//gui setup
	/////////////////////////////////////////////////////////////////////////

		lBKButton.addListener(this, &ofApp::bkToggler);    //setup a listener on our learn background button to toggle it's state momentarily
		guiStatus = true;   //set gui status to true
    
		gui.setup();    //setup the ofxPanel
		gui.setPosition(10, grabber.height + 5);	//Changes our initial starting position

		gui.add(lBKButton.setup("Learn Background"));
		gui.add(showProcessed.setup("Processed Image",true));	//toggle for our processed image
		gui.add(threshToggle.setup("Threshold",true));			//toggle for our threshold
		gui.add(threshSlider.setup("Threshold Amount",150,0,255));	//slider for our threshold

		gui.add(colorFinder.setup("ColorFinding", 90, 0, 180));
		gui.add(satFinder.setup("Saturation Threshold", 100, 0, 255));	//Determines threshold for saturation in color finding
		gui.add(showBG.setup("Show Background Image", true));


	//////////////////////////////////////////////////////////////////////////////////////////
	//Sound Setup
	//////////////////////////////////////////////////////////////////////////////////////////
	
		//MAKE SOME NOISE
		sound.loadSound("World Navigation.mp3");
		sound.play();
		sound.setLoop(true);

		backgroundImage.loadImage("hangar.png");
		backgroundImage.resize(1420, 800);
}



//--------------------------------------------------------------

void ofApp::update(){

	//////////////////////////////////////////////////////////////
	//Model stuff
	//////////////////////////////////////////////////////////////
	
		model.update();	//basic model update function

		//Currently disabled: Change scrubber in animation based on mouse position
		//if(bAnimateMouse)
		//	model.setPositionForAllAnimations(animationPosition);

		mesh = model.getCurrentAnimatedMesh(0);	//Determines number of animations tied to model

	//////////////////////////////////////////////////////////////
	//CV stuff
	//////////////////////////////////////////////////////////////


		//colorFinder.backgroundColor();
		
		grabber.update();   //pull our next frame of video
		image.setROI(boundingBox);	//Sets the Range of Interest for our image
		image.setFromPixels(grabber.getPixelsRef());    //set our CV Color Image to have the same data as the current frame

		greyImage = image;  //convert our color image to grayscale
    
		//change the background comparision baseline if learnBackground is true
		if(learnBackground){
			greyBackground = greyImage;     //set the background to the current frame
			learnBackground = false;        //toggle back to false
		}
    
		greyDiff.setROI(boundingBox);	//sets Range of Interest for our gray images
		greyProcessed.setROI(boundingBox);	//Same as above
		greyDiff.absDiff(greyBackground, greyImage);    //do a comparison between our current background ref and current frame and store result in greyDiff
    
		greyProcessed = greyDiff;  //set our greyProcessed to be our greyDiff
    
		if(threshToggle)	greyProcessed.threshold(threshSlider);   //threshold our image
 
		imageColor = image;				//Duplicates stored image
			imageColor.setROI(boundingBox);
		imageColor.convertRgbToHsv();	//Takes duplicate, changes to HSV
		imageColor.convertToGrayscalePlanarImages(hueSave, satSave, briSave); //Saves HSV to our color image instead of RGB

		hueSave.flagImageChanged();
		satSave.flagImageChanged();
		briSave.flagImageChanged();

		//Saves the data of our hue, saturation, and brightness pixels
		unsigned char * huePixels = hueSave.getPixels();
		unsigned char * satPixels = satSave.getPixels();
		unsigned char * briPixels = briSave.getPixels();

		//Determines what colored items to look for
		for (int i = 0; i < (grabber.width * grabber.height); i++) {

			//Our lighting thresholds, dark lighting currently not necessary
			if(lightSetting){
				greenThreshold = 95;
				redThreshold = 90;
				blueThreshold = 120;
				brownThreshold = 20;
			}else{
				greenThreshold = colorFinder;
				redThreshold = 90;
				blueThreshold = 120;
				brownThreshold = 20;
			}

			//Not a series of conditionals 
			//LOOKS FOR GREEN		
			if( ((huePixels[i] > (greenThreshold - 15)) && (huePixels[i] <= (greenThreshold + 15)))  && (satPixels[i] >= satFinder))				
				{greenPixelCount++;	}

			//LOOKS FOR RED
			else if( ((huePixels[i] > (redThreshold + 80)) || (huePixels[i] <= (redThreshold - 80)))  && (satPixels[i] >= satFinder))				
				{redPixelCount++;	}

			//LOOKS FOR BROWN/YELLOW
			else if( ((huePixels[i] > (brownThreshold - 10)) && (huePixels[i] <= (brownThreshold + 10)))   && (satPixels[i] >= (satFinder) )) // + brownOffset
				{brownPixelCount++;	}

			//LOOKS FOR BLUE
			else if( ((huePixels[i] > (blueThreshold - 10)) && (huePixels[i] <= (blueThreshold + 10))) && (satPixels[i] >= satFinder))
				{bluePixelCount++;	}

			//LOOKS FOR WHITE
			else if( (briPixels[i] > 80) && (satPixels[i] >= satFinder))
				{whitePixelCount++;	}
		}

		pixelCount[0] = greenPixelCount;
		pixelCount[1] = redPixelCount;
		pixelCount[2] = brownPixelCount;
		pixelCount[3] = bluePixelCount;
		pixelCount[4] = whitePixelCount;
	
		holdMax = 0;

		for(int i = 0; i < 4; i++){
			if(pixelCount[i] > holdMax){
				holdMax = pixelCount[i];
				tempMax = i;
			}
		}

		greenPixelCount = redPixelCount = brownPixelCount = bluePixelCount = 0;
}

//////////////////////////////////////////////////////////////////



void ofApp::draw(){

	//Resizes and shows BG image if toggle is true
	if(showBG){
		backgroundImage.resize(ofGetWindowWidth(), ofGetWindowHeight());
		backgroundImage.draw(0,0);
	}

	////////////////////////////////////////////////////////////////////
	//Model stuff
	////////////////////////////////////////////////////////////////////

		//sets up model display option
		ofSetColor(255);
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		ofEnableDepthTest();

		glShadeModel(GL_SMOOTH); //some model / light stuff
		light.enable();
		ofEnableSeparateSpecularLight();

		ofPushMatrix();
			ofTranslate(model.getPosition().x+200, model.getPosition().y, 0);
			if(!autoRotate) ofRotate(-mouseX, 0, 1, 0);	//Rotates the model based on mouse
			else{
				ofRotate(rotAngle, 0, 1, 0);	//automatic rotation
				rotAngle += .1;
			}
			ofTranslate(-model.getPosition().x, -model.getPosition().y, 0);
			model.drawFaces();	//CAN'T SEE WITHOUT A FACE
		ofPopMatrix();



	   if(ofGetGLProgrammableRenderer()){
			glPushAttrib(GL_ALL_ATTRIB_BITS);
			glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
		}
		glEnable(GL_NORMALIZE);


		if(showSkeleton){
			ofPushMatrix();
				ofTranslate(model.getPosition().x-300, model.getPosition().y, 0);
				if(!autoRotate) ofRotate(-mouseX, 0, 1, 0);	//Rotates the model based on mouse
				else{
					ofRotate(rotAngle, 0, 1, 0);
					rotAngle += .1;
				}
				ofTranslate(-model.getPosition().x, -model.getPosition().y, 0);
    
				//Mesh Helper, does something that allows us to get our mesh matrix
				ofxAssimpMeshHelper & meshHelper = model.getMeshHelper(0);
				ofMultMatrix(model.getModelMatrix());
				ofMultMatrix(meshHelper.matrix);
    
				ofMaterial & material = meshHelper.material;
				if(meshHelper.hasTexture())   meshHelper.getTextureRef().bind();

				//Draws our material onto the wireframe
				material.begin();
				mesh.drawWireframe();
				material.end();

				if(meshHelper.hasTexture())	meshHelper.getTextureRef().unbind();
			ofPopMatrix();
		}

		//Sets upper bound for the angle of rotation of model
		if(rotAngle > 360) rotAngle = 0;

		if(ofGetGLProgrammableRenderer())	glPopAttrib();
    
		//turns off our light
		ofDisableDepthTest();
		light.disable();
		ofDisableLighting();
		ofDisableSeparateSpecularLight();
    
		ofSetColor(255, 255, 255);	//Sets text color to white
	
		//Displays the currently most populated pixel color within the grabber window
		switch(tempMax){
			case 0: currentMax = "green"; break;
			case 1: currentMax = "red"; break;
			case 2: currentMax = "brown"; break;
			case 3:	currentMax = "blue"; break;
		}

		//Displays information for current model
		switch(currentModel){
			case 0: captureSource = "It Seez it 3D"; break; 
			case 1: captureSource = "Structure Scanner"; break;
			case 2: captureSource = "Structure Scanner"; break;
			case 3: captureSource = "123D Catch"; break;
			case 4: captureSource = "Structure Scanner"; break;
			case 5: captureSource = "Default"; break;
		}

		const int displayArraySize = 16;
		int displayLine[displayArraySize];
		displayLine[0] = 560;

		for(int i = 1; i < displayArraySize; i++)	displayLine[i] = displayLine[i-1] + 15;

		//Prints things to the UI, doesn't display if gui is hidden
		if(guiStatus){
			ofDrawBitmapString("Place a model in front of the camera, filling the bounding box as much as possible,", 10, displayLine[0]);
			ofDrawBitmapString("and press 'f' to check scan. Press spacebar to trigger animation, 'g' to toggle image.", 10, displayLine[1]);
			ofDrawBitmapString("Press 'a' to auto rotate, 's' to toggle light settings, 'd' to toggle skeleton., 'h' to hide info", 10, displayLine[2]);
			ofDrawBitmapString("Current Model: " + modelArray[currentModel] + ", captured with: " + captureSource, 10, displayLine[3]);
			ofDrawBitmapString("num animations for this model: " + ofToString(model.getAnimationCount()), 10, displayLine[4]);

			ofDrawBitmapString("Hue at mouse point: " + ofToString(getHue), 10, displayLine[6]);
			ofDrawBitmapString("Current max color: " + currentMax, 10, displayLine[7]);
			ofDrawBitmapString("greenPixels: "+ofToString(pixelCount[0], 2), 10, displayLine[8]);
			ofDrawBitmapString("redPixels: "+ofToString(pixelCount[1], 2), 10, displayLine[9]);
			ofDrawBitmapString("brownPixels: "+ofToString(pixelCount[2], 2), 10, displayLine[10]);
			ofDrawBitmapString("bluePixels: "+ofToString(pixelCount[3], 2), 10, displayLine[11]);
			ofDrawBitmapString("Total Pixels in bounding box: " + ofToString(maxPixels, 2), 10, displayLine[12]);

			if(lightSetting) ofDrawBitmapString("Lighting set to Darker Settings.", 10, displayLine[14]);
			else			ofDrawBitmapString("Lighting set to Lab Settings. ", 10, displayLine[14]);

			ofDrawBitmapString("fps: "+ofToString(ofGetFrameRate(), 2), 10, displayLine[15]);
		}


	///////////////////////////////////////////////////////////////////
	//Cv stuff
	///////////////////////////////////////////////////////////////////

		//Displays everything if true, only model and bg if false
		if(guiStatus){
			if (showProcessed)   image.draw(0,0);    //draw our color image
			else  greyProcessed.draw(0,0);    //draw our processed greyscale image

			ofPushMatrix();
				ofSetColor(255);	//sets bounding box to white
				ofNoFill();			//border only
				ofRect(boundingBox.x, boundingBox.y, boundingBox.width, boundingBox.height);
			ofPopMatrix();
		}
    
		//toggle gui and other information
		if(guiStatus)  gui.draw();     //draw our gui if toggled on
}

//////////////////////////////////////////////////////////////////
//function connected to the lBKToggle button
//////////////////////////////////////////////////////////////////

void ofApp::bkToggler(){
    learnBackground = true;     //if mouse is pressed then grab a new background plate
}

//////////////////////////////////////////////////////////////////

void ofApp::keyPressed(int key){
    ofPoint modelPosition(ofGetWidth() * 0.55, (float)ofGetHeight() * 0.95, -40);

	switch (key){
		case 'x': sound.setPaused(true); break;  
		case 'c': sound.setPaused(false); break;  
		case 'f': valueCheck(); break;
		case 'h': guiStatus = !guiStatus; break;
		case 'r': grabRed = !grabRed; break;
		case 'a': autoRotate = !autoRotate; rotAngle = model.getRotationAngle(1); break;
		case 's': lightSetting = !lightSetting; break;
		case 'd': showSkeleton = !showSkeleton; break;
		case 'g': showBG = !showBG; break;
		case ' ':
			bAnimate = !bAnimate;
			break;
		default:
			break;		
    }

	mesh = model.getMesh(0);	//Gets the wireframe of our model

    model.setLoopStateForAllAnimations(OF_LOOP_NORMAL);	//all animations loop by defaults
    model.playAllAnimations();	//plays every animation attached to mesh
    if(!bAnimate)  model.setPausedForAllAnimations(true);	//pauses based on keypress of space
}


//////////////////////////////////////////////////////////////////
//	Function that determines color values and displays models
//////////////////////////////////////////////////////////////////

void ofApp::valueCheck(){
	ofPoint modelPosition(ofGetWidth() * 0.55, (float)ofGetHeight() * 0.9);

	int max = 0;

	//pixelCount.size() not working for some reason
	//Upper bound currently set to four to ignore white pixels
	for(int i = 0; i < 4; i++){
		if(pixelCount[i] > max) max = pixelCount[i];
	}
	
	//Runs through our model array and loads the appropriate one based on the highest pixel count
	for(int j = 0; j < 4; j++){
		if(pixelCount[j] == max){
			model.loadModel(modelArray[j]);
			currentModel = j;
			ofEnableSeparateSpecularLight();
			model.setPosition(modelPosition.x, modelPosition.y, modelPosition.z);
		}
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){}
//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){}
//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	if(button == 0){
		if(x < grabber.width && y < grabber.height){
			int point = (grabber.width * y) + x;
	
			//int point = (ofGetScreenWidth() * y) + x;

			unsigned char * huePixels = hueSave.getPixels();

			getHue = huePixels[point];
		}

		else ofDrawBitmapString("mousepoint not in grabber window", 10, 560);

		model.setPosition(x - 250, y + 400, 1);
	}
}
//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){}
//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){}
//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){}
//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){}
//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){}
