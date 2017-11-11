#pragma once
#include <QDirIterator>
#include <QDebug>
#include <QElapsedTimer>
#include <QPainter>
#include <QTextStream>
#include <QFile>
#include "Mosaic.h"
#include "imageUtils.h"

Template::Template()
{}

Template::Template(const QString& filename, int thumbnailSize) :
	filename(filename),
	image(filename)
{
	convertToGrayscaled(image);
	thumbnail = image.scaled(thumbnailSize, thumbnailSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

Cell::Cell() :
	templateIndex(-1)
{}

Cell::Cell(const QImage& inputImage, int x, int y, int size, int thumbnailSize) :
	x(x),
	y(y),
	size(size),
	templateIndex(-1)
{
	QImage image = inputImage.copy(x, y, size, size);
	thumbnail = image.scaled(thumbnailSize, thumbnailSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

CellNeighbour::CellNeighbour() :
	cellIndex(-1),
	quality(0)
{}

CellNeighbour::CellNeighbour(int cellIndex, quint64 quality) :
	cellIndex(cellIndex),
	quality(quality)
{}

TemplateSubstitution::TemplateSubstitution() :
	quality(0),
	brightness(0)
{}

TemplateSubstitution::TemplateSubstitution(const Cell& cell, const Template& templ, int brightnessVariation)
{
	brightness = getBestBrightness(cell.thumbnail, templ.thumbnail, brightnessVariation);
	quality = getDifference(cell.thumbnail, templ.thumbnail, brightness);
}

Mutation::Mutation() :
	i1(-1),
	i2(-1),
	swap(false)
{}

Mosaic::Mosaic(const Config* config) :
	config(config),
	inputImage(config->inputImage)
{
	if (inputImage.isNull()) {
		qWarning() << "Invalid inputImage";
		exit(-1);
	}
	convertToGrayscaled(inputImage);
	if (!config->detailImage.isEmpty()) {
		detailImage.load(config->detailImage);
		convertToGrayscaled(detailImage);
	}
	if (!config->showDivision) initTemplates();
	initCells();
	if (!config->showDivision) optimize();
	drawResultImage();
}

void Mosaic::initTemplates()
{
	qDebug() << "Loading templates...";
	QDirIterator iter(config->templatesDir, QStringList() << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.tif", QDir::Files, QDirIterator::Subdirectories);
	while (iter.hasNext())
	{
		Template t(iter.next(), config->thumbnailSize);
		if (!t.image.isNull()) templates.push_back(t);
	}
	qDebug() << "Templates:" << templates.size();
	if (templates.empty()) exit(-1);
}

void Mosaic::initCells()
{
	qDebug() << "Generating cells...";
	topLevelCellSize = config->cellSize * int(pow(2.0, config->levelCount - 1) + 0.5);
	topLevelCellCountX = inputImage.width() / topLevelCellSize;
	topLevelCellCountY = inputImage.height() / topLevelCellSize;
	for (int y = 0; y < topLevelCellCountY; y++)
	{
		for (int x = 0; x < topLevelCellCountX; x++)
		{
			initCellsRecursive(x * topLevelCellSize, y * topLevelCellSize, topLevelCellSize, config->levelCount - 1, config->thumbnailSize);
		}
	}
	if (!config->showDivision)
	{
		for (int i = 0; i < cells.size(); i++)
		{
			initSubstitutions(cells[i]);
			cells[i].templateIndex = i % templates.size();
		}
		initNeighbours();
	}
	qDebug() << "Cells:" << cells.size();
	if(cells.isEmpty()) exit(-1);
}

void Mosaic::initCellsRecursive(int x, int y, int size, int level, int thumbnailSize)
{
	int smallSize = size / config->cellSize;
	if (level > 0)
	{
		QImage image = inputImage.copy(x, y, size, size).scaled(smallSize, smallSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		QImage detailImagePiece = !detailImage.isNull() ? detailImage.copy(x, y, size, size).scaled(smallSize, smallSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation) : QImage();
		double diversity = getDiversity(image, detailImagePiece);
		if (diversity > config->divisionThreshold)
		{
			int size2 = size / 2;
			int level2 = level - 1;
			initCellsRecursive(x, y, size2, level2, thumbnailSize);
			initCellsRecursive(x + size2, y, size2, level2, thumbnailSize);
			initCellsRecursive(x, y + size2, size2, level2, thumbnailSize);
			initCellsRecursive(x + size2, y + size2, size2, level2, thumbnailSize);
			return;
		}
	}
	Cell cell(inputImage, x, y, size, thumbnailSize);
	cells.push_back(cell);
}

void Mosaic::initSubstitutions(Cell& cell)
{
	cell.substitutions.reserve(templates.size());
	for (int i = 0; i < templates.size(); i++)
	{
		TemplateSubstitution substitution(cell, templates[i], config->brightnessVariation);
		cell.substitutions.push_back(substitution);
	}
}

static int getCellDistance(const Cell& c1, const Cell& c2)
{
	int dx = 0;
	if (c1.x > c2.x + c2.size) dx = c1.x - c2.x - c2.size;
	else if (c2.x > c1.x + c1.size) dx = c2.x - c1.x - c1.size;
	int dy = 0;
	if (c1.y > c2.y + c2.size) dy = c1.y - c2.y - c2.size;
	else if (c2.y > c1.y + c1.size) dy = c2.y - c1.y - c1.size;
	int size = qMin(c1.size, c2.size);
	return (dx + dy) / (size);
}

void Mosaic::initNeighbours()
{
	int maxDistance = config->dublicateDistance;
	for (int i = 0; i < cells.size(); i++)
	{
		for (int j = i + 1; j < cells.size(); j++)
		{
			Cell& cell1 = cells[i];
			Cell& cell2 = cells[j];
			int distance = getCellDistance(cell1, cell2);
			if (distance < maxDistance)
			{
				quint64 quality = maxDistance - distance;
				quality = quality * quality * 10000;
				cell1.neighbours.push_back(CellNeighbour(j, quality));
				cell2.neighbours.push_back(CellNeighbour(i, quality));
			}
		}
	}
}

void Mosaic::optimize()
{
	qDebug() << "Optimizing...";
	quint64 iterationCount = 0;
	qint64 quality = 0;
	foreach(const Cell& cell, cells)
	{
		quality += getCellQuality(cell, false);
	}

	QElapsedTimer timer;
	timer.start();
	float T0 = 2000000.f;
	float generationTime = config->generationTime * 1000;
	while (true)
	{
		if (iterationCount % 100000 == 0)
		{
			float timeLeft = qMax(0.f, generationTime - timer.elapsed());
			qDebug().nospace() << "iteration - " << iterationCount << ", quality - " << quality << ", time left - " << timeLeft / 1000.0f;
			if (timeLeft <= 0.f) break;
		}
		float T = T0 * (pow(0.95f, timer.elapsed() / generationTime * 200.f) - 0.0005f);
		quality += optimizeIteration(T);
		iterationCount++;
	}
}

static bool randBool(float p)
{
	float x = rand() / (float)RAND_MAX;
	return x <= p;
}

qint64 Mosaic::optimizeIteration(float T)
{
	Mutation mutation = createMutation();
	qint64 quality = getMutationQuality(mutation);
	if (quality < 0 || (T > 0.00001f && randBool(exp(-quality / T))))
	{
		applyMutation(mutation);
		return quality;
	}
	return 0;

}

Mutation Mosaic::createMutation()
{
	Mutation mutation;
	mutation.swap = rand() % 2 == 1;
	if (mutation.swap)
	{
		mutation.i1 = rand() % cells.size();
		mutation.i2 = rand() % cells.size();
	}
	else
	{
		mutation.i1 = rand() % cells.size();
		mutation.i2 = rand() % templates.size();
	}
	return mutation;
}

qint64 Mosaic::getMutationQuality(const Mutation& mutation)
{
	if (mutation.swap)
	{
		Cell& cell1 = cells[mutation.i1];
		Cell& cell2 = cells[mutation.i2];
		qint64 prevQuality = getCellQuality(cell1, true) + getCellQuality(cell2, true);
		std::swap(cell1.templateIndex, cell2.templateIndex);
		qint64 newQuality = getCellQuality(cell1, true) + getCellQuality(cell2, true);
		std::swap(cell1.templateIndex, cell2.templateIndex);
		return newQuality - prevQuality;
	}
	else
	{
		Cell& cell = cells[mutation.i1];
		int prevTempateIndex = cell.templateIndex;
		qint64 prevQuality = getCellQuality(cell, true);
		cell.templateIndex = mutation.i2;
		qint64 newQuality = getCellQuality(cell, true);
		cell.templateIndex = prevTempateIndex;
		return newQuality - prevQuality;
	}
}

void Mosaic::applyMutation(const Mutation& mutation)
{
	if (mutation.swap)
	{
		std::swap(cells[mutation.i1].templateIndex, cells[mutation.i2].templateIndex);
	}
	else
	{
		cells[mutation.i1].templateIndex = mutation.i2;
	}
}

qint64 Mosaic::getCellQuality(const Cell& cell, bool doubleDistanceQuality)
{
	qint64 result = 0;
	foreach(const CellNeighbour& neighbour, cell.neighbours)
	{
		const Cell& cell2 = cells[neighbour.cellIndex];
		if (cell.templateIndex != cell2.templateIndex) continue;
		result += neighbour.quality;
	}
	if (doubleDistanceQuality) result *= 2;
	result += cell.substitutions[cell.templateIndex].quality;
	result += qAbs(cell.substitutions[cell.templateIndex].brightness) * 80;
	return result;
}

void Mosaic::drawResultImage()
{
	qDebug() << "Drawing result...";
	int scale = config->outputScale;
	int W = topLevelCellCountX * topLevelCellSize * scale;
	int H = topLevelCellCountY * topLevelCellSize * scale;
	resultImage = QImage(W, H, QImage::Format_RGB32);
	QPainter painter(&resultImage);
	if (config->showDivision)
	{
		painter.drawImage(QRect(0, 0, W, H), inputImage, QRect(0, 0, W / scale, H / scale));
		painter.setPen(Qt::red);
	}
	foreach(const Cell& cell, cells)
	{
		if (config->showDivision)
		{
			painter.drawRect(cell.x * scale, cell.y * scale, cell.size * scale, cell.size * scale);
			continue;
		}
		const Template& templ = templates[cell.templateIndex];
		int brightness = cell.substitutions[cell.templateIndex].brightness;
		QImage templateImage = templ.image.scaled(cell.size * scale, cell.size * scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		changeBrightness(templateImage, brightness);
		painter.drawImage(cell.x * scale, cell.y * scale, templateImage);
	}
	painter.end();
}

QImage Mosaic::getResultImage()
{
	return resultImage;
}