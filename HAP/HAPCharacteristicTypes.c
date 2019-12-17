// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

const HAPUUID kHAPCharacteristicType_AdministratorOnlyAccess = HAPUUIDCreateAppleDefined(0x1);

const HAPUUID kHAPCharacteristicType_AudioFeedback = HAPUUIDCreateAppleDefined(0x5);

const HAPUUID kHAPCharacteristicType_Brightness = HAPUUIDCreateAppleDefined(0x8);

const HAPUUID kHAPCharacteristicType_CoolingThresholdTemperature = HAPUUIDCreateAppleDefined(0xD);

const HAPUUID kHAPCharacteristicType_CurrentDoorState = HAPUUIDCreateAppleDefined(0xE);

const HAPUUID kHAPCharacteristicType_CurrentHeatingCoolingState = HAPUUIDCreateAppleDefined(0xF);

const HAPUUID kHAPCharacteristicType_CurrentRelativeHumidity = HAPUUIDCreateAppleDefined(0x10);

const HAPUUID kHAPCharacteristicType_CurrentTemperature = HAPUUIDCreateAppleDefined(0x11);

const HAPUUID kHAPCharacteristicType_HeatingThresholdTemperature = HAPUUIDCreateAppleDefined(0x12);

const HAPUUID kHAPCharacteristicType_Hue = HAPUUIDCreateAppleDefined(0x13);

const HAPUUID kHAPCharacteristicType_Identify = HAPUUIDCreateAppleDefined(0x14);

const HAPUUID kHAPCharacteristicType_LockControlPoint = HAPUUIDCreateAppleDefined(0x19);

const HAPUUID kHAPCharacteristicType_LockManagementAutoSecurityTimeout = HAPUUIDCreateAppleDefined(0x1A);

const HAPUUID kHAPCharacteristicType_LockLastKnownAction = HAPUUIDCreateAppleDefined(0x1C);

const HAPUUID kHAPCharacteristicType_LockCurrentState = HAPUUIDCreateAppleDefined(0x1D);

const HAPUUID kHAPCharacteristicType_LockTargetState = HAPUUIDCreateAppleDefined(0x1E);

const HAPUUID kHAPCharacteristicType_Logs = HAPUUIDCreateAppleDefined(0x1F);

const HAPUUID kHAPCharacteristicType_Manufacturer = HAPUUIDCreateAppleDefined(0x20);

const HAPUUID kHAPCharacteristicType_Model = HAPUUIDCreateAppleDefined(0x21);

const HAPUUID kHAPCharacteristicType_MotionDetected = HAPUUIDCreateAppleDefined(0x22);

const HAPUUID kHAPCharacteristicType_Name = HAPUUIDCreateAppleDefined(0x23);

const HAPUUID kHAPCharacteristicType_ObstructionDetected = HAPUUIDCreateAppleDefined(0x24);

const HAPUUID kHAPCharacteristicType_On = HAPUUIDCreateAppleDefined(0x25);

const HAPUUID kHAPCharacteristicType_OutletInUse = HAPUUIDCreateAppleDefined(0x26);

const HAPUUID kHAPCharacteristicType_RotationDirection = HAPUUIDCreateAppleDefined(0x28);

const HAPUUID kHAPCharacteristicType_RotationSpeed = HAPUUIDCreateAppleDefined(0x29);

const HAPUUID kHAPCharacteristicType_Saturation = HAPUUIDCreateAppleDefined(0x2F);

const HAPUUID kHAPCharacteristicType_SerialNumber = HAPUUIDCreateAppleDefined(0x30);

const HAPUUID kHAPCharacteristicType_TargetDoorState = HAPUUIDCreateAppleDefined(0x32);

const HAPUUID kHAPCharacteristicType_TargetHeatingCoolingState = HAPUUIDCreateAppleDefined(0x33);

const HAPUUID kHAPCharacteristicType_TargetRelativeHumidity = HAPUUIDCreateAppleDefined(0x34);

const HAPUUID kHAPCharacteristicType_TargetTemperature = HAPUUIDCreateAppleDefined(0x35);

const HAPUUID kHAPCharacteristicType_TemperatureDisplayUnits = HAPUUIDCreateAppleDefined(0x36);

const HAPUUID kHAPCharacteristicType_Version = HAPUUIDCreateAppleDefined(0x37);

