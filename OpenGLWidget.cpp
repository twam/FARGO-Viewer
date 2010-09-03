#include "OpenGLWidget.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <QWheelEvent>

typedef struct
{
	GLubyte *imageData;
	GLuint bpp;
	GLuint width;
	GLuint height;
	GLuint textureID;
} TextureImage;

TextureImage textures[1];

/**
	loads a TGA file into memory 
*/
bool loadTGA(TextureImage *texture, const char *filename)
{
	// Uncompressed TGA Header
	GLubyte TGAheader[12]={0,0,2,0,0,0,0,0,0,0,0,0};
	// Used To Compare TGA Header
	GLubyte TGAcompare[12];
	// First 6 Useful Bytes From The Header
	GLubyte header[6];
	// Holds Number Of Bytes Per Pixel Used In The TGA File
	GLuint bytesPerPixel;
	// Used To Store The Image Size When Setting Aside Ram
	GLuint imageSize;
	GLuint temp;
	// Set The Default GL Mode To RBGA (32 BPP)
	GLuint type=GL_RGBA;

	FILE *file = fopen(filename, "rb");

	if (file==NULL ||
		fread(TGAcompare,1,sizeof(TGAcompare),file)!=sizeof(TGAcompare) ||
		memcmp(TGAheader,TGAcompare,sizeof(TGAheader))!=0 ||
		fread(header,1,sizeof(header),file)!=sizeof(header)) {
		if (file == NULL) {
			return false;
		} else {
			fclose(file);
			return false;
		}
	}

	// Determine The TGA Width (highbyte*256+lowbyte)
	texture->width  = header[1] * 256 + header[0];
	// Determine The TGA Height (highbyte*256+lowbyte)
	texture->height = header[3] * 256 + header[2];

 	if (texture->width <=0 || texture->height <=0 || (header[4]!=24 && header[4]!=32)) {
		fclose(file);
		return false;
	}

	// Grab The TGA's Bits Per Pixel (24 or 32)
	texture->bpp = header[4];
	// Divide By 8 To Get The Bytes Per Pixel
	bytesPerPixel = texture->bpp/8;
	imageSize = texture->width*texture->height*bytesPerPixel;

	// Reserve Memory To Hold The TGA Data
	texture->imageData=(GLubyte *)malloc(imageSize);

	if(	texture->imageData==NULL ||							// Does The Storage Memory Exist?
		fread(texture->imageData, 1, imageSize, file)!=imageSize)	// Does The Image Size Match The Memory Reserved?
	{
		if(texture->imageData!=NULL)						// Was Image Data Loaded
			free(texture->imageData);						// If So, Release The Image Data

		fclose(file);										// Close The File
		return false;										// Return False
	}

	// Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)
	for(GLuint i=0; i < imageSize; i+=bytesPerPixel) {
		temp=texture->imageData[i];
		texture->imageData[i] = texture->imageData[i + 2];
		texture->imageData[i + 2] = temp;
	}

	fclose (file);

	// generate OpenGL texture ID
	glGenTextures(1, &texture[0].textureID);

	// bind our textur
	glBindTexture(GL_TEXTURE_2D, texture[0].textureID);
	// linear filtered
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// linear filtered
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// // Was The TGA 24 Bits
	if (texture[0].bpp==24) {
		// If So Set The 'type' To GL_RGB
		type=GL_RGB;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, type, texture[0].width, texture[0].height, 0, type, GL_UNSIGNED_BYTE, texture[0].imageData);
	glBindTexture(GL_TEXTURE_2D, 0);

	//Texture Building Went Ok, Return True
	return true;
}

OpenGLWidget::OpenGLWidget(QWidget *parent)
: QGLWidget(QGLFormat(QGL::SampleBuffers), parent), simulation(NULL)
{
	diskVertices = NULL;
	diskNormals = NULL;
	diskColors = NULL;
	diskIndices = NULL;

	orbitsVertices = NULL;
	orbitsDetailLevel = 128;

	diskBorderVertices = NULL;
	diskBorderDetailLevel = 128;

	skyVertices = NULL;
	skyDistance = 300.0;
	skyNumberOfObjects = 1000;

	resetCamera();
}

OpenGLWidget::~OpenGLWidget()
{
	// clean up disk data
	delete [] diskIndices;
	delete [] diskVertices;
	delete [] diskColors;
	delete [] diskNormals;

	// clean up orbit data
	delete [] orbitsVertices;

	// clean up sky data
	delete [] skyVertices;
}

