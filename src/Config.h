#pragma once
#include <QString>

class Config
{
public:
	Config(const QString& fileName);
	QString inputImage;
	QString detailImage;
	QString templatesDir;
	int generationTime;
	QString outputImage;
	int outputScale;
	int cellSize;
	int thumbnailSize;
	int brightnessVariation;
	int levelCount;
	float divisionThreshold;
	int dublicateDistance;
	bool showDivision;
};
