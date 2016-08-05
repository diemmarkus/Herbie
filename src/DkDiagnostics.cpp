/*******************************************************************************************************
 DkDiagnostics.cpp
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

#include "DkDiagnostics.h"

#include <QApplication>
#include <QAction>
#include <QMenu>
#include <QFileInfo>
#include <QDir>

#ifdef Q_WS_X11
#include <sys/sysinfo.h>
#endif
#ifdef WIN32
#include <windows.h>
#endif


Diagnostics::Diagnostics(QWidget *parent) : QMainWindow(parent) {
	//ui.setupUi(this);

	setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnBottomHint);
	setAttribute(Qt::WA_TranslucentBackground);
	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	
	init();
	createLayout();
	adjustSize();
	//resize(250,200);

	connect(sysDiag, SIGNAL(cpuPercent(double, int)), this, SLOT(updateCpuVal(double, int)));
	
	if (DkSettings::showMemory) {
		connect(sysDiag, SIGNAL(currentMemoryString(QString)), memory, SLOT(setVal(QString)));
		connect(sysDiag, SIGNAL(currentMemoryPercent(double)), memory, SLOT(updateBar(double)));
	}

	sysDiag->update();
	timer->start();

	updatePosition(-1);

	int err = GetLastError();
	if (err != 0) {
		QErrorMessage* m = new QErrorMessage(this);
		m->showMessage(tr("Sorry, the winAPI reported this error: %1.\n Please report this error to me.").arg(err));
	}

}

Diagnostics::~Diagnostics() {
	saveSettings();
}

void Diagnostics::init() {
	
	allCpu = 0;
	memory = 0;
	plotHistory = 0;

	// set icon
	QString iconPath = QFileInfo(QApplication::applicationDirPath(), "herbie.ico").absoluteFilePath();
	QIcon wIcon = QIcon(iconPath);

	qDebug() << "Icon path is ok: " << QFileInfo(iconPath).exists();

	if (!wIcon.isNull())
		setWindowIcon(wIcon);
	else
		qDebug() << "could not load icon from: " << iconPath;

	// load settings
	loadSettings();
	
	sysDiag = new SystemDiagnostics(this);	
	timer = new QTimer(this);
	timer->setInterval(1000);
	connect(timer, SIGNAL(timeout()), sysDiag, SLOT(update()));

	desktop = QApplication::desktop();
	connect(desktop, SIGNAL(workAreaResized(int)), this, SLOT(updatePosition(int)));

	trayActions.resize(tray_end);

	trayActions[tray_preferences] = new QAction(tr("&Preferences"), this);
	connect(trayActions[tray_preferences], SIGNAL(triggered()), this, SLOT(showPreferences()));

	trayActions[tray_lock] = new QAction(tr("&Lock"), this);
	trayActions[tray_lock]->setCheckable(true);
	trayActions[tray_lock]->setChecked(DkSettings::locked);
	connect(trayActions[tray_lock], SIGNAL(toggled(bool)), this, SLOT(lock(bool)));

	trayActions[tray_minimize] = new QAction(tr("&Minimize"), this);
	trayActions[tray_minimize]->setCheckable(true);
	trayActions[tray_minimize]->setChecked(false);
	connect(trayActions[tray_minimize], SIGNAL(toggled(bool)), this, SLOT(showMinimized(bool)));

	trayActions[tray_quit] = new QAction(tr("&Quit"), this);
	trayActions[tray_quit]->setShortcuts(QKeySequence::Quit);
	connect(trayActions[tray_quit], SIGNAL(triggered()), this, SLOT(close()));

	trayMenu = new QMenu(tr("Diagostics"), this);
	trayMenu->addActions(trayActions.toList());

	//QIcon tIcon(":/diagnostics/img/trayIcon.png");
	trayIcon = new QSystemTrayIcon(wIcon, this);
	trayIcon->setContextMenu(trayMenu);
	trayIcon->show();
}

void Diagnostics::createLayout() {

	// find out how many cpu's are used
	updateStyles();

	//DkSettings::bgdColor = QColor(0,0,0,50);
	viewport = new QLabel();
	viewport->setObjectName("DkViewport");

	if (DkSettings::showCores) {
		for (int idx = 0; idx < sysDiag->getNumCpu(); idx++) {
		
			DkPerformanceLabel* cL = new DkPerformanceLabel(tr("Core"));
			cL->setStyleSheet("QLabel{font-size: " + QString::number(std::max(qRound(DkSettings::fontSize*0.5f), 12)) + "px;}");
			cpuPercent.push_back(cL);
		}
	}

	if (DkSettings::showMemory)
		memory = new DkPerformanceLabel(tr("Memory"));
	allCpu = new DkPerformanceLabel(tr("CPU"));

	QVBoxLayout* myLayout = new QVBoxLayout();

	if (DkSettings::showMemory)
		myLayout->addWidget(memory);
	myLayout->addWidget(allCpu);

	if (DkSettings::showCores) {
		for (int idx = 0; idx < sysDiag->getNumCpu(); idx++)
			myLayout->addWidget(cpuPercent[idx]);
	}

	QLabel* rightBars = new QLabel();
	rightBars->setObjectName("DkRightBars");
	rightBars->setStyleSheet(styleSheet());
	rightBars->setLayout(myLayout);
	rightBars->setMinimumWidth(DkSettings::fontSize*10);
	rightBars->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	rightBars->adjustSize();


	QWidget* dummy = 0;
	if (DkSettings::showHistory) {
		// init plot
		plotHistory = new DkPerformancePlot();
		plotHistory->setMinimumWidth(DkSettings::fontSize*10);

		dummy = new QWidget();	// borders
		QHBoxLayout* dLayout = new QHBoxLayout(dummy);
		dLayout->addWidget(plotHistory);
	}

	QHBoxLayout* vpLayout = new QHBoxLayout();
	vpLayout->setContentsMargins(0,0,0,0);
	vpLayout->setSpacing(0);
	if (DkSettings::showHistory)
		vpLayout->addWidget(dummy);
	vpLayout->addWidget(rightBars);

	viewport->setLayout(vpLayout);
	viewport->adjustSize();

	setCentralWidget(viewport);
}

void Diagnostics::loadSettings() {

	QSettings settings;
	DkSettings::bottomRight = settings.value("AppSettings/position", DkSettings::bottomRight).toPoint();
	DkSettings::fgdColor = settings.value("AppSettings/fgdColor", DkSettings::fgdColor).value<QColor>();
	DkSettings::bgdColor = settings.value("AppSettings/bgdColor", DkSettings::bgdColor).value<QColor>();
	DkSettings::barColor = settings.value("AppSettings/barColor", DkSettings::barColor).value<QColor>();
	DkSettings::locked = settings.value("AppSettings/locked", DkSettings::locked).toBool();
	DkSettings::showCores = settings.value("AppSettings/showCores", DkSettings::showCores).toBool();
	DkSettings::showMemory = settings.value("AppSettings/showMemory", DkSettings::showMemory).toBool();
	DkSettings::showHistory = settings.value("AppSettings/showHistory", DkSettings::showHistory).toBool();
	DkSettings::fontSize = settings.value("AppSettings/fontSize", DkSettings::fontSize).toInt();

	lock(DkSettings::locked);
}

void Diagnostics::saveSettings() {

	QSettings settings;
	settings.setValue("AppSettings/position", DkSettings::bottomRight);
	settings.setValue("AppSettings/fgdColor", DkSettings::fgdColor);
	settings.setValue("AppSettings/bgdColor", DkSettings::bgdColor);
	settings.setValue("AppSettings/barColor", DkSettings::barColor);
	settings.setValue("AppSettings/locked", DkSettings::locked);
	settings.setValue("AppSettings/showCores", DkSettings::showCores);
	settings.setValue("AppSettings/showMemory", DkSettings::showMemory);
	settings.setValue("AppSettings/showHistory", DkSettings::showHistory);
	settings.setValue("AppSettings/fontSize", DkSettings::fontSize);
}

#define Q_OS_WINCE
void Diagnostics::mouseDoubleClickEvent(QMouseEvent *event) {

	// TODO: exit the cmd is not working...
	QProcess p;
	bool started = p.startDetached("cmd /c taskmgr.exe & exit");

	if (!started)
		qDebug() << "could not start task manager";
}

void Diagnostics::mousePressEvent(QMouseEvent *event) {

	if (event->buttons() == Qt::LeftButton)
		mouseGrab = event->globalPos();

	QMainWindow::mousePressEvent(event);
}

void Diagnostics::mouseMoveEvent(QMouseEvent *event) {

	if (event->buttons() == Qt::LeftButton) {
		move(geometry().topLeft() + (event->globalPos()-mouseGrab));
		DkSettings::bottomRight = DkSettings::screenCorner-geometry().bottomRight();
	}
		
	mouseGrab = event->globalPos();

	QMainWindow::mouseMoveEvent(event);
}

void Diagnostics::mouseReleaseEvent(QMouseEvent *event) {

	QMainWindow::mouseReleaseEvent(event);
}

void Diagnostics::contextMenuEvent(QContextMenuEvent *event) {

	trayMenu->exec(event->globalPos());
}

void Diagnostics::keyPressEvent(QKeyEvent* event) {

	QMainWindow::keyPressEvent(event);
}

void Diagnostics::keyReleaseEvent(QKeyEvent* event) {

	if (event->key() == Qt::Key_Escape)
		close();

	QMainWindow::keyReleaseEvent(event);
}

void Diagnostics::changeEvent(QEvent* event) {

	// forcing show
	QMainWindow::changeEvent(event);	
}

bool Diagnostics::event(QEvent* event) {
	
	if (event->type() == QEvent::WindowStateChange) {
		qDebug() << "state changed, preventing it?";
		qDebug() << "minimized: " << isMinimized();
		return true;
	}

	return QMainWindow::event(event);
}

void Diagnostics::updateCpuVal(double val, int idx) {

	// total cpu performance
	if (idx == -1) {
		if (plotHistory)
			plotHistory->addValue(val);
		allCpu->setVal(QString::number(val, 'f', 1) + "%");
		allCpu->updateBar(val);
		//qDebug() << "cpu val (clean): " << val;
	}
	else if (idx < cpuPercent.size()) {
		cpuPercent[idx]->setVal(QString::number(val, 'f', 1) + "%");
		cpuPercent[idx]->updateBar(val);
	}

}

void Diagnostics::showPreferences() {

	DkPreferences* preferences = new DkPreferences();
	preferences->setStyleSheet("");
	preferences->show();

	//restart here - it's easier
	if (preferences->exec()) {
		restart();
	}
}

void Diagnostics::updatePosition(int idx) {

	DkSettings::screenCorner = QPoint();
	for (int idx = 0; idx < desktop->screenCount(); idx++) {
		if (desktop->screenGeometry(idx).right() > DkSettings::screenCorner.x()) {
			DkSettings::screenCorner = desktop->screenGeometry(idx).bottomRight();
		}
	}

	qDebug() << "new screen corner: " << DkSettings::screenCorner;

	// TODO: correct if user positions top right and switches from 2 monitors to 1
	if (!DkSettings::bottomRight.isNull()) {
		QRect r = geometry();
		r.moveBottomRight(DkSettings::screenCorner-DkSettings::bottomRight);
		qDebug() << r;
		setGeometry(r);
	}
}

void Diagnostics::updateStyles() {
	QString styles = 
		"QLabel#DkViewport{background-color: " + DkUtils::colorToString(DkSettings::bgdColor) + ";}" +
		"QLabel{color: " + DkUtils::colorToString(DkSettings::fgdColor) + "; font-size: " + QString::number(DkSettings::fontSize) + "px;}";
	
	setStyleSheet(styles);

	qDebug() << "styles updated...";
}

void Diagnostics::showMinimized(bool minimize) {

	if (minimize)
		QMainWindow::showMinimized();
	else
		showNormal();
}

void Diagnostics::restart() {

	qDebug() << "restarting...";

	QString exe = "\"" + QApplication::applicationFilePath() + "\"";

	QProcess process;
	bool started = process.startDetached(exe);

	// close me if the new instance started
	if (started)
		close();
	else
		qDebug() << "sorry, I could not restart: " << exe;
}

void Diagnostics::lock(bool locked) {
	DkSettings::locked = locked;

	if (locked) {
		HWND hwnd = (HWND)this->winId();
		int extendedStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
		SetWindowLong(hwnd, GWL_EXSTYLE, extendedStyle | WS_EX_TRANSPARENT);
		qDebug() << "locking...";
	}
	else {
		HWND hwnd = (HWND)this->winId();
		int extendedStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
		SetWindowLong(hwnd, GWL_EXSTYLE, extendedStyle & ~WS_EX_TRANSPARENT);
	}
}
