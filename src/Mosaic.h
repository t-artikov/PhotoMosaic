#pragma once
#include <QImage>
#include <QVector>
#include "Config.h"

class Cell;

class Template
{
public:
	Template();
	Template(const QString& filename, int thumbnailSize);
	QString filename;
	QImage image;
	QImage thumbnail;
};

class TemplateSubstitution
{
public:
	TemplateSubstitution();
	TemplateSubstitution(const Cell& cell, const Template& templ, int brightnessVariation);
	quint64 quality;
	int brightness;
};

class CellNeighbour
{
public:
	CellNeighbour();
	CellNeighbour(int cellIndex, quint64 quality);
	int cellIndex;
	quint64 quality;
};

class Cell
{
public:
	Cell();
	Cell(const QImage& inputImage, int x, int y, int size, int thumbnailSize);
	int x;
	int y;
	int size;
	QImage thumbnail;
	int templateIndex;
	QVector<TemplateSubstitution> substitutions;
	QVector<CellNeighbour> neighbours;

};

class Mutation
{
public:
	Mutation();
	bool swap;
	int i1;
	int i2;
};

class Mosaic
{
public:
	Mosaic(const Config* config);
	QImage getResultImage();
private:
	void initTemplates();
	void initCells();
	void initCellsRecursive(int x, int y, int size, int level, int thumbnailSize);
	void initSubstitutions(Cell& cell);
	void initNeighbours();
	void optimize();
	qint64 optimizeIteration(float temperature);
	Mutation createMutation();
	qint64 getMutationQuality(const Mutation& mutation);
	void applyMutation(const Mutation& mutation);
	qint64 getCellQuality(const Cell& cell, bool doubleDistanceQuality);
	void drawResultImage();

	const Config* config;
	QImage inputImage;
	QImage detailImage;
	QVector<Template> templates;
	QVector<Cell> cells;
	int topLevelCellSize;
	int topLevelCellCountX;
	int topLevelCellCountY;
	QImage resultImage;
};