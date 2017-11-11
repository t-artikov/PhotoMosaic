#include <ctime>
#include "Mosaic.h"
#include <QCoreApplication>
#include <QStringList>

int main(int argc, char** argv)
{
	QCoreApplication app(argc, argv);
	app.setLibraryPaths(QStringList() << app.applicationDirPath());
	Config config(argc < 2 ? "config.ini" : argv[1]);
	Mosaic mosaic(&config);
	mosaic.getResultImage().save(config.outputImage);
}