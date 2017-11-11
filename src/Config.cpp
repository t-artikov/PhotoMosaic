#include "Config.h"
#include <QSettings>
#include <QDebug>

Config::Config(const QString& fileName)
{
	QSettings config(fileName, QSettings::IniFormat);
	if (config.status() != QSettings::NoError || !config.contains("input/filename")) {
		qWarning() << "Invalid config";
		qWarning() << "Usage: PhotoMosaic configFile.ini";
		exit(-1);
	}

	inputImage = config.value("input/filename", "").toString();
	detailImage = config.value("input/detailImage", "").toString();
	templatesDir = config.value("input/templatesDir", "").toString();

	generationTime = config.value("generation/time", 10).toInt();
	cellSize = config.value("generation/cellSize", 20).toInt();
	thumbnailSize = cellSize;
	brightnessVariation = config.value("generation/brightnessVariation", 200).toInt();
	levelCount = config.value("generation/levelCount", 3).toInt();
	if (levelCount < 1) levelCount = 1;

	divisionThreshold = config.value("generation/divisionThreshold", 10).toFloat();
	dublicateDistance = config.value("generation/dublicateDistance", 5).toInt();
	showDivision = config.value("generation/showDivision", false).toBool();

	outputImage = config.value("output/filename", "out.png").toString();
	outputScale = config.value("output/scale", 1.f).toInt();
}
