#include "OpenGLWidget.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <QWheelEvent>

GLuint textures[1];

OpenGLWidget::OpenGLWidget(QWidget *parent)
: OpenGLNavigationWidget(QGLFormat(QGL::SampleBuffers), parent), simulation(NULL)
{
	diskVertices = NULL;
	diskNormals = NULL;
	diskColors = NULL;
	diskIndices = NULL;

	orbitsVertices = NULL;
	orbitsDetailLevel = 128;

	rocheLobeVertices = NULL;
	rocheLobeDetailLevel = 256;
	
	diskBorderVertices = NULL;
	diskBorderDetailLevel = 128;

	skyVertices = NULL;
	skyDistance = 300.0;
	skyNumberOfObjects = 1000;

	palette = new Palette;
	palette->addColor(1,QColor(0x00,0x00,0x00,0x00));
	palette->addColor(3,QColor(0xFF,0x00,0x00,0xFF));
	palette->addColor(4,QColor(0xFF,0x80,0x00,0xFF));
	palette->addColor(5,QColor(0xFF,0xFF,0xFF,0xFF));

	setNormalMoveFactor(0.05);
	setFastMoveFactor(5*0.05);
	setNormalRotationFactor(1.0);
	setFastRotationFactor(5.0);
	setNormalZoomFactor(1.0);
	setFastZoomFactor(5.0);

	setCameraDefaultPosition(Vector<GLdouble, 3>(3,0.0,0.0,-50.0));
	setCameraDefaultLookAt(Vector<GLdouble, 3>(3,0.0,0.0,0.0));
	setCameraDefaultUp(Vector<GLdouble, 3>(3,0.0,1.0,0.0));

	resetCamera();

	minimumValue = 10;
	maximumValue = 1000;
	logarithmicScale = true;

	gridChanged = true;
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

	// clean up Roche Lobe data
	delete [] rocheLobeVertices;

	// clean up sky data
	delete [] skyVertices;

	delete palette;
}