const HAPUUID kHAPCharacteristicType_PairSetup = HAPUUIDCreateAppleDefined(0x4C);

const HAPUUID kHAPCharacteristicType_PairVerify = HAPUUIDCreateAppleDefined(0x4E);

const HAPUUID kHAPCharacteristicType_PairingFeatures = HAPUUIDCreateAppleDefined(0x4F);

const HAPUUID kHAPCharacteristicType_PairingPairings = HAPUUIDCreateAppleDefined(0x50);

const HAPUUID kHAPCharacteristicType_FirmwareRevision = HAPUUIDCreateAppleDefined(0x52);

const HAPUUID kHAPCharacteristicType_HardwareRevision = HAPUUIDCreateAppleDefined(0x53);

const HAPUUID kHAPCharacteristicType_AirParticulateDensity = HAPUUIDCreateAppleDefined(0x64);

const HAPUUID kHAPCharacteristicType_AirParticulateSize = HAPUUIDCreateAppleDefined(0x65);

const HAPUUID kHAPCharacteristicType_SecuritySystemCurrentState = HAPUUIDCreateAppleDefined(0x66);

const HAPUUID kHAPCharacteristicType_SecuritySystemTargetState = HAPUUIDCreateAppleDefined(0x67);

const HAPUUID kHAPCharacteristicType_BatteryLevel = HAPUUIDCreateAppleDefined(0x68);

const HAPUUID kHAPCharacteristicType_CarbonMonoxideDetected = HAPUUIDCreateAppleDefined(0x69);

const HAPUUID kHAPCharacteristicType_ContactSensorState = HAPUUIDCreateAppleDefined(0x6A);

const HAPUUID kHAPCharacteristicType_CurrentAmbientLightLevel = HAPUUIDCreateAppleDefined(0x6B);

const HAPUUID kHAPCharacteristicType_CurrentHorizontalTiltAngle = HAPUUIDCreateAppleDefined(0x6C);

const HAPUUID kHAPCharacteristicType_CurrentPosition = HAPUUIDCreateAppleDefined(0x6D);

const HAPUUID kHAPCharacteristicType_CurrentVerticalTiltAngle = HAPUUIDCreateAppleDefined(0x6E);

const HAPUUID kHAPCharacteristicType_HoldPosition = HAPUUIDCreateAppleDefined(0x6F);

const HAPUUID kHAPCharacteristicType_LeakDetected = HAPUUIDCreateAppleDefined(0x70);

const HAPUUID kHAPCharacteristicType_OccupancyDetected = HAPUUIDCreateAppleDefined(0x71);

const HAPUUID kHAPCharacteristicType_PositionState = HAPUUIDCreateAppleDefined(0x72);

const HAPUUID kHAPCharacteristicType_ProgrammableSwitchEvent = HAPUUIDCreateAppleDefined(0x73);

const HAPUUID kHAPCharacteristicType_StatusActive = HAPUUIDCreateAppleDefined(0x75);

const HAPUUID kHAPCharacteristicType_SmokeDetected = HAPUUIDCreateAppleDefined(0x76);

const HAPUUID kHAPCharacteristicType_StatusFault = HAPUUIDCreateAppleDefined(0x77);

const HAPUUID kHAPCharacteristicType_StatusJammed = HAPUUIDCreateAppleDefined(0x78);

const HAPUUID kHAPCharacteristicType_StatusLowBattery = HAPUUIDCreateAppleDefined(0x79);

const HAPUUID kHAPCharacteristicType_StatusTampered = HAPUUIDCreateAppleDefined(0x7A);

const HAPUUID kHAPCharacteristicType_TargetHorizontalTiltAngle = HAPUUIDCreateAppleDefined(0x7B);

const HAPUUID kHAPCharacteristicType_TargetPosition = HAPUUIDCreateAppleDefined(0x7C);

const HAPUUID kHAPCharacteristicType_TargetVerticalTiltAngle = HAPUUIDCreateAppleDefined(0x7D);

const HAPUUID kHAPCharacteristicType_SecuritySystemAlarmType = HAPUUIDCreateAppleDefined(0x8E);

const HAPUUID kHAPCharacteristicType_ChargingState = HAPUUIDCreateAppleDefined(0x8F);

const HAPUUID kHAPCharacteristicType_CarbonMonoxideLevel = HAPUUIDCreateAppleDefined(0x90);

