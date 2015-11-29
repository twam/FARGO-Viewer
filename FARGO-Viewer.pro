version.target = version.h
version.commands = echo "\\$${LITERAL_HASH}ifndef _VERSION_H_" > version.h && echo "const char git_version[] = \\\"`git describe --tags --always --dirty`\\\"\\;" >> version.h && echo "\\$${LITERAL_HASH}endif" >> version.h
version.depends = .git
QMAKE_EXTRA_TARGETS += version
PRE_TARGETDEPS += version.h

TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
QT+=opengl
unix:!macx:LIBS += -lGLEW -lGLU	
macx:LIBS += -lGLEW

QMAKE_CXXFLAGS_DEBUG = -march=native -O2 -pipe -g
QMAKE_CXXFLAGS_RELEASE = -march=native -O2 -pipe -DNDEBUG

debug {
	TARGET = FARGO-Viewer.debug
}

release {
	TARGET = FARGO-Viewer
}

# Input
HEADERS += MainWidget.h OpenGLWidget.h Simulation.h config.h Palette.h PaletteWidget.h ColorWidget.h RocheLobe.h Vector.h Matrix.h OpenGLNavigationWidget.h FARGO.h version.h
SOURCES += main.cpp MainWidget.cpp OpenGLWidget.cpp Simulation.cpp config.cpp Palette.cpp PaletteWidget.cpp ColorWidget.cpp RocheLobe.cpp OpenGLNavigationWidget.cpp FARGO.cpp