void OpenGLWidget::resetCamera()
{
	cameraPositionX = 0.0;
	cameraPositionY = 0.0;
	cameraPositionZ = -50.0;

	cameraRotationX =0.0;
	cameraRotationY = 0.0;
	cameraRotationZ = 0.0;	
}

void OpenGLWidget::setSimulation(Simulation* simulation)
{
	this->simulation = simulation;

	initDisk();
	initDiskBorder();
	initOrbits();

	resetCamera();

	update();
}

GLboolean checkExtension(const char *extName)
{
	/*
	** Search for extName in the extensions string.  Use of strstr()
	** is not sufficient because extension names can be prefixes of
	** other extension names.  Could use strtok() but the constant
	** string returned by glGetString can be in read-only memory.
	*/
	char *p = (char *)glGetString(GL_EXTENSIONS);
	char *end;
	int extNameLen;

	extNameLen = strlen(extName);
	end = p + strlen(p);

	while (p < end) {
		int n = strcspn(p, " ");
		if ((extNameLen == n) && (strncmp(extName, p, n) == 0)) {
			return GL_TRUE;
		}
		p += (n + 1);
	}

	return GL_FALSE;
}



void OpenGLWidget::initializeGL()
{
	supportMultisampling = checkExtension("GL_ARB_multisample");

	glEnable(GL_DEPTH_TEST);
	
//	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
//	glEnable(GL_COLOR_MATERIAL);

	GLfloat global_ambient[] = {1.0f, 1.0f, 1.0f, 1.0f};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

//	glEnable(GL_LIGHTING);

	GLfloat diffuse[] = {0.8f, 0.8f, 0.8f , 1.0f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	GLfloat position[] = {0.0f, 0.0f, 24.0f, 1.0f};
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glEnable(GL_LIGHT0);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_NORMALIZE);
	glEnable(GL_BLEND);

	glShadeModel(GL_SMOOTH);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glDepthFunc(GL_LEQUAL);
	
	initSky();

	glEnable(GL_TEXTURE_2D);
	loadTGA(&textures[0],"sun.tga");
}

void OpenGLWidget::resizeGL(int width, int height)
{
	// setup viewport
	glViewport(0, 0, (GLint)width, (GLint)height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, (GLfloat)width/(GLfloat)height, 0.1f, 500.0f);
	glMatrixMode(GL_MODELVIEW);
}

void OpenGLWidget::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		last_drag_pos = event->pos();
	}

	if (event->button() == Qt::RightButton) {
		last_drag_pos = event->pos();
	}
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
	if (event->buttons() & Qt::LeftButton) {
		cameraPositionX += (GLdouble)(event->pos().x() - last_drag_pos.x()) / 50.0;
		cameraPositionY -= (GLdouble)(event->pos().y() - last_drag_pos.y()) / 50.0;
		last_drag_pos = event->pos();
		update();
	}

	if (event->buttons() & Qt::RightButton) {
		cameraRotationY += 2.0 * (event->pos().x() - last_drag_pos.x());
		cameraRotationX += 2.0 * (event->pos().y() - last_drag_pos.y());
		last_drag_pos = event->pos();
		update();
	}
}

void OpenGLWidget::wheelEvent(QWheelEvent *event)
{
	cameraPositionZ += (GLdouble)(event->delta()/8)/360.0*20.0;
	update();
}

void OpenGLWidget::renderPlanets()
{
	GLfloat no_mat[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	GLfloat mat_emission[4] = {0.8f, 0.8f, 0.8f, 1.0f};

	if (simulation == NULL)
		return;

	glEnable(GL_LIGHTING);
	for (unsigned int i = 0; i < simulation->getNumberOfPlanets(); ++i) {
		glPushMatrix();
		glTranslated(simulation->getPlanetPosition(i)[0],simulation->getPlanetPosition(i)[1],simulation->getPlanetPosition(i)[2]);
		GLUquadricObj* quadric = gluNewQuadric();
		//glBindTexture(GL_TEXTURE_2D, textures[0].textureID);
		gluQuadricTexture(quadric,true);
		glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
		gluSphere(quadric,0.25,32,32);
		glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);
		gluDeleteQuadric(quadric);
		glBindTexture(GL_TEXTURE_2D, 0);
		glPopMatrix();
	}
	glDisable(GL_LIGHTING);
}

