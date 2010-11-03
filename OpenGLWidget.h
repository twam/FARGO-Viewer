#ifndef _OPENGLWIDGET_H_
#define _OPENGLWIDGET_H_

#include <QGLWidget>
#include "Simulation.h"
#include "Palette.h"
#include "RocheLobe.h"

class OpenGLWidget : public QGLWidget
{
	Q_OBJECT

	public:
		OpenGLWidget(QWidget *parent);
		~OpenGLWidget();
		void setSimulation(Simulation *simulation);
		void resetCamera();
		
		inline Palette* getPalette() { return palette; }
		
		void setLogarithmic(bool value);
		void setMinimumValue(double value);
		void setMaximumValue(double value);
		inline double getMinimumValue() const { return minimumValue; }
		inline double getMaximumValue() const { return maximumValue; }

	public slots:
		void updateShowDisk(bool value);
		void updateShowDiskBorder(bool value);
		void updateShowPlanets(bool value);
		void updateShowOrbits(bool value);
		void updateShowRocheLobe(bool value);
		void updateShowSky(bool value);
		void updateShowText(bool value);
		void updateShowKey(bool value);
		void updateUseMultisampling(bool value);
		void updateSaveScreenshots(bool value);

	protected:
		void initializeGL();
		void resizeGL(int width, int height);
		void paintGL();
		void wheelEvent(QWheelEvent *event);
		void mousePressEvent(QMouseEvent *event);
		void mouseMoveEvent(QMouseEvent *event);

	private:
		QPoint last_drag_pos;
		GLdouble cameraPositionX;
		GLdouble cameraPositionY;
		GLdouble cameraPositionZ;
		GLdouble cameraRotationX;
		GLdouble cameraRotationY;
		GLdouble cameraRotationZ;

		// disk
		bool showDisk;
		GLfloat* diskVertices;
		GLfloat* diskNormals;
		GLfloat* diskColors;
		GLuint* diskIndices;
		void initDisk();
		void renderDisk();
		void diskColor(GLfloat* color, double value, double minValue, double maxValue, bool logarithmic);

		// disk border
		bool showDiskBorder;
		GLfloat* diskBorderVertices;
		unsigned int diskBorderDetailLevel;
		void initDiskBorder();
		void renderDiskBorder();

		// planets
		bool showPlanets;
		void renderPlanets();

		// orbits
		bool showOrbits;
		GLfloat* orbitsVertices;
		unsigned int orbitsDetailLevel;
		void initOrbits();
		void renderOrbits();

		// roche lobe
		bool showRocheLobe;
		GLfloat* rocheLobeVertices;
		unsigned int rocheLobeDetailLevel;
		void initRocheLobe();
		void renderRocheLobe();
		
		// sky
		bool showSky;
		double skyDistance;
		unsigned int skyNumberOfObjects;
		GLfloat* skyVertices;
		void initSky();
		void renderSky();

		// text
		bool showText;

		// key
		bool showKey;
		void renderKey();

		bool saveScreenshots;
		bool supportMultisampling;
		bool useMultisampling;
		Simulation* simulation;

		Palette* palette;
		double minimumValue;
		double maximumValue;
		bool logarithmicScale;
};

#endif