void OpenGLWidget::setSimulation(Simulation* simulation)
{
	this->simulation = simulation;

	initDisk();
	initGrid();
	initDiskBorder();
	initOrbits();
	initRocheLobe();

	resetCamera();

	gridChanged = true;
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

	QImage sunGL("sun.tga");
	textures[0]=bindTexture(sunGL,GL_TEXTURE_2D, GL_RGBA);
	glBindTexture(GL_TEXTURE_2D, 0);
	
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
		glBindTexture(GL_TEXTURE_2D, textures[0]);
		gluQuadricTexture(quadric,true);
		glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
		gluSphere(quadric,0.1,32,32);
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

void OpenGLWidget::initRocheLobe()
{
	if (simulation == NULL)
		return;

	// we need at least 2 planets :)
	if (simulation->getNumberOfPlanets() < 2)
		return;

	rocheLobeVertices = new GLfloat[3*rocheLobeDetailLevel];
}

void OpenGLWidget::renderRocheLobe()
{
	if (simulation == NULL)
		return;

	if (simulation->getNumberOfPlanets() < 2)
		return;

	// mass ratio
	double q = simulation->getPlanetMass(1)[0]/simulation->getPlanetMass(0)[0];

	double L1 = calculateL1Point(q);

	glEnable(GL_LINE_SMOOTH);
	glPushMatrix();

	double x = simulation->getPlanetPosition(1)[0];
	double y = simulation->getPlanetPosition(1)[1];
	//double v_x = simulation->getPlanetVelocity(1)[0];
	//double v_y = simulation->getPlanetVelocity(1)[1];
	//double mass = simulation->getPlanetMass(1)[0];

	// angular momentum
	//double j = mass * x * v_y - mass * y * v_x;
	// distance
	//double d = sqrt(pow2(x)+pow2(y));
	// Runge-Lenz vector A = (p x L) - m * G * m * M * r/|r|;
	//double A_x =  j/mass * (1.0+mass) * v_y - 1.0 * 1.0 * pow2(1.0+mass) * x/d; 
	//double A_y = -j/mass * (1.0+mass) * v_x - 1.0 * 1.0 * pow2(1.0+mass) * y/d;
	//double eccentricity = sqrt(pow2(A_x) + pow2(A_y))/(1.0*1.0*pow2(1.0+mass));
	//double semi_major_axis = pow2(j/mass) / (1.0 * (1.0+mass)) / (1.0 - pow2(eccentricity));

	double phi = atan2(y,x);

	for (unsigned int j = 0; j < rocheLobeDetailLevel; ++j) {
		double r = calculateRocheRadius(q, L1, rochePotential(q,L1,0),2.0*M_PI/(float)rocheLobeDetailLevel*(float)j+phi)*sqrt(x*x+y*y);
		rocheLobeVertices[3*j+0] = r*sin(2.0*M_PI/(float)rocheLobeDetailLevel*(float)j);
		rocheLobeVertices[3*j+1] = r*cos(2.0*M_PI/(float)rocheLobeDetailLevel*(float)j);
		rocheLobeVertices[3*j+2] = 0.0;
	}

//	glTranslated(eccentricity*semi_major_axis,0,0);

	glColor3ub(0x80,0x80,0x80);
	glBegin(GL_LINE_LOOP);
	for (unsigned int j = 0; j < rocheLobeDetailLevel; ++j) {
		glVertex3fv(&rocheLobeVertices[3*j]);
	}
	glEnd();
	
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

void OpenGLWidget::initGrid()
{
	if (simulation == NULL)
		return;
	
	// nothing to be done here, as grid is already created by initDisk()
}

void OpenGLWidget::renderDisk()
{
	if (simulation == NULL)
		return;

	unsigned int index;

	if (gridChanged) {
		// set colors
		if (simulation->getQuantity() != NULL) {
			for (unsigned int nRadial = 0; nRadial <= simulation->getNRadial(); ++nRadial) {
				for (unsigned int nAzimuthal = 0; nAzimuthal < simulation->getNAzimuthal(); ++nAzimuthal) {
					index =  nRadial * simulation->getNAzimuthal() + nAzimuthal;
					diskColor(&diskColors[4*index], simulation->getQuantity()[index], minimumValue, maximumValue, logarithmicScale);
				}
			}
			gridChanged = false;
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

void OpenGLWidget::renderGrid()
{
	if (simulation == NULL)
		return;

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_LINE_SMOOTH);
	glPushMatrix();
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, diskVertices);
	glNormalPointer(GL_FLOAT, 0, diskNormals);
	glColor3ub(0x80,0x80,0x80);
	glDrawElements(GL_QUADS, 4*((simulation->getNRadial())*simulation->getNAzimuthal()), GL_UNSIGNED_INT, diskIndices);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glPopMatrix();
	glDisable(GL_LINE_SMOOTH);	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void OpenGLWidget::diskColor(GLfloat* color, double value, double minValue, double maxValue, bool logarithmic)
{
	QColor qcolor;

	if (logarithmic == false) {
		value = min(value, maxValue);
		value = max(value, minValue);
		value = (value-minValue)/(maxValue-minValue);
	} else {
		if (minValue == 0)
			minValue = DBL_EPSILON;
		maxValue = log10(maxValue);
		minValue = log10(minValue);
		value = log10(value);
		value = min(value, maxValue);
		value = max(value, minValue);
		value = (value-minValue)/(maxValue-minValue);
	}

	qcolor = palette->getColorNormalized(value);

	color[0] = qcolor.redF();
	color[1] = qcolor.greenF();
	color[2] = qcolor.blueF();
	color[3] = qcolor.alphaF();
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

	glPushMatrix();

	// do camera rotation without translation, so stars appear with infinity distance
	glLoadIdentity();
	glMultMatrixd(cameraRotationMatrix);	

	glColor3f(1.0,1.0,1.0);
	glPointSize(1.0);
	glBegin(GL_POINTS);
	for (unsigned int i = 0; i < skyNumberOfObjects; ++i) {
		glVertex3fv(&skyVertices[3*i]);
	}
	glEnd();

	glPopMatrix();

	glDisable(GL_POINT_SMOOTH);
}

void OpenGLWidget::renderKey()
{
	GLfloat marginRight;
	if (logarithmicScale) {
		marginRight = 45;
	} else {
		marginRight = 70;
	}
	const GLfloat marginTop = 35;
	const unsigned int fontSize = 10;
	const GLfloat keyWidth = 20.0;
	GLfloat keyHeight = height()-2*marginTop;
	QFont fontNormal = QFont("Helvetica", fontSize, QFont::Bold);
	QFont fontScript = QFont("Helvetica", fontSize*3.0/4.0, QFont::Bold);
	QFontMetrics fontMetricsNormal = QFontMetrics(fontNormal);

	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0,width(),0,height(),-150,150);
	glMatrixMode(GL_MODELVIEW);

	glBegin(GL_QUAD_STRIP);
	unsigned int count = palette->getNumberOfColors();
	unsigned int value;
	value = palette->getFirstValue();
	while (count > 0) {
		QColor color = palette->getColorByValue(value);
		color.setAlphaF(1.0);
		qglColor(color);
		GLfloat cellHeight = keyHeight*(1.0-((double)value-(double)palette->getMinValue())/((double)palette->getMaxValue()-(double)palette->getMinValue()));
		glVertex2f(width()-marginRight-keyWidth,height()-marginTop-cellHeight);
		glVertex2f(width()-marginRight,height()-marginTop-cellHeight);
		count--;
		value = palette->getNextValue(value);
	}
	glEnd();

	glColor3f(1.0,1.0,1.0);
	glBegin(GL_LINE_LOOP);
	glVertex2f(width()-marginRight-keyWidth,height()-marginTop-keyHeight);
	glVertex2f(width()-marginRight,height()-marginTop-keyHeight);
	glVertex2f(width()-marginRight,height()-marginTop);
	glVertex2f(width()-marginRight-keyWidth,height()-marginTop);
	glEnd();

	if (logarithmicScale) {
		int a,b, a_max, b_max;
		double pos;

		a = floor(log10(minimumValue));
		b = floor(minimumValue/pow(10.0,a))+1;
		if (b == 10) {
				a++;
				b = 0;
		}

		a_max = floor(log10(maximumValue));
		b_max = floor(maximumValue/pow(10.0,a_max));

		//printf("min: %lg a: %i b: %i i b*10^a = %lg\n", minimumValue, a, b, b*pow(10,a));
		//printf("max: %lg a: %i b: %i i b*10^a = %lg\n", maximumValue, a_max, b_max, b_max*pow(10,a_max));

		while ((a<a_max) || ((a==a_max) && (b<=b_max))) {
			glBegin(GL_LINE_STRIP);
			pos = (1-((double)a+log10((double)b)-log10(minimumValue))/(log10(maximumValue)-log10(minimumValue)));
			glVertex2f(width()-marginRight+3.0, height()-marginTop-pos*keyHeight);
			glVertex2f(width()-marginRight+0.0, height()-marginTop-pos*keyHeight);
			glEnd();

			if (b==1) {
				renderText(width()-marginRight+7.0, marginTop+(double)fontSize/2.0+keyHeight*pos, QString("10"),fontNormal);
				renderText(width()-marginRight+7.0+fontMetricsNormal.width("10")+1, marginTop-(double)fontSize/2.0+(double)fontSize/2.0+keyHeight*pos, QString("%1").arg(a),fontScript);
			}
			
			b++;
			if (b == 10) {
				b = 1;
				a++;
			}
		}

	} else {
		unsigned int maxTics = trunc(keyHeight/(2.0*fontSize));

		/*
		// show min value
		glBegin(GL_LINE_STRIP);
		glVertex2f(width()-marginRight+3.0, height()-marginTop-(keyHeight*1.0));
		glVertex2f(width()-marginRight+0.0, height()-marginTop-(keyHeight*1.0));
		glEnd();
		renderText(width()-marginRight+6.0, marginTop+(double)fontSize/2.0+keyHeight*1.0, QString("%1").arg(minimumValue),fontNormal);


		// show max value
		glBegin(GL_LINE_STRIP);
		glVertex2f(width()-marginRight+3.0, height()-marginTop-(keyHeight*0.0));
		glVertex2f(width()-marginRight+0.0, height()-marginTop-(keyHeight*0.0));
		glEnd();
		renderText(width()-marginRight+6.0, marginTop+(double)fontSize/2.0+keyHeight*0.0, QString("%1").arg(maximumValue),fontNormal); */

		for (unsigned int pos = 0; pos <= maxTics; ++pos) {
			glBegin(GL_LINE_STRIP);
			glVertex2f(width()-marginRight+3.0, height()-marginTop-(keyHeight*(GLfloat)pos/(GLfloat)maxTics));
			glVertex2f(width()-marginRight+0.0, height()-marginTop-(keyHeight*(GLfloat)pos/(GLfloat)maxTics));
			glEnd();

			double value = (float)(maxTics-pos)/(float)(maxTics)*(maximumValue-minimumValue)+minimumValue;
			int a = trunc(log10(fabs(value)));
			double b = value/pow(10.0,a);
			
			QString bStr = QString("%1\x95" "10").arg(b,0,'f',1);
			QString aStr = QString("%1").arg(a);
			
			renderText(width()-marginRight+7.0, marginTop+(double)fontSize/2.0+keyHeight*(GLfloat)pos/(GLfloat)maxTics, bStr ,fontNormal);
			renderText(width()-marginRight+7.0+fontMetricsNormal.width(bStr)+1, marginTop-(double)fontSize/2.0+(double)fontSize/2.0+keyHeight*(GLfloat)pos/(GLfloat)maxTics, aStr ,fontScript);
		}
	}

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glPopMatrix();
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

	// setup camera view
	setupCamera();

	if (showSky)
		renderSky();

	if (showDisk)
		renderDisk();

	if (showGrid)
		renderGrid();

	if (showOrbits)
		renderOrbits();

	if (showRocheLobe)
		renderRocheLobe();

	if (showPlanets)
		renderPlanets();
	
	if (showDiskBorder)
		renderDiskBorder();

	if (useMultisampling && supportMultisampling) {
		glDisable(GL_MULTISAMPLE_ARB);
	}

	if (showText) {
		if (simulation != NULL) {
			glColor3f(1.0,1.0,1.0);
			renderText(width()-120, 20, QString("Timestep: %1").arg(simulation->getCurrentTimestep()), QFont("Helvetica", 12, QFont::Bold) );
		}
	}

	if (showKey) {
		renderKey();
	}

	glFlush();

	if (saveScreenshots && (simulation != NULL)) {
		char temp[200];
		sprintf(temp, "/tmp/image_%i.png", simulation->getCurrentTimestep());
		QString filename = temp;

		// get image
		QImage image = grabFrameBuffer();
		image.save(filename, "PNG");
	}
}

void OpenGLWidget::updateShowDisk(bool value)
{
	showDisk = value;
	update();
}

void OpenGLWidget::updateShowGrid(bool value)
{
	showGrid = value;
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

void OpenGLWidget::updateShowRocheLobe(bool value)
{
	showRocheLobe = value;
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

void OpenGLWidget::updateShowKey(bool value)
{
	showKey = value;
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

void OpenGLWidget::setLogarithmic(bool value)
{
	logarithmicScale = value;

	if (minimumValue == 0)
		minimumValue = DBL_MIN;

	updateFromGrid();
	update();
}

void OpenGLWidget::setMinimumValue(double value)
{
	minimumValue = value;
	
	if (minimumValue > maximumValue)
		maximumValue = minimumValue;

	updateFromGrid();
	update();
}

void OpenGLWidget::setMaximumValue(double value)
{
	maximumValue = value;

	if (maximumValue < minimumValue)
		minimumValue = maximumValue;

	updateFromGrid();
	update();
}

void OpenGLWidget::updateFromGrid()
{
	gridChanged = true;
	update();
}

