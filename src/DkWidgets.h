/*******************************************************************************************************
 DkWidgets.h
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

#pragma once

#include <QColor>
#include <QWidget>
#include <QColorDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QPainter>
#include <QResizeEvent>

#include <QPoint>
#include <QDebug>

#include <DkUtils.h>

class DkSettings {

public:
	static QColor fgdColor;
	static QColor bgdColor;
	static QColor barColor;
	static QPoint bottomRight;
	static QPoint screenCorner;
	static bool locked;
	static bool showCores;
	static bool showHistory;
	static bool showMemory;
	static int fontSize;
};

class DkPerformanceBar : public QLabel {
	Q_OBJECT

public:
	DkPerformanceBar(QWidget* parent = 0);

	public slots:
		void updateVal(int val);

protected:
	void paintEvent(QPaintEvent* event);


	QColor fgdCol;
	QColor bgdCol;
	int value;
};

class DkPerformanceLabel : public QLabel {
	Q_OBJECT

public:
	DkPerformanceLabel(QString name = QString(), QWidget* parent = 0);

	public slots:
		void setVal(QString val) {
			valLabel->setText(val);
		};
		void updateBar(double val) {
			bar->updateVal(qRound(val));
		};

protected:
	QLabel* nameLabel;
	QLabel* valLabel;
	DkPerformanceBar* bar;
};

class DkPerformancePlot : public QLabel {
	Q_OBJECT

public:
	DkPerformancePlot(QWidget* parent = 0);

	public slots:
		void addValue(double val) {
			history.push_back(val);

			if ((history.size())*stepSize > width()) {
				history.pop_front();
			}

			update();
		};

protected:
	void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent *event);

	int stepSize;
	QVector<double> history;
	QColor plotColor;
	QColor fgdColor;
	QLinearGradient gradient;	

};


class DkColorChooser : public QWidget {
	Q_OBJECT

public:
	DkColorChooser(QColor defaultColor = QColor(), QString text = "Color", QWidget* parent = 0, Qt::WindowFlags flags = 0);
	virtual ~DkColorChooser() {};

	void setColor(QColor color);
	QColor getColor();

	public slots:
		void on_resetButton_clicked();
		void on_colorButton_clicked();
		void on_colorDialog_accepted();

protected:
	QColorDialog* colorDialog;
	QPushButton* colorButton;

	QColor defaultColor;
	QString text;

	void init();
};

class DkPreferences : public QDialog {
	Q_OBJECT

public:
	DkPreferences(QWidget* parent = 0);

	public slots:
		void on_okButton_pressed() {
			qDebug() << "ok pressed...";
			DkSettings::bgdColor = bgdColorChooser->getColor();
			DkSettings::fgdColor = fgdColorChooser->getColor();
			DkSettings::barColor = barColorChooser->getColor();	
			DkSettings::showCores = cbShowCores->isChecked();
			DkSettings::showMemory = cbShowMemory->isChecked();
			DkSettings::showHistory = cbShowHistory->isChecked();
			DkSettings::fontSize = spFontSize->value();
			accept();
		}

		void on_cancelButton_pressed() {
			reject();
		}

protected:
	void createLayout();

	DkColorChooser* fgdColorChooser;
	DkColorChooser* bgdColorChooser;
	DkColorChooser* barColorChooser;

	QCheckBox* cbShowCores;
	QCheckBox* cbShowMemory;
	QCheckBox* cbShowHistory;
	QSpinBox* spFontSize;

	QPushButton* okButton;
	QPushButton* cancelButton;
};

