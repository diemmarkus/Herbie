/*******************************************************************************************************
 DkSystemDiagnostics.cpp
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

#include "DkSystemDiagnostics.h"

// SystemDiagnostics --------------------------------------------------------------------
SystemDiagnostics::SystemDiagnostics(QWidget* parent) : QObject(parent) {

	numCpus = 0;
	maxCpu = 0;
	totalMemory = 0;
	QueryPerformanceFrequency((LARGE_INTEGER *)&frequency);
	qDebug() << "initialized... frequency: " << (double)frequency/1000000;

	// localize query
	QString processorName = DkUtils::getFunctionName(DkUtils::PROCESSOR);
	QString processorAttr = DkUtils::getFunctionName(DkUtils::PROCESSOR_TIME);
	QString queryString = DkUtils::makePdhPath(processorName, processorAttr);

	std::wstring queryStringW = DkUtils::qStringToStdWString(queryString);

	long status = PdhOpenQueryW(NULL, NULL, &cpuQueryTotal);
	qDebug() << "open query status: " << status;
	status = PdhAddCounterW(cpuQueryTotal, queryStringW.c_str(), NULL, &cpuCounterTotal);
	qDebug() << "add counter status: " << status;
	qDebug() << "query string: " << queryString;
	// L"\\Processor(_Total)\\% Processor Time"

	if (DkSettings::showCores) {
		cpuQuery.resize(getNumCpu());
		cpuCounters.resize(getNumCpu());

		for (int idx = 0; idx < getNumCpu(); idx++) {

			queryString = DkUtils::makePdhPath(processorName, processorAttr, idx);
			queryStringW = DkUtils::qStringToStdWString(queryString);

			qDebug() << queryString;
			PdhOpenQueryW(NULL, NULL, &cpuQuery[idx]);
			PdhAddCounterW(cpuQuery[idx], queryStringW.c_str(), NULL, &cpuCounters[idx]);
		}
	}
}

void SystemDiagnostics::update() {

	if (DkSettings::showMemory)
		getCurrentMemory();
	getCycle();
	getFrequency();
	//getCoreTemperature();

	//getMaxProcessIdx();
	//updateProcessList();
}

int SystemDiagnostics::getNumCpu() {

	if (!numCpus) {
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);

		numCpus = sysinfo.dwNumberOfProcessors;
	}

	return numCpus;
}

double SystemDiagnostics::getMaxCpu() {

	if (!maxCpu) {

	}

	return maxCpu;
}

#define rdtsc __asm __emit 0fh __asm __emit 031h
double SystemDiagnostics::getFrequency() {


	//DWORD dwEDX, dwEAX;
	////while(true)
	////{ 
	//	_asm
	//	{
	//		push edx
	//			push eax
	//			rdtsc
	//			mov dwEDX, edx
	//			mov dwEAX, eax
	//			pop edx
	//			pop eax
	//	}
	//	unsigned __int64 ui64Stamp1 = dwEDX;
	//	ui64Stamp1 <<= 32;
	//	ui64Stamp1 |= dwEAX;
	//	Sleep(1000);
	//	_asm
	//	{
	//		push edx
	//			push eax
	//			rdtsc
	//			mov dwEDX, edx
	//			mov dwEAX, eax
	//			pop edx
	//			pop eax
	//	}
	//	unsigned __int64 ui64Stamp2 = dwEDX;
	//	ui64Stamp2 <<= 32;
	//	ui64Stamp2 |= dwEAX;

	//	double frequency = (ui64Stamp2 - ui64Stamp1) / 1000000;
		//qDebug() <<  frequency << " MHz\n";
	//}

	return frequency;
}

double SystemDiagnostics::getTotalMemory() {

	if (!totalMemory) {

		double mem = -1;

#ifdef Q_OS_WIN

		MEMORYSTATUSEX MemoryStatus;
		ZeroMemory(&MemoryStatus, sizeof(MEMORYSTATUSEX));
		MemoryStatus.dwLength = sizeof(MEMORYSTATUSEX);

		if (GlobalMemoryStatusEx(&MemoryStatus)) {
			mem = MemoryStatus.ullTotalPhys;
		}

		if (mem > 0)
			totalMemory = mem;

#elif defined Q_OS_X11

		struct sysinfo info;

		if (!sysinfo(&info))
			mem = info.totalram;

#endif
	}

	qDebug() << "total memory: " << totalMemory;

	return totalMemory;
}

double SystemDiagnostics::getCurrentMemory() {

	MEMORYSTATUSEX memStat;
	ZeroMemory(&memStat, sizeof(MEMORYSTATUSEX));
	memStat.dwLength = sizeof(MEMORYSTATUSEX);

	GlobalMemoryStatusEx(&memStat);

	double memUsed = memStat.ullTotalPhys - memStat.ullAvailPhys;
	QString mem = DkUtils::stringifyMemory(memUsed);

	emit currentMemoryPercent(memStat.dwMemoryLoad);
	emit currentMemoryString(mem);

	return 0;
}

double SystemDiagnostics::getCycle() {

	// found here: http://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process
	PDH_FMT_COUNTERVALUE counterVal;
	counterVal.doubleValue = 0.0;

	long status = PdhCollectQueryData(cpuQueryTotal);
	PdhGetFormattedCounterValue(cpuCounterTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
	emit cpuPercent(counterVal.doubleValue, -1);

	if (!DkSettings::showCores)
		return 0;

	for (int idx = 0; idx < getNumCpu(); idx++) {

		PDH_FMT_COUNTERVALUE cVal;

		PdhCollectQueryData(cpuQuery[idx]);
		PdhGetFormattedCounterValue(cpuCounters[idx], PDH_FMT_DOUBLE, NULL, &cVal);

		emit cpuPercent(cVal.doubleValue, idx);
	}

	return 0;
}

void SystemDiagnostics::updateProcessList() {

	DkTimer dt;
	QString argProcess = DkUtils::getFunctionName(DkUtils::PROCESS);
	QString argAttr = DkUtils::getFunctionName(DkUtils::PROCESSOR_TIME);
	QString allProcessQuery = "\\" + argProcess + "(*)\\" + argAttr;
	std::wstring allProcessQueryW = DkUtils::qStringToStdWString(allProcessQuery);

	LPDWORD pSize = new DWORD[1];
	*pSize = 0;

	// this line takes ~1 sec on start-up
	PdhEnumObjectsW(NULL, NULL, NULL, pSize, PERF_DETAIL_NOVICE, true);
	delete pSize;
	//wchar_t* oList = new wchar_t[*pSize+1];
	qDebug() << "enum objects took me: " << dt.getTotal();


	LPDWORD pathSize = new DWORD[1];
	*pathSize = 0;

	PdhExpandWildCardPathW(NULL, allProcessQueryW.c_str(), NULL, pathSize, NULL);
	wchar_t* pList = new wchar_t[*pathSize+1];
	PdhExpandWildCardPathW(NULL, allProcessQueryW.c_str(), pList, pathSize, NULL);

	QVector<bool> delVector;
	delVector.resize(processList.size());
	delVector.fill(true);

	int s = 0;
	for (unsigned int cnt = 0; cnt < *pathSize; cnt++) {

		// iterate over the path list (by skipping the pointer)
		QString cQuery = DkUtils::stdWStringToQString(pList+cnt);

		s++;
		if (cQuery.isEmpty() || cQuery.contains("_Total") || cQuery.contains("Idle")) {
			cnt += cQuery.length();
			continue;
		}

		if (!processList.contains(cQuery)) {
			std::wstring cQueryW = DkUtils::qStringToStdWString(cQuery);

			processList.append(cQuery);
			PDH_HQUERY processQuery;
			PDH_HCOUNTER processCounter;
			long status = PdhOpenQueryW(NULL, NULL, &processQuery);
			status = PdhAddCounterW(processQuery, cQueryW.c_str(), NULL, &processCounter);

			processQueries.append(processQuery);
			processCounters.append(processCounter);
			delVector.append(false);

			qDebug() << "adding: " << cQuery;
		}
		else
			delVector[processList.indexOf(cQuery)] = false;

		cnt += cQuery.length();				
	}

	delete pList;
	delete pathSize;

	// clean process list
	for (int idx = 0; idx < delVector.size(); idx++) {

		// do we need to delete the current entry?
		if (!delVector[idx])
			continue;

		qDebug() << "removing: " << processList[idx];
		processList.removeAt(idx);
		processQueries.remove(idx);
		processCounters.remove(idx);

	}

	qDebug() << "process list size: " << s << " stringlist size: " << processList.size();

	qDebug() << "updating process list took me: " << dt.getTotal();
	//PdhLookupPerfNameByIndex(NULL, id, NULL, &pathSize); 

	//wchar_t* perfName = new wchar_t[pathSize];
	//PdhLookupPerfNameByIndex(NULL, id, (LPWSTR)perfName, &pathSize);

	//// copy to qstring
	//QString perfNameQ = QString::fromStdWString((const std::wstring)perfName);

}

int SystemDiagnostics::getMaxProcessIdx() {

	DkTimer dt;
	double maxPerformance = 0.0;
	int maxIdx = -1;

	PDH_FMT_COUNTERVALUE counterVal;
	counterVal.doubleValue = 0.0;

	// takes way too long (~30ms)
	for (int idx = 0; idx < processQueries.size(); idx++) {

		long status = PdhCollectQueryData(processQueries[idx]);
		PdhGetFormattedCounterValue(processCounters[idx], PDH_FMT_DOUBLE | PDH_FMT_NOCAP100, NULL, &counterVal);

		if (counterVal.doubleValue > maxPerformance) {
			maxPerformance = counterVal.doubleValue;
			maxIdx = idx;
		}
	}

	if (maxIdx != -1) 
		qDebug() << "hardest process: " << processList[maxIdx] << " cpu: " << maxPerformance/numCpus;

	qDebug() << "max idx time: " << dt.getTotal();

	return maxIdx;
}

//#include <iostream>
//#include <comdef.h>
//#include <Wbemidl.h>
//#pragma comment(lib, "wbemuuid.lib")

double SystemDiagnostics::getCoreTemperature() {

	return -1;
	
	//int pTemperature = -1;

	//CoInitialize(NULL); // needs comdef.h
	//CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
	//
	//IWbemLocator *pLocator = NULL; // needs Wbemidl.h & Wbemuuid.lib
	////CoCreateInstance(CLSID_WbemAdministrativeLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLocator);
	//CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLocator);

	//IWbemServices *pServices = NULL;
	//pLocator->ConnectServer(L"root\\WMI", NULL, NULL, NULL, 0, NULL, NULL, &pServices);
	////pLocator->Release();
	//
	//IEnumWbemClassObject *pEnum;
	//pServices->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM MSAcpi_ThermalZoneTemperature"), WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &pEnum);
	////pServices->Release();
	//
	//IWbemClassObject *pObject;
	//ULONG returned;
	//pEnum->Next(WBEM_INFINITE, 1, &pObject, &returned);
	//pEnum->Release();
	//
	//BSTR temp = SysAllocString(L"CurrentTemperature");
	//VARIANT v;
	//VariantInit(&v);

	//if (!v.vt)
	//	return 0;

	//pObject->Get(temp, 0, &v, NULL, NULL);
	//pObject->Release();
	//SysFreeString(temp);
	//
	//pTemperature = V_I4(&v);
	//VariantClear(&v);
	//
	//CoUninitialize();

	//qDebug() << "temperature: " << pTemperature;

	//return (double)pTemperature;

	//// 2nd approach
	//int pTemperature = -1;
	////if (pTemperature == NULL)
	////	return E_INVALIDARG;

	////*pTemperature = -1;
	//HRESULT ci = CoInitialize(NULL); // needs comdef.h
	//HRESULT hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
	//if (SUCCEEDED(hr))
	//{
	//	IWbemLocator *pLocator; // needs Wbemidl.h & Wbemuuid.lib
	//	hr = CoCreateInstance(CLSID_WbemAdministrativeLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLocator);
	//	if (SUCCEEDED(hr))
	//	{
	//		IWbemServices *pServices;
	//		BSTR ns = SysAllocString(L"root\\WMI");
	//		hr = pLocator->ConnectServer(ns, NULL, NULL, NULL, 0, NULL, NULL, &pServices);
	//		pLocator->Release();
	//		SysFreeString(ns);
	//		if (SUCCEEDED(hr))
	//		{
	//			BSTR query = SysAllocString(L"SELECT * FROM MSAcpi_ThermalZoneTemperature");
	//			BSTR wql = SysAllocString(L"WQL");
	//			IEnumWbemClassObject *pEnum;
	//			hr = pServices->ExecQuery(wql, query, WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &pEnum);
	//			SysFreeString(wql);
	//			SysFreeString(query);
	//			pServices->Release();
	//			if (SUCCEEDED(hr))
	//			{
	//				IWbemClassObject *pObject;
	//				ULONG returned;
	//				hr = pEnum->Next(WBEM_INFINITE, 1, &pObject, &returned);
	//				pEnum->Release();
	//				if (SUCCEEDED(hr))
	//				{
	//					BSTR temp = SysAllocString(L"CurrentTemperature");
	//					VARIANT v;
	//					VariantInit(&v);
	//					hr = pObject->Get(temp, 0, &v, NULL, NULL);
	//					pObject->Release();
	//					SysFreeString(temp);
	//					if (SUCCEEDED(hr))
	//					{
	//						pTemperature = V_I4(&v);
	//						qDebug() << "succeeded...";
	//					}
	//					VariantClear(&v);
	//				}
	//			}
	//		}
	//		if (ci == S_OK)
	//		{
	//			CoUninitialize();
	//		}
	//	}
	//}

	//qDebug() << "temperature: " << pTemperature;

	//return (double)pTemperature;
}