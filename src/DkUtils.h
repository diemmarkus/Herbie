/*******************************************************************************************************
 DkUtils.h
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

#include <Qstring>
#include <QColor>

#ifdef WIN32
#include <windows.h>
#include <pdh.h>
#include <Psapi.h>
#endif

#include <time.h>
#include <string>

class DkUtils {

public:
	static QString colorToString(QColor& col) {

		return "rgba(" + QString::number(col.red()) + "," + QString::number(col.green()) + "," + QString::number(col.blue()) + "," + QString::number((float)col.alpha()/255.0f*100.0f) + "%)";
	};

	static std::wstring qStringToStdWString(const QString &str) {
#ifdef _MSC_VER
		return std::wstring((const wchar_t *)str.utf16());
#else
		return str.toStdWString();
#endif
	}

	// code from: http://stackoverflow.com/questions/5625884/conversion-of-stdwstring-to-qstring-throws-linker-error
	static QString DkUtils::stdWStringToQString(const std::wstring &str) {
#ifdef _MSC_VER
		return QString::fromUtf16((const ushort *)str.c_str());
#else
		return QString::fromStdWString(str);
#endif
	}

	//// deprecated
	//static LPCWSTR stringToWchar(std::string str) {
	//	wchar_t *wChar = new wchar_t[(int)str.length()+1];
	//	size_t convertedChars = 0;
	//	mbstowcs_s(&convertedChars, wChar, str.length()+1, str.c_str(), _TRUNCATE);
	//	//mbstowcs(wChar, str.c_str(), str.length()+1);

	//	return (LPCWSTR)wChar;
	//};

	//static LPCWSTR stringToWchar(QString str) {

	//	wchar_t *wChar = new wchar_t[str.length()+1];
	//	str.toWCharArray(wChar);
	//	wChar[str.length()] = L'\0';

	//	return wChar;
	//};

	static QString stringifyMemory(double mem) {

		QString answer;
		if (abs(mem) < 0x400) {
			answer = QString::number(mem, 'f', 2) + " bytes";
		}
		else if (abs(mem) < 0x100000) {
			answer = QString::number(mem/0x400, 'f', 2) + " KB";
		}
		else if (abs(mem) < 0x40000000) {
			answer = QString::number(mem/0x100000, 'f', 2) + " MB";
		}
		else if (abs(mem) < 0x10000000000) {
			answer = QString::number(mem/0x40000000, 'f', 2) + " GB";
		}
		else
			answer = "> 999 GB man";

		return answer;

	};

	enum performanceIDs{
		PROCESSOR = 238,
		PROCESSOR_TIME = 6,
		TOTAL_PROCESSOR_TIME = 240,
		USER_TIME = 242,
		PROCESS = 230,
	};

	static QString getFunctionName(int id = PROCESSOR) {

		// calling PdhLookup with NULL returns the space needed
		DWORD pathSize = 0;
		PdhLookupPerfNameByIndexW(NULL, id, NULL, &pathSize); 

		wchar_t* perfName = new wchar_t[pathSize];
		PdhLookupPerfNameByIndexW(NULL, id, (LPWSTR)perfName, &pathSize);

		// copy to qstring
		//QString perfNameQ = QString::fromStdWString((const std::wstring)perfName);
		QString perfNameQ = DkUtils::stdWStringToQString(perfName);
		delete perfName;

		return perfNameQ;
	}

	static QString makePdhPath(QString argProcessor, QString argAttr, int idx = -1) {

		QString idxString = QString::number(idx);
		if (idx == -1)
			idxString = "_Total";

		QString pdhPath = "\\" + argProcessor + "(" + idxString + ")\\" + argAttr;
		return pdhPath;

	}


};



/**
* A small class which measures the time.
* This class is designed to measure the time of a method, especially
* intervals and the total time can be measured.
**/
class DkTimer {

protected:
	clock_t firstTick;	/**< the first tick**/
	clock_t	lastTick;	/**< the last tick**/

public:

	/**
	* Initializes the class and stops the clock.
	**/
	DkTimer() {
		firstTick = clock();
		lastTick = firstTick;
	};

	/**
	* Default destructor.
	**/
	~DkTimer() {};

	/**
	* Returns a string with the total time interval.
	* The time interval is measured from the time,
	* the object was initialized.
	* @return the time in seconds or milliseconds.
	**/
	QString getTotal() {
		lastTick = clock();
		double ct = (double) (lastTick-firstTick) / CLOCKS_PER_SEC;

		return stringifyTime(ct);
	};

	double getTotalTime() {

		lastTick = clock();
		return (double) (lastTick-firstTick) / CLOCKS_PER_SEC;
	}

	/**
	* Returns a string with the time interval.
	* The time interval since the last call of stop(), getIvl()
	* or getTotal().
	* @return the time in seconds or milliseconds.
	**/
	QString getIvl() {
		clock_t tmp = clock();
		double ct = (double) (tmp-lastTick) / CLOCKS_PER_SEC;
		lastTick = tmp;

		return stringifyTime(ct);
	};



	/**
	* Converts time to std::string.
	* @param ct current time interval
	* @return std::string the time interval as string
	**/ 
	QString stringifyTime(double ct) {

		QString msg = " ";

		if (ct < 1)
			msg += QString::number(ct*1000) + " ms";
		else if (ct < 60)
			msg += QString::number(ct) + " sec";
		else if (ct < 3600) {
			double m = (int)ct/60;	// floor
			msg += QString::number(m) + " min " + QString::number(ct-m*60, 'g', 0) + " sec";
		}
		else {
			double h = (int)ct/3600;
			msg += QString::number(h) + " hours " + QString::number(ct-h*3600.0f, 'g', 0) + " min";
		}

		return msg;

	};

	/**
	* Stops the clock.
	**/
	void stop() {
		lastTick = clock();
	};

	/**
	* Returns the current time.
	* @return double current time in seconds.
	**/ 
	double static getTime() {
		return (double) clock();
	};
};