const HAPUUID kHAPCharacteristicType_CarbonMonoxidePeakLevel = HAPUUIDCreateAppleDefined(0x91);

const HAPUUID kHAPCharacteristicType_CarbonDioxideDetected = HAPUUIDCreateAppleDefined(0x92);

const HAPUUID kHAPCharacteristicType_CarbonDioxideLevel = HAPUUIDCreateAppleDefined(0x93);

const HAPUUID kHAPCharacteristicType_CarbonDioxidePeakLevel = HAPUUIDCreateAppleDefined(0x94);

const HAPUUID kHAPCharacteristicType_AirQuality = HAPUUIDCreateAppleDefined(0x95);

const HAPUUID kHAPCharacteristicType_ServiceSignature = HAPUUIDCreateAppleDefined(0xA5);

const HAPUUID kHAPCharacteristicType_AccessoryFlags = HAPUUIDCreateAppleDefined(0xA6);

const HAPUUID kHAPCharacteristicType_LockPhysicalControls = HAPUUIDCreateAppleDefined(0xA7);

const HAPUUID kHAPCharacteristicType_TargetAirPurifierState = HAPUUIDCreateAppleDefined(0xA8);

const HAPUUID kHAPCharacteristicType_CurrentAirPurifierState = HAPUUIDCreateAppleDefined(0xA9);

const HAPUUID kHAPCharacteristicType_CurrentSlatState = HAPUUIDCreateAppleDefined(0xAA);

const HAPUUID kHAPCharacteristicType_FilterLifeLevel = HAPUUIDCreateAppleDefined(0xAB);

const HAPUUID kHAPCharacteristicType_FilterChangeIndication = HAPUUIDCreateAppleDefined(0xAC);

const HAPUUID kHAPCharacteristicType_ResetFilterIndication = HAPUUIDCreateAppleDefined(0xAD);

const HAPUUID kHAPCharacteristicType_CurrentFanState = HAPUUIDCreateAppleDefined(0xAF);

const HAPUUID kHAPCharacteristicType_Active = HAPUUIDCreateAppleDefined(0xB0);

const HAPUUID kHAPCharacteristicType_CurrentHeaterCoolerState = HAPUUIDCreateAppleDefined(0xB1);

const HAPUUID kHAPCharacteristicType_TargetHeaterCoolerState = HAPUUIDCreateAppleDefined(0xB2);

const HAPUUID kHAPCharacteristicType_CurrentHumidifierDehumidifierState = HAPUUIDCreateAppleDefined(0xB3);

const HAPUUID kHAPCharacteristicType_TargetHumidifierDehumidifierState = HAPUUIDCreateAppleDefined(0xB4);

const HAPUUID kHAPCharacteristicType_WaterLevel = HAPUUIDCreateAppleDefined(0xB5);

const HAPUUID kHAPCharacteristicType_SwingMode = HAPUUIDCreateAppleDefined(0xB6);

const HAPUUID kHAPCharacteristicType_TargetFanState = HAPUUIDCreateAppleDefined(0xBF);

const HAPUUID kHAPCharacteristicType_SlatType = HAPUUIDCreateAppleDefined(0xC0);

const HAPUUID kHAPCharacteristicType_CurrentTiltAngle = HAPUUIDCreateAppleDefined(0xC1);

const HAPUUID kHAPCharacteristicType_TargetTiltAngle = HAPUUIDCreateAppleDefined(0xC2);

const HAPUUID kHAPCharacteristicType_OzoneDensity = HAPUUIDCreateAppleDefined(0xC3);

const HAPUUID kHAPCharacteristicType_NitrogenDioxideDensity = HAPUUIDCreateAppleDefined(0xC4);

const HAPUUID kHAPCharacteristicType_SulphurDioxideDensity = HAPUUIDCreateAppleDefined(0xC5);

const HAPUUID kHAPCharacteristicType_PM2_5Density = HAPUUIDCreateAppleDefined(0xC6);

const HAPUUID kHAPCharacteristicType_PM10Density = HAPUUIDCreateAppleDefined(0xC7);

const HAPUUID kHAPCharacteristicType_VOCDensity = HAPUUIDCreateAppleDefined(0xC8);

const HAPUUID kHAPCharacteristicType_RelativeHumidityDehumidifierThreshold = HAPUUIDCreateAppleDefined(0xC9);

