#pragma once
#include <QImage>

void convertToGrayscaled(QImage& image);
void changeBrightness(QImage& image, int brightness);
double getDiversity(const QImage& image, const QImage& detailImage);
quint64 getDifference(const QImage& i1, const QImage& i2, int brightness);
int getBestBrightness(const QImage& i1, const QImage& i2, int brightnessVariation);
