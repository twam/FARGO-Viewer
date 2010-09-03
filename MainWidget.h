#ifndef _MAINWIDGET_H_
#define _MAINWIDGET_H_

#include <QWidget>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QCheckBox>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QSlider>
#include <QTimer>

#include "OpenGLWidget.h"
#include "Simulation.h"

class MainWidget : public QWidget
{
	Q_OBJECT

	public:
		MainWidget(QWidget* parent = 0);
		~MainWidget();
		void setSimulation(Simulation* simulation);

		OpenGLWidget* openGLWidget;

	public slots:
		void timerUpdate();
		void updateFromSimulation();
		void clickedPlayPause(bool value);
		void clickedStop();
		void clickedForward();
		void clickedBackward();
		void clickedBeginning();
		void clickedEnd();
		void clickedIncreaseFps();
		void clickedDecreaseFps();
		void triggeredOpen();
		void triggeredExit();
		void triggeredSetWindowSize();
		void changedTimeline(int value);
		void fpsUpdate();
		void timestepUpdate();

	private:
		void createMenu();
		void createButtons();

		QMenuBar* menuBar;
		QMenu* fileMenu;
		QMenu* viewMenu;
		QMenu* optionsMenu;
		QAction* exitAction;
		QAction* openAction;
		QAction* showDiskAction;
		QAction* showPlanetsAction;
		QAction* showOrbitsAction;
		QAction* showSkyAction;
		QAction* showTextAction;
		QAction* showDiskBorderAction;
		QAction* showKeyAction;
		QAction* useMultisampling;
		QAction* saveScreenshotsAction;
		QAction* setWindowSizeAction;
		
		QToolButton* playPauseButton;
		QToolButton* stopButton;
		QToolButton* forwardButton;
		QToolButton* backwardButton;
		QToolButton* beginningButton;
		QToolButton* endButton;
		QToolButton* increaseFpsButton;
		QToolButton* decreaseFpsButton;

		QLineEdit* timestepLineEdit;
		QLineEdit* fpsLineEdit;

		QSlider* timelineSlider;

		QHBoxLayout* buttonsLayout;
		QVBoxLayout* mainLayout;
		
		QTimer* timer;

		Simulation* simulation;
		double fps;

	protected:

};

#endif