void OpenGLWidget::initOrbits()
{
	if (this->simulation == NULL)
		return;

	// we need 3 coordinates for each point of each planet (except for central star)
	delete [] orbitsVertices;
	orbitsVertices = new GLfloat[3*orbitsDetailLevel*(simulation->getNumberOfPlanets()-1)];
}

void OpenGLWidget::renderOrbits()
{
	if (simulation == NULL)
		return;

	glEnable(GL_LINE_SMOOTH);

	for (unsigned int i = 1; i < simulation->getNumberOfPlanets(); ++i) {
		glPushMatrix();

		double x = simulation->getPlanetPosition(i)[0];
		double y = simulation->getPlanetPosition(i)[1];
		double v_x = simulation->getPlanetVelocity(i)[0];
		double v_y = simulation->getPlanetVelocity(i)[1];
		double mass = simulation->getPlanetMass(i)[0];

		// angular momentum
		double j = mass * x * v_y - mass * y * v_x;
		// distance
		double d = sqrt(pow2(x)+pow2(y));
		// Runge-Lenz vector A = (p x L) - m * G * m * M * r/|r|;
		double A_x =  j/mass * (1.0+mass) * v_y - 1.0 * 1.0 * pow2(1.0+mass) * x/d; 
		double A_y = -j/mass * (1.0+mass) * v_x - 1.0 * 1.0 * pow2(1.0+mass) * y/d;
		double eccentricity = sqrt(pow2(A_x) + pow2(A_y))/(1.0*1.0*pow2(1.0+mass));
	        double semi_major_axis = pow2(j/mass) / (1.0 * (1.0+mass)) / (1.0 - pow2(eccentricity));
		double semi_minor_axis = semi_major_axis * sqrt(1.0 -pow2(eccentricity));

		for (unsigned int j = 0; j < orbitsDetailLevel; ++j) {
			orbitsVertices[((i-1)*3*orbitsDetailLevel)+3*j+0] = semi_major_axis*sin(2.0*M_PI/(float)orbitsDetailLevel*(float)j);
			orbitsVertices[((i-1)*3*orbitsDetailLevel)+3*j+1] = semi_minor_axis*cos(2.0*M_PI/(float)orbitsDetailLevel*(float)j);
			orbitsVertices[((i-1)*3*orbitsDetailLevel)+3*j+2] = 0.0;
		}

		glTranslated(eccentricity*semi_major_axis,0,0);
	
		glColor3ub(0x80,0x80,0x80);
		glBegin(GL_LINE_LOOP);
		for (unsigned int j = 0; j < orbitsDetailLevel; ++j) {
			glVertex3fv(&orbitsVertices[((i-1)*3*orbitsDetailLevel)+3*j]);
		}
		glEnd();

		glPopMatrix();
	}

	glDisable(GL_LINE_SMOOTH);
}

void OpenGLWidget::initDisk()
{
	if (simulation == NULL)
		return;

	unsigned int index = 0;

	delete [] diskVertices;
	diskVertices = new GLfloat[3*((simulation->getNRadial()+1)*simulation->getNAzimuthal())];
	delete [] diskNormals;
	diskNormals = new GLfloat[3*((simulation->getNRadial()+1)*simulation->getNAzimuthal())];
	delete [] diskIndices;
	diskIndices = new GLuint[4*((simulation->getNRadial())*simulation->getNAzimuthal())];
	delete [] diskColors;
	diskColors = new GLfloat[4*((simulation->getNRadial()+1)*simulation->getNAzimuthal())];

	for (unsigned int nRadial = 0; nRadial <= simulation->getNRadial(); ++nRadial) {
		for (unsigned int nAzimuthal = 0; nAzimuthal < simulation->getNAzimuthal(); ++nAzimuthal) {
			index =  nRadial * simulation->getNAzimuthal() + nAzimuthal;
			diskVertices[3*index+0] = simulation->getRadii()[nRadial]*cos(2.0*M_PI/simulation->getNAzimuthal()*nAzimuthal);
			diskVertices[3*index+1] = simulation->getRadii()[nRadial]*sin(2.0*M_PI/simulation->getNAzimuthal()*nAzimuthal);
			diskVertices[3*index+2] = 0;
		}
	}

	// set indices (tells open gl which vertices belong to a quad)
	for (unsigned int nRadial = 0; nRadial < simulation->getNRadial(); ++nRadial) {
		for (unsigned int nAzimuthal = 0; nAzimuthal < simulation->getNAzimuthal(); ++nAzimuthal) {
			index = nRadial * simulation->getNAzimuthal() + nAzimuthal;
			diskIndices[4*index+0] = index;
			diskIndices[4*index+1] = index+simulation->getNAzimuthal();
			if (nAzimuthal == simulation->getNAzimuthal()-1) {
				diskIndices[4*index+2] = index+1;
				diskIndices[4*index+3] = index+1-simulation->getNAzimuthal();
			} else {
				diskIndices[4*index+2] = index+simulation->getNAzimuthal()+1;
				diskIndices[4*index+3] = index+1;
			}
		}
	}

	// normals
	for (unsigned int nRadial = 0; nRadial <= simulation->getNRadial(); ++nRadial) {
		for (unsigned int nAzimuthal = 0; nAzimuthal < simulation->getNAzimuthal(); ++nAzimuthal) {
			index =  nRadial * simulation->getNAzimuthal() + nAzimuthal;
			diskNormals[3*index+0] = 0.0;
			diskNormals[3*index+1] = 0.0;
			diskNormals[3*index+2] = 1.0;
		}
	}

	// colors
	for (unsigned int nRadial = 0; nRadial <= simulation->getNRadial(); ++nRadial) {
		for (unsigned int nAzimuthal = 0; nAzimuthal < simulation->getNAzimuthal(); ++nAzimuthal) {
			index =  nRadial * simulation->getNAzimuthal() + nAzimuthal;
			diskColors[4*index+0] = 1.0;
			diskColors[4*index+1] = 1.0;
			diskColors[4*index+2] = 1.0;
			diskColors[4*index+3] = 1.0;
		}
	}
}

