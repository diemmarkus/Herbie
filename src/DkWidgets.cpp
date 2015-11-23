/*******************************************************************************************************
 DkWidgets.cpp
 Created on:	30.09.2012

 Herbie is a system diagnostics widget that perfectly integrates to the desktop. 
 
 Copyright (C) 2012-2012 Markus Diem <markus@nomacs.org>

 This file is part of Herbie.

 Herbie is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Herbie is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 *******************************************************************************************************/

#include "DkWidgets.h"

QColor DkSettings::fgdColor = QColor(255,255,255);
QColor DkSettings::bgdColor = QColor(0,0,0,100);
QColor DkSettings::barColor = QColor(42,142,255);
QPoint DkSettings::bottomRight = QPoint(30, 100);
QPoint DkSettings::screenCorner = QPoint();
bool DkSettings::locked = false;
bool DkSettings::showCores = true;
bool DkSettings::showHistory = true;
bool DkSettings::showMemory = true;

// DkPerformanceBar --------------------------------------------------------------------
DkPerformanceBar::DkPerformanceBar(QWidget* parent) : QLabel(parent) {

	fgdCol = DkSettings::barColor;
	bgdCol = DkSettings::fgdColor;
	bgdCol.setAlpha(bgdCol.alpha()*0.5f);
	value = 0;

	setStyleSheet("QLabel{background-color: " + DkUtils::colorToString(bgdCol) + ";}");
	setFixedHeight(3);
}

void DkPerformanceBar::updateVal(int val) {
	value = val;
	update();
}

void DkPerformanceBar::paintEvent(QPaintEvent* event) {

	QRect barRect(QPoint(), QSize(qRound(width()*value/100.0f), height()));
	QPainter painter(this);
	painter.setPen(Qt::NoPen);
	painter.setBackgroundMode(Qt::TransparentMode);
	painter.setBrush(fgdCol);
	painter.drawRect(barRect);
}

// DkPerformanceLabel --------------------------------------------------------------------
DkPerformanceLabel::DkPerformanceLabel(QString name, QWidget* parent) : QLabel(parent) {

	nameLabel = new QLabel(name);
	nameLabel->setFont(QFont("Segoe UI"));
	valLabel = new QLabel("--");
	valLabel->setFont(QFont("Segoe UI"));
	valLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	bar = new DkPerformanceBar();

	QLabel* textLabel = new QLabel();
	QHBoxLayout* layout = new QHBoxLayout;
	layout->setContentsMargins(0,0,0,0);
	layout->addWidget(nameLabel);
	layout->addWidget(valLabel);

	textLabel->setLayout(layout);

	QVBoxLayout* vLayout = new QVBoxLayout;
	vLayout->setContentsMargins(0,0,0,0);
	vLayout->setSpacing(2);
	vLayout->addWidget(textLabel);
	vLayout->addWidget(bar);

	setLayout(vLayout);
}

// performance plot --------------------------------------------------------------------
DkPerformancePlot::DkPerformancePlot(QWidget* parent) : QLabel(parent) {

	fgdColor = DkSettings::fgdColor;
	fgdColor.setAlpha(fgdColor.alpha()*0.5f);
	plotColor = DkSettings::barColor;
	stepSize = 2;
}

void DkPerformancePlot::resizeEvent(QResizeEvent *event) {

	gradient.setStart(QPoint(0,0));
	gradient.setFinalStop(QPoint(event->size().width()*0.2, 0));

	QLabel::resizeEvent(event);
}

void DkPerformancePlot::paintEvent(QPaintEvent *event) {

	int coordBorder = 0;

	QPolygon coords;
	coords.push_back(QPoint(1, height()));
	coords.push_back(QPoint(width(), height()));
	coords.push_back(QPoint(width(), 1));

	QRect xRect = geometry();
	xRect.setTopRight(QPoint(xRect.right()-coordBorder, xRect.bottom()-coordBorder));	// don't draw overlapping twice

	QRect yRect = geometry();
	yRect.setLeft(yRect.right()-coordBorder);

	QPolygonF histPoly;

	for (int idx = 0; idx < history.size(); idx++) {

		int hSize = width() - (history.size() - idx) * stepSize - coordBorder + 1;

		if (hSize > 0 && hSize < width())
			histPoly.push_back(QPointF(hSize, height()-history[idx]/100.0f*height()));
	}

	QPainter painter(this);

	// draw poly line
	QPen pen;
	gradient.setColorAt(1, plotColor);
	QColor penCol = plotColor;
	penCol.setAlpha(0);
	gradient.setColorAt(0, penCol);
	pen.setBrush(gradient);
	pen.setWidth(2);

	painter.setPen(pen);
	painter.setBrush(Qt::NoBrush);
	painter.drawPolyline(histPoly);

	int x = (width()-history.size()*stepSize-coordBorder-1 > 0) ? width()-history.size()*stepSize-coordBorder-1 : 0;

	// draw poly area
	histPoly.push_front(QPointF(x, height()-coordBorder-1));
	histPoly.push_back(QPointF(width()-stepSize-coordBorder+1, height()-coordBorder-1));

	QColor fillArea = plotColor;
	fillArea.setAlpha(fillArea.alpha()*0.5f);
	gradient.setColorAt(1, fillArea);
	fillArea.setAlpha(0);
	gradient.setColorAt(0, fillArea);

	painter.setPen(Qt::NoPen);
	painter.setBrush(gradient);
	painter.drawPolygon(histPoly);

	// draw coordinates
	pen.setBrush(fgdColor);
	pen.setWidth(6);
	painter.setPen(pen);
	painter.setBrush(Qt::NoBrush);
	painter.drawPolyline(coords);

	painter.end();

}


