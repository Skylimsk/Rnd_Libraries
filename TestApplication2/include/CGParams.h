#pragma once

#include <map>
#include <string>

struct ImageDisplay {

    std::string LaneNumber = "-";
    int EnergyMode = 2; // 1|2
    int DualRowMode = 0; // 0|1
    int DualRowDirection = 0;  // 0|1 // Dual Row LCS Direction // 0: negative , 1: positive
    int HighLowLayout = 2; // 1|2
    double DefaultSpeed = 5; // 5-15

};



struct ImageDisplayData : public ImageDisplay {


public:
    bool IsRemoved = false;

//public slots:
//    void Slot_SetLaneNumber(QString _value) { LaneNumber = _value; }
//    void Slot_SetEnergyMode(int _value) { EnergyMode = _value + 1; }
//    void Slot_SetDualRowMode(int _value) { DualRowMode = _value; }
//    void Slot_SetDualRowDirection(int _value) { DualRowDirection = _value; }
//    void Slot_SetHighLowLayout(int _value) { HighLowLayout = _value + 1; }
//    void Slot_SetDefaultSpeed(QString _value) { DefaultSpeed = _value.toDouble(); }

};



struct _ImageDisplayParameters
{
    //int PictureboxHeight = 640;
    //int MaxZoomAmount = 11;
    //QString DualRowDirection = "0101    False";

    int EnergyMode_Default = 2;
    int DualRowMode_Default = 0;
    int DualRowDirection_Default = 0; // Dual Row LCS Direction // 0: negative , 1: positive
    std::string LaneNumber_Default = "0101";
    int HighLowLayout_Default = 2;
    double DefaultSpeed_Default = 5;

    ImageDisplay LaneData; // current lane data
    std::map<int, ImageDisplayData*> AllLaneData;
    int TotalLane = 0; //TotalImageDisplayData
    //const uint LaneData_TotalParam = 6;

    double VehicleSpeed = DefaultSpeed_Default;
    double SpeedRatio = 1;
    double SpeedRatioPrev = 1;
    //bool IsRotated = true;
    // energymode: single = 1; dual = 2;
    // dualrowmode: unfold = 0; folde = 1;
    // highlowlayout: low-high (01) = 1; high-low (10) = 2;

    void setVehicleSpeed(double value) {

        VehicleSpeed = value;
        SpeedRatio = VehicleSpeed / LaneData.DefaultSpeed; // DAQ min max 0.5 - 4
        //SpeedRatio = (((VehicleSpeed - MinSpeed) / (MaxSpeed - MinSpeed)) * (2.0 - 1.0) + 1.0); // DAQ min max 0.5 - 4
        SpeedRatio = std::min<double>(std::max<double>(SpeedRatio, 0.5), 4.0);

    };

};
extern _ImageDisplayParameters CGImageDisplayParameters;



struct _ImageCalculationVariables
{
    int PixelMaxValue = 65535;
    int AirSampleStart = 10;
    int AirSampleEnd = 80;
    double SharpStrength = 0.8;// 0.1; //kilang = 0.8
    int HPThreshold = 200;// 100; //kilang = 200
    int HPCoefficient = 4;// 5; //kilang = 4
    double HPPower = 1.75;
    int MaterialTransparency = 150;
    float FMaterialTransparency = 0.0;
    double BrightnessValue = 0;
    double ContrastValue = 1;
    double BrightnessValueDefault = 0;
    double ContrastValueDefault = 1;
    int WdThreshold = (int)(PixelMaxValue * 0.8);
    double BackgroundThreshold = 0.9;
    int BilateralHalfWidth = 5;
    double BilateralSigmaD = 3;
    double BilateralSigmaR = 0.06;
    int BilateralIteration = 1;

    // for high penetration
    bool HP_IsMultiplyCoeff = true;

    double getBackgroundThreshold() { return BackgroundThreshold * 8000; }
    double getBackgroundThreshold16Bit() { return getBackgroundThreshold() * 8; }

};
extern _ImageCalculationVariables CGImageCalculationVariables;



struct _ImageMode
{
    bool IsFilters = false;
    bool IsGrayscale = true;
    bool IsSharpen = true; // false: blur as default for DAQ
    bool IsEdgeGlow = false;
    bool IsEdgeSolid = false;
    bool IsInverse = false;
    bool IsHighPe = false;
    bool IsContrastEq = false;
    bool IsWireDe = false;
    bool IsBrightnessContrast = false;
    bool IsMaterial = false;
    bool IsOrganicStripping = false;
    bool IsMineralStripping = false;
    int ColorIndex = 0;
    int EnergyIndex = 2;
    int EdgeMode = 1;
    int RoiMode = 0;
    //colorindex: colormap pseudoZ = 1; colormap sunset = 2;
    //energyindex: low energy = 0; high energy = 1;  hybrid = 2;

    bool IsFilterApplied() {
        return IsFilters = (IsInverse || ColorIndex || getEdgeMode() > 1 || IsHighPe || IsContrastEq || IsWireDe || IsBrightnessContrast);
    }
    int getEdgeMode() {
        if (IsEdgeSolid)
        {
            return EdgeMode = 3;
        }
        else if (IsEdgeGlow)
        {
            return EdgeMode = 2;
        }
        else if (IsSharpen)
        {
            return EdgeMode = 1;
        }
        else {
            return EdgeMode = 0;
        }
    }
    int getRoiMode() {
        if (IsHighPe)
        {
            return RoiMode = 1;
        }
        else if (IsContrastEq)
        {
            return RoiMode = 2;
        }
        else if (IsWireDe)
        {
            return RoiMode = 3;
        }
        else if (IsBrightnessContrast)
        {
            return RoiMode = 4;
        }
        else {
            return RoiMode = 0;
        }
    }

    void RestoreToDefault()
    {
        IsFilters = false;
        IsGrayscale = true;
        IsSharpen = true;
        IsEdgeGlow = false;
        IsEdgeSolid = false;
        IsInverse = false;
        IsHighPe = false;
        IsContrastEq = false;
        IsWireDe = false;
        IsBrightnessContrast = false;
        IsMaterial = false;
        IsOrganicStripping = false;
        IsMineralStripping = false;
        ColorIndex = 0;
        EnergyIndex = 2;
        EdgeMode = 1;
        RoiMode = 0;
    }

};
extern _ImageMode CGImageMode;