void OpenGLWidget::renderDisk()
{
	if (simulation == NULL)
		return;

	unsigned int index;

	// set colors
	for (unsigned int nRadial = 0; nRadial <= simulation->getNRadial(); ++nRadial) {
		for (unsigned int nAzimuthal = 0; nAzimuthal < simulation->getNAzimuthal(); ++nAzimuthal) {
			index =  nRadial * simulation->getNAzimuthal() + nAzimuthal;
			diskColor(&diskColors[4*index], simulation->getDensity()[index], 100, 10000, true);
		}
	}

	glPushMatrix();
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, diskVertices);
	glNormalPointer(GL_FLOAT, 0, diskNormals);
	glColorPointer(4, GL_FLOAT, 0, diskColors);
	glDrawElements(GL_QUADS, 4*((simulation->getNRadial())*simulation->getNAzimuthal()), GL_UNSIGNED_INT, diskIndices);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glPopMatrix();
}

void OpenGLWidget::diskColor(GLfloat* color, double value, double minValue, double maxValue, bool logarithmic)
{
	if (logarithmic == false) {
		value = min(value, maxValue);
		value = max(value, minValue);
		value = value/(maxValue-minValue);
	} else {
		if (minValue == 0)
			minValue = DBL_EPSILON;
		maxValue = log(maxValue)/log(10);
		minValue = log(minValue)/log(10);
		value = log(value)/log(10);
		value = min(value, maxValue);
		value = max(value, minValue);
		value = value/(maxValue-minValue);
		value = log(value)/log(10);
	}

	if (value < 3.0/5.0) {
		value *= 5.0/3.0;
		color[0] = 1.0;
		color[1] = 0; 
		color[2] = 0;
		color[3] = value*1.0;
	} else if (value < 4.0/5.0) {
		value -= 3.0/5.0;
		value *= 5.0/1.0;
		color[0] = 1.0;
		color[1] = value*0.5;
		color[2] = 0;
		color[3] = 1.0;
	} else {
		value -= 4.0/5.0;
		value *= 5.0/1.0;
		color[0] = 1.0;
		color[1] = 0.5+value*0.5;
		color[2] = value*1.0;
		color[3] = 1.0;
	}
}

void OpenGLWidget::initDiskBorder()
{
	if (this->simulation == NULL)
		return;

	// we need 3 coordinates for each point of each planet (except for central star)
	delete [] diskBorderVertices;
	diskBorderVertices = new GLfloat[3*diskBorderDetailLevel*2];
}