const HAPUUID kHAPCharacteristicType_RelativeHumidityHumidifierThreshold = HAPUUIDCreateAppleDefined(0xCA);

const HAPUUID kHAPCharacteristicType_ServiceLabelIndex = HAPUUIDCreateAppleDefined(0xCB);

const HAPUUID kHAPCharacteristicType_ServiceLabelNamespace = HAPUUIDCreateAppleDefined(0xCD);

const HAPUUID kHAPCharacteristicType_ColorTemperature = HAPUUIDCreateAppleDefined(0xCE);

const HAPUUID kHAPCharacteristicType_ProgramMode = HAPUUIDCreateAppleDefined(0xD1);

const HAPUUID kHAPCharacteristicType_InUse = HAPUUIDCreateAppleDefined(0xD2);

const HAPUUID kHAPCharacteristicType_SetDuration = HAPUUIDCreateAppleDefined(0xD3);

const HAPUUID kHAPCharacteristicType_RemainingDuration = HAPUUIDCreateAppleDefined(0xD4);

const HAPUUID kHAPCharacteristicType_ValveType = HAPUUIDCreateAppleDefined(0xD5);

const HAPUUID kHAPCharacteristicType_IsConfigured = HAPUUIDCreateAppleDefined(0xD6);

const HAPUUID kHAPCharacteristicType_ActiveIdentifier = HAPUUIDCreateAppleDefined(0xE7);

const HAPUUID kHAPCharacteristicType_SupportedVideoStreamConfiguration = HAPUUIDCreateAppleDefined(0x114);

const HAPUUID kHAPCharacteristicType_SupportedAudioStreamConfiguration = HAPUUIDCreateAppleDefined(0x115);

const HAPUUID kHAPCharacteristicType_SupportedRTPConfiguration = HAPUUIDCreateAppleDefined(0x116);

const HAPUUID kHAPCharacteristicType_SelectedRTPStreamConfiguration = HAPUUIDCreateAppleDefined(0x117);

const HAPUUID kHAPCharacteristicType_SetupEndpoints = HAPUUIDCreateAppleDefined(0x118);

const HAPUUID kHAPCharacteristicType_Volume = HAPUUIDCreateAppleDefined(0x119);

const HAPUUID kHAPCharacteristicType_Mute = HAPUUIDCreateAppleDefined(0x11A);

const HAPUUID kHAPCharacteristicType_NightVision = HAPUUIDCreateAppleDefined(0x11B);

const HAPUUID kHAPCharacteristicType_OpticalZoom = HAPUUIDCreateAppleDefined(0x11C);

const HAPUUID kHAPCharacteristicType_DigitalZoom = HAPUUIDCreateAppleDefined(0x11D);

const HAPUUID kHAPCharacteristicType_ImageRotation = HAPUUIDCreateAppleDefined(0x11E);

const HAPUUID kHAPCharacteristicType_ImageMirroring = HAPUUIDCreateAppleDefined(0x11F);

const HAPUUID kHAPCharacteristicType_StreamingStatus = HAPUUIDCreateAppleDefined(0x120);

const HAPUUID kHAPCharacteristicType_TargetControlSupportedConfiguration = HAPUUIDCreateAppleDefined(0x123);

const HAPUUID kHAPCharacteristicType_TargetControlList = HAPUUIDCreateAppleDefined(0x124);

const HAPUUID kHAPCharacteristicType_ButtonEvent = HAPUUIDCreateAppleDefined(0x126);

const HAPUUID kHAPCharacteristicType_SelectedAudioStreamConfiguration = HAPUUIDCreateAppleDefined(0x128);

const HAPUUID kHAPCharacteristicType_SupportedDataStreamTransportConfiguration = HAPUUIDCreateAppleDefined(0x130);

const HAPUUID kHAPCharacteristicType_SetupDataStreamTransport = HAPUUIDCreateAppleDefined(0x131);

const HAPUUID kHAPCharacteristicType_SiriInputType = HAPUUIDCreateAppleDefined(0x132);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 34AB8811-AC7F-4340-BAC3-FD6A85F9943B
const HAPUUID kHAPCharacteristicType_ADKVersion = {
    { 0x3B, 0x94, 0xF9, 0x85, 0x6A, 0xFD, 0xC3, 0xBA, 0x40, 0x43, 0x7F, 0xAC, 0x11, 0x88, 0xAB, 0x34 }
};
