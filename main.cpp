#include <QApplication>
#include "MainWidget.h"
#include "Simulation.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	//Simulation simulation;
	//simulation.loadFromFile("/home/twam/scratch1/GammaCeph_200x200_Rad/in/gamma-ceph.par");
	//simulation.loadFromFile("/scratch/keid/twam/BW/scratch/Std/Mass1.0/1/in/sg.par");

	MainWidget mainWidget;
	mainWidget.show();
	//mainWidget.setSimulation(&simulation);

	return app.exec();
}