void OpenGLWidget::renderDiskBorder()
{
	double r;

	if (simulation == NULL)
		return;
	
	glEnable(GL_LINE_SMOOTH);

	for (unsigned int i = 0; i < 2; ++i) {
		glPushMatrix();

		if (i == 0) {
			r = simulation->getRMin();
		} else {
			r = simulation->getRMax();
		}

		for (unsigned int j = 0; j < orbitsDetailLevel; ++j) {
			diskBorderVertices[i*3*orbitsDetailLevel+3*j+0] = r*sin(2.0*M_PI/(float)orbitsDetailLevel*(float)j);
			diskBorderVertices[i*3*orbitsDetailLevel+3*j+1] = r*cos(2.0*M_PI/(float)orbitsDetailLevel*(float)j);
			diskBorderVertices[i*3*orbitsDetailLevel+3*j+2] = 0.0;
		}

		glColor3ub(0x80,0x80,0x80);
		glBegin(GL_LINE_LOOP);
		for (unsigned int j = 0; j < orbitsDetailLevel; ++j) {
			glVertex3fv(&diskBorderVertices[i*3*diskBorderDetailLevel+3*j]);
		}
		glEnd();

		glPopMatrix();
	}

	glDisable(GL_LINE_SMOOTH);	
}

void OpenGLWidget::initSky()
{
	float phi, theta;

	delete [] skyVertices;
	skyVertices = new GLfloat[3*skyNumberOfObjects];

	for (unsigned int i = 0; i < skyNumberOfObjects; ++i) {
		phi = 2.0*M_PI*rand()/(float)RAND_MAX;
		theta = 2.0*M_PI*rand()/(float)RAND_MAX;
		skyVertices[3*i+0] = skyDistance*cos(phi)*sin(theta);
		skyVertices[3*i+1] = skyDistance*sin(phi)*sin(theta);
		skyVertices[3*i+2] = skyDistance*cos(theta);
	}
}

void OpenGLWidget::renderSky()
{
	glEnable(GL_POINT_SMOOTH);

	glColor3f(1.0,1.0,1.0);
	glPointSize(1.0);
	glBegin(GL_POINTS);
	for (unsigned int i = 0; i < skyNumberOfObjects; ++i) {
		glVertex3fv(&skyVertices[3*i]);
	}
	glEnd();

	glDisable(GL_POINT_SMOOTH);
}

void OpenGLWidget::paintGL()
{
	if (useMultisampling && supportMultisampling) {
		glEnable(GL_MULTISAMPLE_ARB);
	}

	// clear view
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glClear(GL_COLOR_BUFFER_BIT);

	if (showSky) {
		// do camera rotation without translation, so stars appear with infinity distance
		glLoadIdentity();
		glRotated(cameraRotationY, 0.0, 1.0, 0.0);
		glRotated(cameraRotationX, 1.0, 0.0, 0.0);
		renderSky();
	}

	// setup camera view
	glLoadIdentity();
	glTranslated(cameraPositionX, cameraPositionY, cameraPositionZ);
	glRotated(cameraRotationY, 0.0, 1.0, 0.0);
	glRotated(cameraRotationX, 1.0, 0.0, 0.0);

	if (showOrbits)
		renderOrbits();

	if (showPlanets)
		renderPlanets();

	if (showDisk)
		renderDisk();
	
	if (showDiskBorder)
		renderDiskBorder();

	if (showText) {
		if (simulation != NULL) {
			glColor3f(1.0,1.0,1.0);
			renderText(width()-120, 20, QString("Timestep: %1").arg(simulation->getCurrentTimestep()), QFont("Helvetica", 12, QFont::Bold) );
		}
	}

	if (useMultisampling && supportMultisampling) {
		glDisable(GL_MULTISAMPLE_ARB);
	}

	glFlush();

	if (saveScreenshots && (simulation != NULL)) {
		char temp[200];
		sprintf(temp, "/tmp/image_%i.jpg", simulation->getCurrentTimestep());
		QString filename = temp;
	
		// get image
		QImage image = grabFrameBuffer();
		image.save(filename, 0, 100);
	}
}

void OpenGLWidget::updateShowDisk(bool value)
{
	showDisk = value;
	update();
}

void OpenGLWidget::updateShowDiskBorder(bool value)
{
	showDiskBorder = value;
	update();
}

void OpenGLWidget::updateShowPlanets(bool value)
{
	showPlanets = value;
	update();
}

void OpenGLWidget::updateShowOrbits(bool value)
{
	showOrbits = value;
	update();
}

void OpenGLWidget::updateShowSky(bool value)
{
	showSky = value;
	update();
}

void OpenGLWidget::updateShowText(bool value)
{
	showText = value;
	update();
}


void OpenGLWidget::updateUseMultisampling(bool value)
{
	useMultisampling = value;
	update();
}

void OpenGLWidget::updateSaveScreenshots(bool value)
{
	saveScreenshots = value;
	update();
}