/*******************************************************************************************************
 DkSystemDiagnostics.h
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

#include <QWidget>

#include <QObject>
#include <QVector>
#include <QStringList>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#include <pdh.h>
#include <Psapi.h>
#pragma comment(lib,"pdh.lib")
#endif

#include "DkUtils.h"
#include "DkWidgets.h"

class SystemDiagnostics : public QObject {
	Q_OBJECT

public:
	SystemDiagnostics(QWidget* parent = 0);

	int getNumCpu();
	double getMaxCpu();
	double getTotalMemory();

	double getCurrentMemory();
	double getCycle();
	int getMaxProcessIdx();
	double getCoreTemperature();
	double getFrequency();

public slots:
	void update();
	void updateProcessList();

signals:
	void cpuPercent(double val, int cpuIdx);
	void currentMemoryPercent(double val);
	void currentMemoryString(QString memVal);

protected:
	int numCpus;
	double maxCpu;
	double totalMemory;

	PDH_HQUERY cpuQueryTotal;
	PDH_HCOUNTER cpuCounterTotal;

	QVector<PDH_HQUERY> cpuQuery;
	QVector<PDH_HCOUNTER> cpuCounters;

	QStringList processList;
	QVector<PDH_HQUERY> processQueries;
	QVector<PDH_HCOUNTER> processCounters;

	unsigned __int64 frequency;
	unsigned __int64 lastFreq;
	QVector<unsigned __int64> lastCycles;
};

