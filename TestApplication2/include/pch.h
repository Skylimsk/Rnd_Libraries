#pragma once

#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <string>
#include<memory>
//for retun tuple
//#include <tuple>

//image save
#include <iomanip>
#include <cstdint>

// RGBA64 class
#include "RGBA64.h"
#include "CGParams.h"


enum EnergyMode {
    Single = 1,
    Dual = 2
};



enum DualRowMode {
    Unfold = 0,
    Fold = 1
};

enum EnergyLayout {
    Low_High = 1,
    High_Low = 2
};



typedef struct _CGData1D {
    /// <summary>
    /// Data 
    /// </summary>
    double* Data = 0;
    /// <summary>
    /// Size : Total size
    /// </summary>
    int Size = 0;
} CGData1D;


typedef struct _CGDataI {
    /// <summary>
    /// Data 
    /// </summary>
    int** Data = 0;
    /// <summary>
    /// Row : Height
    /// </summary>
    int Row = 0;
    /// <summary>
    /// Column : Width
    /// </summary>
    int Column = 0;
} CGDataI;


typedef struct _CGData {
    /// <summary>
    /// Data 
    /// </summary>
    double** Data = 0;
    /// <summary>
    /// Row : Height
    /// </summary>
    int Row = 0;
    /// <summary>
    /// Column : Width
    /// </summary>
    int Column = 0;
    /// <summary>
    /// Column : Width
    /// </summary>
    unsigned int Min = 0;
    /// <summary>
    /// Column : Width
    /// </summary>
    unsigned int Max = 0;
    /// <summary>
    /// Clear Crop : crop clear area in image
    /// </summary>
    //CGCropData* CropData = 0;
    /// <summary>
    /// Data 
    /// </summary>
    //double* DataAve = 0;
} CGData;


typedef struct _CGDataU321D {
    /// <summary>
    /// Data 
    /// </summary>
    uint32_t* Data = 0;
    /// <summary>
    /// Row : Height
    /// </summary>
    int Row = 0;
    /// <summary>
    /// Column : Width
    /// </summary>
    int Column = 0;
} CGDataU321D;

typedef struct _CGData64 {
    /// <summary>
    /// Data 
    /// </summary>
    RGBA64* Data = 0;
    /// <summary>
    /// Row : Height
    /// </summary>
    int Row = 0;
    /// <summary>
    /// Column : Width
    /// </summary>
    int Column = 0;
    /// <summary>
    /// Raw : Raw data
    /// </summary>
    uint16_t* Raw = 0;
    /// <summary>
    /// Data Extra
    /// </summary>
    RGBA64* DataEX = 0;
} CGData64;



typedef struct _CGDataU1D {
    /// <summary>
    /// Data 
    /// </summary>
    uint32_t* Data = 0;
    /// <summary>
    /// Row : Height
    /// </summary>
    int Row = 0;
    /// <summary>
    /// Column : Width
    /// </summary>
    int Column = 0;
} CGDataU1D;

typedef struct _ImageData {
    /// <summary>
    /// RawData : raw data from file
    /// </summary>
    CGData RawData; //analysis
    //CGDataU321D RawData; //daq
    /// <summary>
    /// CaliData : calibrated data from raw data
    /// </summary>
    CGData CaliData;
    /// <summary>
    /// MergedData :  Merged calibrated data as high and low energy data
    /// </summary>
    CGData MergedData;
    /// <summary>
    /// ImageData :  Data calculated to be viewed as image pixel value
    /// </summary>
    CGData64 ImageData;
    /// <summary>
    /// IntensityData :  Data as intensity value
    /// </summary>
    CGData IntensityData;
    /// <summary>
    /// HiEIntensityData :  High energy data intensity value
    /// </summary>
    CGData HiEIntensityData;
    /// <summary>
    /// LoEIntensityData :  Low energy data intensity value
    /// </summary>
    CGData LoEIntensityData;
    /// <summary>
    /// HiFilteredData :  High energy data filtered with CGAlgorithm
    /// </summary>
    CGData HiFilteredData;
    /// <summary>
    /// LoFilteredData :  Low energy data filtered with CGAlgorithm
    /// </summary>
    CGData LoFilteredData;
    /// <summary>
    /// FilterImageData :  Data calculated with applied filters to be viewed
    /// </summary>
    CGData64 FilterImageData;
    /// <summary>
    /// DisplayHeight : Image height displayed
    /// </summary>
    int DisplayHeight = 0;
    /// <summary>
    /// DisplayWidth : Image height displayed
    /// </summary>
    int DisplayWidth = 0;
} ImageDataMatrix;
using ImageDataMatrixPtr = std::shared_ptr<ImageDataMatrix>;

template <typename T>
extern void malloc2D(T**& ptr, int row, int col) { // tukar T**&

    ptr = static_cast<T**>(malloc(sizeof(T*) * row)); // new T[row, col];
    for (int i = 0; i < row; i++) {

        ptr[i] = static_cast<T*>(malloc(sizeof(T) * col));
        memset(ptr[i], 0, sizeof(T) * col);

    }

}


template <typename T>
extern void malloc1D(T*& ptr, int size) { // tukar T**&

    ptr = static_cast<T*>(malloc(sizeof(T) * size));
    memset(ptr, 0, sizeof(T) * size);

}
