/*******************************************************************************************************
 DkDiagnostics.h
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

#include <QMainWindow>
#include <QVBoxLayout>
#include <QLabel>
#include <QWidget>
#include <QKeyEvent>
#include <QDesktopWidget>
#include <QSystemTrayIcon>
#include <QColorDialog>
#include <QErrorMessage>

#include <QDebug>
#include <QTimer>
#include <QSettings>
#include <QProcess>

#include <string>

#include "DkWidgets.h"
#include "DkUtils.h"
#include "DkSystemDiagnostics.h"

class Diagnostics : public QMainWindow {
	Q_OBJECT

public:

	enum trayActionNames {
		tray_preferences,
		tray_lock,
		tray_minimize,
		tray_quit,

		tray_end
	};

	Diagnostics(QWidget *parent = 0);
	~Diagnostics();

public slots:
	void updateCpuVal(double val, int idx = -1);
	void updatePosition(int idx);
	void showPreferences();
	void updateStyles();
	void restart();
	void lock(bool locked);
	void showMinimized(bool minimize);

protected:
	void mouseDoubleClickEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void contextMenuEvent(QContextMenuEvent *event);
	void keyReleaseEvent(QKeyEvent* event);
	void keyPressEvent(QKeyEvent* event);
	void changeEvent(QEvent* event);
	bool event(QEvent* event);
	
	void init();
	void createLayout();
	void loadSettings();
	void saveSettings();

	QVector<DkPerformanceLabel*> cpuPercent;
	DkPerformanceLabel* allCpu;
	DkPerformanceLabel* memory;
	DkPerformancePlot* plotHistory;

	QPoint mouseGrab;
	QLabel* viewport;
	SystemDiagnostics* sysDiag;
	QTimer* timer;
	int fontSize;
	QDesktopWidget* desktop;
	QSystemTrayIcon* trayIcon;

	QMenu* trayMenu;
	QVector<QAction* > trayActions;

};
