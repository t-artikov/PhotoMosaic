#include "imageUtils.h"

void convertToGrayscaled(QImage& image)
{
	int W = image.width();
	int H = image.height();
	for (int y = 0; y < H; y++)
	{
		for (int x = 0; x < W; x++)
		{
			int c = qGray(image.pixel(x, y));
			image.setPixel(x, y, qRgb(c, c, c));
		}
	}
}

void changeBrightness(QImage& image, int brightness)
{
	int W = image.width();
	int H = image.height();
	for (int y = 0; y < H; y++)
	{
		for (int x = 0; x < W; x++)
		{
			int c = qRed(image.pixel(x, y));
			c = c + (0.5 - c / 255.0) * qAbs(brightness) + brightness * 0.5;
			c = qMax(0, qMin(c, 255));
			image.setPixel(x, y, qRgb(c, c, c));
		}
	}
}

static double getBrightness(const QImage& image)
{
	int W = image.width();
	int H = image.height();
	quint64 sum = 0;
	for (int y = 0; y < H; y++)
	{
		for (int x = 0; x < W; x++)
		{
			sum += qRed(image.pixel(x, y));
		}
	}
	return sum / double(W * H);
}

double getDiversity(const QImage& image, const QImage& detailImage)
{
	int W = image.width();
	int H = image.height();
	double b = getBrightness(image);
	double result = 0.0;
	for (int y = 0; y < H; y++)
	{
		for (int x = 0; x < W; x++)
		{
			int c = qRed(image.pixel(x, y));
			int d = qAbs(c - b);
			double detailK = !detailImage.isNull() ? qRed(detailImage.pixel(x, y)) / 255.0 : 1.0;
			result += d * d * detailK;
		}
	}
	result /= (W * H);
	result = sqrt(result);
	return result;
}

quint64 getDifference(const QImage& i1, const QImage& i2, int brightness)
{
	int size = i1.width();
	qint64 result = 0;
	for (int y = 0; y < size; y++)
	{
		for (int x = 0; x < size; x++)
		{
			int c1 = qRed(i1.pixel(x, y));
			int c2 = qRed(i2.pixel(x, y));
			int c2b = c2 + (0.5 - c2 / 255.0) * qAbs(brightness) + brightness * 0.5;
			c2b = qMax(0, qMin(c2b, 255));
			int d = qAbs(c2b - c1);
			result += d;
		}
	}
	return result;
}

int getBestBrightness(const QImage& i1, const QImage& i2, int brightnessVariation)
{
	int size = i1.width();
	double brightnessSum = 0;
	for (int y = 0; y < size; y++)
	{
		for (int x = 0; x < size; x++)
		{
			int c1 = qRed(i1.pixel(x, y));
			int c2 = qRed(i2.pixel(x, y));
			double b = 0.0;
			if (c1 < c2)
			{
				b = 255.0 * (c1 - c2) / (double)c2;
			}
			else if (c1 > c2)
			{
				b = 255.0 * (c1 - c2) / (255.0 - c2);
			}
			brightnessSum += b;
		}
	}
	int brightness = brightnessSum / (size * size);
	if (brightness < -brightnessVariation / 3) brightness = -brightnessVariation / 3;
	if (brightness > brightnessVariation) brightness = brightnessVariation;
	return brightness;
}
