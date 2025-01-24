#pragma once

#include "pch.h"

class DataCalibration {

public:
    
    static CGData CalibrateDataMatrix(double**& rawDataMatrix, int size1, int size2, int maximumValue, int airStart, int airEnd);

};