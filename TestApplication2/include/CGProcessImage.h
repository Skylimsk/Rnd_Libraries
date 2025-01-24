#pragma once

#include "ImageReader.h"
#include "DataCalibration.h"


class CGProcessImage {

public:
	CGProcessImage();
	~CGProcessImage();

public:
	static void ReadText(const std::string& filename, double**& matrix, int& rows, int& cols);
	static void ReadText(const std::string& filename, uint32_t*& matrix, int& rows, int& cols);

	//Set
	void SetEnergyMode(int value);
	void SetDualRowMode(int value);
	void SetDualRowDirection(int value);;
	void SetHighLowLayout(int value);
	void SetPixelMaxValue(int value);
	void SetAirSampleStart(int value);
	void SetAirSampleEnd(int value);

	// process
	void Process(double**& matrix, int& rows, int& columns);

	//Get
	bool GetCaliData(double**& data, int& row, int& column);
	bool GetMergedData(double**& data, int& row, int& column);

private:
	ImageDisplay LaneData;
	_ImageCalculationVariables CGImageCalculationVariables;
	_ImageMode CGImageMode;
	// raw data
	ImageDataMatrixPtr m_Data_Matrix = nullptr;

private:
	void CheckDisplaySetting();
	void GetMergeData(int rows, int columns);
	void GetIntensityData(int height, int width, double backgroundValue);

};