int DkSettings::fontSize = 27;

// DkColorChooser --------------------------------------------------------------------
DkColorChooser::DkColorChooser(QColor defaultColor, QString text, QWidget* parent, Qt::WindowFlags flags) : QWidget(parent, flags) {

	this->defaultColor = defaultColor;
	this->text = text;

	init();

}

void DkColorChooser::init() {

	colorDialog = new QColorDialog(this);
	colorDialog->setObjectName("colorDialog");
	colorDialog->setOption(QColorDialog::ShowAlphaChannel, true);

	QVBoxLayout* vLayout = new QVBoxLayout(this);
	vLayout->setContentsMargins(11,0,11,0);

	QLabel* colorLabel = new QLabel(text);
	colorButton = new QPushButton("", this);
	colorButton->setFlat(true);
	colorButton->setObjectName("colorButton");

	QPushButton* resetButton = new QPushButton(tr("Reset"), this);
	resetButton->setObjectName("resetButton");
	resetButton->setAutoDefault(true);

	QWidget* colWidget = new QWidget(this);
	QHBoxLayout* hLayout = new QHBoxLayout(colWidget);
	hLayout->setContentsMargins(11,0,11,0);

	hLayout->addWidget(colorButton);
	hLayout->addWidget(resetButton);
	hLayout->addStretch();

	vLayout->addWidget(colorLabel);
	vLayout->addWidget(colWidget);

	setColor(defaultColor);
	QMetaObject::connectSlotsByName(this);
}

void DkColorChooser::setColor(QColor color) {

	colorDialog->setCurrentColor(color);
	colorButton->setStyleSheet("QPushButton {background-color: " + DkUtils::colorToString(color) + "; border:0px; min-height:24px}");
}

QColor DkColorChooser::getColor() {
	return colorDialog->currentColor();
}

void DkColorChooser::on_resetButton_clicked() {
	setColor(defaultColor);
}

void DkColorChooser::on_colorButton_clicked() {
	colorDialog->show();
}

void DkColorChooser::on_colorDialog_accepted() {

	setColor(colorDialog->currentColor());
}

// preferences --------------------------------------------------------------------
DkPreferences::DkPreferences(QWidget* parent) : QDialog(parent) {

	createLayout();
}

void DkPreferences::createLayout() {

	fgdColorChooser = new DkColorChooser(DkSettings::fgdColor, tr("Font Color"), this);
	bgdColorChooser = new DkColorChooser(DkSettings::bgdColor, tr("Background Color"), this);
	barColorChooser = new DkColorChooser(DkSettings::barColor, tr("Bar Color"), this);

	cbShowCores = new QCheckBox(tr("Show &Cores"));
	cbShowCores->setChecked(DkSettings::showCores);
	cbShowMemory = new QCheckBox(tr("Show &Memory"));
	cbShowMemory->setChecked(DkSettings::showMemory);
	cbShowHistory = new QCheckBox(tr("Show & History"));
	cbShowHistory->setChecked(DkSettings::showHistory);

	spFontSize = new QSpinBox();
	spFontSize->setValue(DkSettings::fontSize);
	spFontSize->setRange(12, 42);

	QLabel* fontSizeLabel = new QLabel(tr("Font Size"));
	QWidget* spinWidget = new QWidget;
	QHBoxLayout* spinLayout = new QHBoxLayout(spinWidget);
	spinLayout->addWidget(spFontSize);
	spinLayout->addWidget(fontSizeLabel);

	okButton = new QPushButton(tr("&OK"), this);
	okButton->setObjectName("okButton");
	okButton->setDefault(true);

	cancelButton = new QPushButton(tr("&Cancel"), this);
	cancelButton->setObjectName("cancelButton");

	QWidget* okCancelWidget = new QWidget;
	QHBoxLayout* okCancelLayout = new QHBoxLayout(okCancelWidget);
	okCancelLayout->setAlignment(Qt::AlignRight);
	okCancelLayout->addWidget(okButton);
	okCancelLayout->addWidget(cancelButton);

	QWidget* colWidget = new QWidget;
	QVBoxLayout* colLayout = new QVBoxLayout(colWidget);
	colLayout->addWidget(fgdColorChooser);
	colLayout->addWidget(bgdColorChooser);
	colLayout->addWidget(barColorChooser);

	QWidget* showWidget = new QWidget;
	QVBoxLayout* showLayout = new QVBoxLayout(showWidget);
	showLayout->setContentsMargins(0, 27, 0, 0);
	showLayout->addWidget(cbShowCores);
	showLayout->addWidget(cbShowMemory);
	showLayout->addWidget(cbShowHistory);
	showLayout->addWidget(spinWidget);
	showLayout->addStretch();

	QWidget* hWidget = new QWidget;
	QHBoxLayout* hLayout = new QHBoxLayout(hWidget);
	hLayout->addWidget(colWidget);
	hLayout->addWidget(showWidget);

	QVBoxLayout* myLayout = new QVBoxLayout(this);
	myLayout->addWidget(hWidget);
	myLayout->addWidget(okCancelWidget);

	okButton->setDefault(true);

	QMetaObject::connectSlotsByName(this);
}

