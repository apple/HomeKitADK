// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_SERVICE_TYPES_H
#define HAP_SERVICE_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Accessory Information.
 *
 * Every accessory must expose a single instance of the Accessory information service with the following definition.
 * The values of Manufacturer, Model, Name and Serial Number must be persistent through the lifetime of the accessory.
 *
 * Any other Apple-defined characteristics added to this service must only contain one or more of the following
 * permissions: Paired Read or Notify. Custom characteristics added to this service must only contain one or more of the
 * following permissions: Paired Read, Notify, Broadcast, and Hidden. All other permissions are not permitted.
 *
 * Required Characteristics:
 * - Firmware Revision
 * - Identify
 * - Manufacturer
 * - Model
 * - Name
 * - Serial Number
 *
 * Optional Characteristics:
 * - Accessory Flags
 * - Hardware Revision
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.1 Accessory Information
 */
/**@{*/
#define kHAPServiceDebugDescription_AccessoryInformation "accessory-information"

extern const HAPUUID kHAPServiceType_AccessoryInformation;
/**@}*/

/**
 * Garage Door Opener.
 *
 * This service describes a garage door opener that controls a single door. If a garage has more than one door, then
 * each door should have its own Garage Door Opener Service.
 *
 * Required Characteristics:
 * - Current Door State
 * - Target Door State
 * - Obstruction Detected
 *
 * Optional Characteristics:
 * - Lock Current State
 * - Lock Target State
 * - Name
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.16 Garage Door Opener
 */
/**@{*/
#define kHAPServiceDebugDescription_GarageDoorOpener "garage-door-opener"

extern const HAPUUID kHAPServiceType_GarageDoorOpener;
/**@}*/

/**
 * Light Bulb.
 *
 * This service describes a light bulb.
 *
 * Required Characteristics:
 * - On
 *
 * Optional Characteristics:
 * - Brightness
 * - Hue
 * - Name
 * - Saturation
 * - Color Temperature
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.23 Light Bulb
 */
/**@{*/
#define kHAPServiceDebugDescription_LightBulb "lightbulb"

extern const HAPUUID kHAPServiceType_LightBulb;

HAP_DEPRECATED_MSG("Use kHAPServiceDebugDescription_LightBulb instead.")
#define kHAPServiceDebugDescription_Lightbulb "lightbulb"

HAP_DEPRECATED_MSG("Use kHAPServiceType_LightBulb instead.")
extern const HAPUUID kHAPServiceType_Lightbulb;
/**@}*/

/**
 * Lock Management.
 *
 * The HomeKit Lock Management Service is designed to expose deeper interaction with a Lock device.
 *
 * Required Characteristics:
 * - Lock Control Point
 * - Version
 *
 * Optional Characteristics:
 * - Logs
 * - Audio Feedback
 * - Lock Management Auto Security Timeout
 * - Administrator Only Access
 * - Lock Last Known Action
 * - Current Door State
 * - Motion Detected
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.25 Lock Management
 */
/**@{*/
#define kHAPServiceDebugDescription_LockManagement "lock-management"

extern const HAPUUID kHAPServiceType_LockManagement;
/**@}*/

/**
 * Lock Mechanism.
 *
 * The HomeKit Lock Mechanism Service is designed to expose and control the physical lock mechanism on a device.
 *
 * Required Characteristics:
 * - Lock Current State
 * - Lock Target State
 *
 * Optional Characteristics:
 * - Name
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.26 Lock Mechanism
 */
/**@{*/
#define kHAPServiceDebugDescription_LockMechanism "lock-mechanism"

extern const HAPUUID kHAPServiceType_LockMechanism;
/**@}*/

/**
 * Outlet.
 *
 * This service describes a power outlet.
 *
 * Required Characteristics:
 * - On
 * - Outlet In Use
 *
 * Optional Characteristics:
 * - Name
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.30 Outlet
 */
/**@{*/
#define kHAPServiceDebugDescription_Outlet "outlet"

extern const HAPUUID kHAPServiceType_Outlet;
/**@}*/

/**
 * Switch.
 *
 * This service describes a binary switch.
 *
 * Required Characteristics:
 * - On
 *
 * Optional Characteristics:
 * - Name
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.38 Switch
 */
/**@{*/
#define kHAPServiceDebugDescription_Switch "switch"

extern const HAPUUID kHAPServiceType_Switch;
/**@}*/

/**
 * Thermostat.
 *
 * This service describes a thermostat.
 *
 * Required Characteristics:
 * - Current Heating Cooling State
 * - Target Heating Cooling State
 * - Current Temperature
 * - Target Temperature
 * - Temperature Display Units
 *
 * Optional Characteristics:
 * - Cooling Threshold Temperature
 * - Current Relative Humidity
 * - Heating Threshold Temperature
 * - Name
 * - Target Relative Humidity
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.42 Thermostat
 */
/**@{*/
#define kHAPServiceDebugDescription_Thermostat "thermostat"

extern const HAPUUID kHAPServiceType_Thermostat;
/**@}*/

/**
 * Pairing.
 *
 * Defines characteristics to support pairing between a controller and an accessory.
 *
 * Required Characteristics:
 * - Pair Setup
 * - Pair Verify
 * - Pairing Features
 * - Pairing Pairings
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 5.13.1 Pairing Service
 */
/**@{*/
#define kHAPServiceDebugDescription_Pairing "pairing"

extern const HAPUUID kHAPServiceType_Pairing;
/**@}*/

/**
 * Security System.
 *
 * This service describes a security system service.
 *
 * This service requires iOS 9 or later.
 *
 * Required Characteristics:
 * - Security System Current State
 * - Security System Target State
 *
 * Optional Characteristics:
 * - Name
 * - Security System Alarm Type
 * - Status Fault
 * - Status Tampered
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.31 Security System
 */
/**@{*/
#define kHAPServiceDebugDescription_SecuritySystem "security-system"

extern const HAPUUID kHAPServiceType_SecuritySystem;
/**@}*/

/**
 * Carbon Monoxide Sensor.
 *
 * This service describes a carbon monoxide sensor.
 *
 * This service requires iOS 9 or later.
 *
 * Required Characteristics:
 * - Carbon Monoxide Detected
 *
 * Optional Characteristics:
 * - Name
 * - Status Active
 * - Status Fault
 * - Status Tampered
 * - Status Low Battery
 * - Carbon Monoxide Level
 * - Carbon Monoxide Peak Level
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.8 Carbon Monoxide Sensor
 */
/**@{*/
#define kHAPServiceDebugDescription_CarbonMonoxideSensor "sensor.carbon-monoxide"

extern const HAPUUID kHAPServiceType_CarbonMonoxideSensor;
/**@}*/

/**
 * Contact Sensor.
 *
 * This service describes a Contact Sensor.
 *
 * This service requires iOS 9 or later.
 *
 * Required Characteristics:
 * - Contact Sensor State
 *
 * Optional Characteristics:
 * - Name
 * - Status Active
 * - Status Fault
 * - Status Tampered
 * - Status Low Battery
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.9 Contact Sensor
 */
/**@{*/
#define kHAPServiceDebugDescription_ContactSensor "sensor.contact"

extern const HAPUUID kHAPServiceType_ContactSensor;
/**@}*/

/**
 * Door.
 *
 * This service describes a motorized door.
 *
 * This service requires iOS 9 or later.
 *
 * Required Characteristics:
 * - Current Position
 * - Target Position
 * - Position State
 *
 * Optional Characteristics:
 * - Name
 * - Hold Position
 * - Obstruction Detected
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.11 Door
 */
/**@{*/
#define kHAPServiceDebugDescription_Door "door"

extern const HAPUUID kHAPServiceType_Door;
/**@}*/

/**
 * Humidity Sensor.
 *
 * This service describes a Humidity Sensor.
 *
 * This service requires iOS 9 or later.
 *
 * Required Characteristics:
 * - Current Relative Humidity
 *
 * Optional Characteristics:
 * - Name
 * - Status Active
 * - Status Fault
 * - Status Tampered
 * - Status Low Battery
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.20 Humidity Sensor
 */
/**@{*/
#define kHAPServiceDebugDescription_HumiditySensor "sensor.humidity"

extern const HAPUUID kHAPServiceType_HumiditySensor;
/**@}*/

/**
 * Leak Sensor.
 *
 * This service describes a leak sensor.
 *
 * This service requires iOS 9 or later.
 *
 * Required Characteristics:
 * - Leak Detected
 *
 * Optional Characteristics:
 * - Name
 * - Status Active
 * - Status Fault
 * - Status Tampered
 * - Status Low Battery
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.22 Leak Sensor
 */
/**@{*/
#define kHAPServiceDebugDescription_LeakSensor "sensor.leak"

extern const HAPUUID kHAPServiceType_LeakSensor;
/**@}*/

/**
 * Light Sensor.
 *
 * This service describes a light sensor.
 *
 * This service requires iOS 9 or later.
 *
 * Required Characteristics:
 * - Current Ambient Light Level
 *
 * Optional Characteristics:
 * - Name
 * - Status Active
 * - Status Fault
 * - Status Low Battery
 * - Status Tampered
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.24 Light Sensor
 */
/**@{*/
#define kHAPServiceDebugDescription_LightSensor "sensor.light"

extern const HAPUUID kHAPServiceType_LightSensor;
/**@}*/

/**
 * Motion Sensor.
 *
 * This service describes a motion sensor.
 *
 * This service requires iOS 9 or later.
 *
 * Required Characteristics:
 * - Motion Detected
 *
 * Optional Characteristics:
 * - Name
 * - Status Active
 * - Status Fault
 * - Status Tampered
 * - Status Low Battery
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.28 Motion Sensor
 */
/**@{*/
#define kHAPServiceDebugDescription_MotionSensor "sensor.motion"

extern const HAPUUID kHAPServiceType_MotionSensor;
/**@}*/

/**
 * Occupancy Sensor.
 *
 * This service describes an occupancy sensor.
 *
 * This service requires iOS 9 or later.
 *
 * Required Characteristics:
 * - Occupancy Detected
 *
 * Optional Characteristics:
 * - Name
 * - Status Active
 * - Status Fault
 * - Status Tampered
 * - Status Low Battery
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.29 Occupancy Sensor
 */
/**@{*/
#define kHAPServiceDebugDescription_OccupancySensor "sensor.occupancy"

extern const HAPUUID kHAPServiceType_OccupancySensor;
/**@}*/

/**
 * Smoke Sensor.
 *
 * This service describes a Smoke detector Sensor.
 *
 * This service requires iOS 9 or later.
 *
 * Required Characteristics:
 * - Smoke Detected
 *
 * Optional Characteristics:
 * - Name
 * - Status Active
 * - Status Fault
 * - Status Tampered
 * - Status Low Battery
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.35 Smoke Sensor
 */
/**@{*/
#define kHAPServiceDebugDescription_SmokeSensor "sensor.smoke"

extern const HAPUUID kHAPServiceType_SmokeSensor;
/**@}*/

/**
 * Stateless Programmable Switch.
 *
 * This service describes a stateless programmable switch.
 *
 * The following rules apply to a stateless programmable switch accessory:
 * - Each physical switch on the accessory must be represented by a unique instance of this service.
 * - If there are multiple instances of this service on the accessory, they must be linked to a `Service Label`.
 * - If there are multiple instances of this service on the accessory,
 *   `Service Label Index` is a required characteristic.
 * - `Service Label Index` value for each instance of this service linked to the same `Service Label` must be unique.
 * - The User visible label on the physical accessory should match the `Service Label Namespace`
 *   described by the accessory.
 * - If there is only one instance of this service on the accessory, `Service Label` is not required and consequently
 *   `Service Label Index` must not be present.
 *
 * This service requires iOS 10.3 or later.
 *
 * Required Characteristics:
 * - Programmable Switch Event
 *
 * Optional Characteristics:
 * - Name
 * - Service Label Index
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.37 Stateless Programmable Switch
 */
/**@{*/
#define kHAPServiceDebugDescription_StatelessProgrammableSwitch "stateless-programmable-switch"

extern const HAPUUID kHAPServiceType_StatelessProgrammableSwitch;
/**@}*/

/**
 * Temperature Sensor.
 *
 * This service describes a Temperature Sensor.
 *
 * This service requires iOS 9 or later.
 *
 * Required Characteristics:
 * - Current Temperature
 *
 * Optional Characteristics:
 * - Name
 * - Status Active
 * - Status Fault
 * - Status Low Battery
 * - Status Tampered
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.41 Temperature Sensor
 */
/**@{*/
#define kHAPServiceDebugDescription_TemperatureSensor "sensor.temperature"

extern const HAPUUID kHAPServiceType_TemperatureSensor;
/**@}*/

/**
 * Window.
 *
 * This service describes a motorized window.
 *
 * This service requires iOS 9 or later.
 *
 * Required Characteristics:
 * - Current Position
 * - Target Position
 * - Position State
 *
 * Optional Characteristics:
 * - Name
 * - Hold Position
 * - Obstruction Detected
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.44 Window
 */
/**@{*/
#define kHAPServiceDebugDescription_Window "window"

extern const HAPUUID kHAPServiceType_Window;
/**@}*/

/**
 * Window Covering.
 *
 * This service describes motorized window coverings or shades - examples include shutters, blinds, awnings etc.
 *
 * This service requires iOS 9 or later.
 *
 * Required Characteristics:
 * - Target Position
 * - Current Position
 * - Position State
 *
 * Optional Characteristics:
 * - Name
 * - Hold Position
 * - Current Horizontal Tilt Angle
 * - Target Horizontal Tilt Angle
 * - Current Vertical Tilt Angle
 * - Target Vertical Tilt Angle
 * - Obstruction Detected
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.45 Window Covering
 */
/**@{*/
#define kHAPServiceDebugDescription_WindowCovering "window-covering"

extern const HAPUUID kHAPServiceType_WindowCovering;
/**@}*/

/**
 * Air Quality Sensor.
 *
 * This service describes an air quality sensor. `Air Quality` refers to the cumulative air quality recorded
 * by the accessory which may be based on multiple sensors present.
 *
 * This service requires iOS 9 or later and is updated in iOS 10.
 *
 * Required Characteristics:
 * - Air Quality
 *
 * Optional Characteristics:
 * - Name
 * - Ozone Density
 * - Nitrogen Dioxide Density
 * - Sulphur Dioxide Density
 * - PM2.5 Density
 * - PM10 Density
 * - VOC Density
 * - Status Active
 * - Status Fault
 * - Status Tampered
 * - Status Low Battery
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.3 Air Quality Sensor
 */
/**@{*/
#define kHAPServiceDebugDescription_AirQualitySensor "sensor.air-quality"

extern const HAPUUID kHAPServiceType_AirQualitySensor;
/**@}*/

/**
 * Battery Service.
 *
 * This service describes a battery service.
 *
 * This service requires iOS 9 or later.
 *
 * Required Characteristics:
 * - Battery Level
 * - Charging State
 * - Status Low Battery
 *
 * Optional Characteristics:
 * - Name
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.5 Battery Service
 */
/**@{*/
#define kHAPServiceDebugDescription_BatteryService "battery"

extern const HAPUUID kHAPServiceType_BatteryService;
/**@}*/

/**
 * Carbon Dioxide Sensor.
 *
 * This service describes a carbon dioxide sensor.
 *
 * This service requires iOS 9 or later.
 *
 * Required Characteristics:
 * - Carbon Dioxide Detected
 *
 * Optional Characteristics:
 * - Name
 * - Status Active
 * - Status Fault
 * - Status Tampered
 * - Status Low Battery
 * - Carbon Dioxide Level
 * - Carbon Dioxide Peak Level
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.7 Carbon Dioxide Sensor
 */
/**@{*/
#define kHAPServiceDebugDescription_CarbonDioxideSensor "sensor.carbon-dioxide"

extern const HAPUUID kHAPServiceType_CarbonDioxideSensor;
/**@}*/

/**
 * HAP Protocol Information.
 *
 * Every accessory must expose a single instance of the HAP protocol information. For a bridge accessory, only the
 * primary HAP accessory object must contain this service. The `Version` value is transport dependent. Refer to
 * BLE Protocol Version Characteristic for BLE protocol version. Refer to IP Protocol Version for IP protocol version.
 *
 * Required Characteristics:
 * - Version
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.17 HAP Protocol Information
 */
/**@{*/
#define kHAPServiceDebugDescription_HAPProtocolInformation "protocol.information.service"

extern const HAPUUID kHAPServiceType_HAPProtocolInformation;
/**@}*/

/**
 * Fan.
 *
 * This service describes a fan.
 *
 * If the fan service is included in air purifier accessories, `Current Fan State` and `Target Fan State`
 * are required characteristics.
 *
 * This service requires iOS 10.3 or later.
 *
 * Required Characteristics:
 * - Active
 *
 * Optional Characteristics:
 * - Name
 * - Current Fan State
 * - Target Fan State
 * - Rotation Direction
 * - Rotation Speed
 * - Swing Mode
 * - Lock Physical Controls
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.13 Fan
 */
/**@{*/
#define kHAPServiceDebugDescription_Fan "fanv2"

extern const HAPUUID kHAPServiceType_Fan;

HAP_DEPRECATED_MSG("Use kHAPServiceDebugDescription_Fan instead.")
#define kHAPServiceDebugDescription_FanV2 "fanv2"

HAP_DEPRECATED_MSG("Use kHAPServiceType_Fan instead.")
extern const HAPUUID kHAPServiceType_FanV2;
/**@}*/

/**
 * Slat.
 *
 * This service describes a slat which tilts on a vertical or a horizontal axis.
 *
 * `Current Tilt Angle` and `Target Tilt Angle` may be included in this service if the user can set the slats to a
 * particular tilt angle.
 *
 * `Swing Mode` implies that the slats can swing automatically (e.g. vents on a fan).
 *
 * This service requires iOS 10.3 or later.
 *
 * Required Characteristics:
 * - Current Slat State
 * - Slat Type
 *
 * Optional Characteristics:
 * - Name
 * - Swing Mode
 * - Current Tilt Angle
 * - Target Tilt Angle
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.34 Slat
 */
/**@{*/
#define kHAPServiceDebugDescription_Slat "vertical-slat"

extern const HAPUUID kHAPServiceType_Slat;
/**@}*/

/**
 * Filter Maintenance.
 *
 * This service can be used to describe maintenance operations for a filter.
 *
 * This service requires iOS 10.3 or later.
 *
 * Required Characteristics:
 * - Filter Change Indication
 *
 * Optional Characteristics:
 * - Name
 * - Filter Life Level
 * - Reset Filter Indication
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.15 Filter Maintenance
 */
/**@{*/
#define kHAPServiceDebugDescription_FilterMaintenance "filter-maintenance"

extern const HAPUUID kHAPServiceType_FilterMaintenance;
/**@}*/

/**
 * Air Purifier.
 *
 * This service describes an air purifier. An air purifier accessory can have additional linked services such as:
 * - `Filter Maintenance` service(s) to describe one or more air filters.
 * - `Air Quality Sensor` services to describe air quality sensors.
 * - `Fan` service to describe a fan which can be independently controlled.
 * - `Slat` service to control vents.
 *
 * If `Fan` is included as a linked service in an air purifier accessory:
 * - Changing `Active` characteristic on the `Air Purifier` must result in corresponding change to
 *   `Active` characteristic on the `Fan`.
 * - Changing `Active` characteristic on the `Fan` from "Inactive" to "Active" does not require the `Active` on the
 *   `Air Purifier` to change. This enables "Fan Only" mode on air purifier.
 * - Changing `Active` characteristic on the `Fan` from "Active" to "Inactive" must result in the `Active` on the
 *   `Air Purifier` to change to "Inactive".
 *
 * An air purifier accessory service may include `Rotation Speed` to control fan speed
 * if the fan cannot be independently controlled.
 *
 * This service requires iOS 10.3 or later.
 *
 * Required Characteristics:
 * - Active
 * - Current Air Purifier State
 * - Target Air Purifier State
 *
 * Optional Characteristics:
 * - Name
 * - Rotation Speed
 * - Swing Mode
 * - Lock Physical Controls
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.2 Air Purifier
 */
/**@{*/
#define kHAPServiceDebugDescription_AirPurifier "air-purifier"

extern const HAPUUID kHAPServiceType_AirPurifier;
/**@}*/

/**
 * Heater Cooler.
 *
 * This service can be used to describe either of the following:
 * - a heater
 * - a cooler
 * - a heater and a cooler
 *
 * A heater/cooler accessory may have additional:
 * - `Fan` service to describe a fan which can be independently controlled
 * - `Slat` service to control vents
 *
 * A heater must include `Heating Threshold Temperature`. A cooler must include `Cooling Threshold Temperature`.
 *
 * A heater/cooler accessory service may include `Rotation Speed` to control fan speed if the fan cannot be
 * independently controlled.
 *
 * This service requires iOS 10.3 or later.
 *
 * Required Characteristics:
 * - Active
 * - Current Temperature
 * - Current Heater Cooler State
 * - Target Heater Cooler State
 *
 * Optional Characteristics:
 * - Name
 * - Rotation Speed
 * - Temperature Display Units
 * - Swing Mode
 * - Cooling Threshold Temperature
 * - Heating Threshold Temperature
 * - Lock Physical Controls
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.18 Heater Cooler
 */
/**@{*/
#define kHAPServiceDebugDescription_HeaterCooler "heater-cooler"

extern const HAPUUID kHAPServiceType_HeaterCooler;
/**@}*/

/**
 * Humidifier Dehumidifier.
 *
 * This service can be used to describe either of the following:
 * - an air humidifier
 * - an air dehumidifier
 * - an air humidifier and an air dehumidifier
 *
 * An air humidifier/dehumidifier accessory may have additional:
 * - `Fan` service to describe a fan which can be independently controlled
 * - `Slat` service to control vents
 *
 * A dehumidifier must include `Relative Humidity Dehumidifier Threshold`.
 * A humidifier must include `Relative Humidity Humidifier Threshold`.
 *
 * A humidifier/dehumidifier accessory service may include `Rotation Speed` to control fan speed if the fan cannot be
 * independently controlled.
 *
 * This service requires iOS 10.3 or later.
 *
 * Required Characteristics:
 * - Active
 * - Current Relative Humidity
 * - Current Humidifier Dehumidifier State
 * - Target Humidifier Dehumidifier State
 *
 * Optional Characteristics:
 * - Name
 * - Relative Humidity Dehumidifier Threshold
 * - Relative Humidity Humidifier Threshold
 * - Rotation Speed
 * - Swing Mode
 * - Water Level
 * - Lock Physical Controls
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.19 Humidifier Dehumidifier
 */
/**@{*/
#define kHAPServiceDebugDescription_HumidifierDehumidifier "humidifier-dehumidifier"

extern const HAPUUID kHAPServiceType_HumidifierDehumidifier;
/**@}*/

/**
 * Service Label.
 *
 * This service describes label scheme.
 *
 * This service requires iOS 10.3 or later.
 *
 * Required Characteristics:
 * - Service Label Namespace
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.32 Service Label
 */
/**@{*/
#define kHAPServiceDebugDescription_ServiceLabel "service-label"

extern const HAPUUID kHAPServiceType_ServiceLabel;
/**@}*/

/**
 * Irrigation System.
 *
 * This service describes an irrigation system. This service must be present on an irrigation systems which supports
 * on-device schedules or supports a top-level `Active` control across multiple valves.
 *
 * A sprinkler system accessory maybe:
 *
 * - a combination of `Irrigation System` service on a bridge accessory with a collection of one or more `Valve`
 *   services (with `Valve Type` set to "Irrigation") as bridged accessories (The bridge accessory is typically
 *   connected to each valve using wires). OR
 *
 * - a combination of `Irrigation System` service with a collection of one or more linked `Valve` services (with
 *   `Valve Type` set to "Irrigation") (The valves are collocated e.g. hose based system). OR
 *
 * - a combination of `Valve` service(s) with `Valve Type` set to "Irrigation" (e.g., a system with one or more valves
 *   which does not support scheduling)
 *
 * An irrigation system is set to "Active" when the system is enabled. When one of the valves is set to "In Use", the
 * irrigation system must be set to in use.
 *
 * An accessory which includes this services must include the `Set Duration` in the `Valve`.
 *
 * An irrigation system accessory which does not auto detect the number of valves it is connected to and requires user
 * to provide this information must include the `Is Configured` in the `Valve`.
 *
 * `Remaining Duration` on this service implies the total remaining duration to water across all the valves.
 *
 * This service requires iOS 11.2 or later.
 *
 * Required Characteristics:
 * - Active
 * - Program Mode
 * - In Use
 *
 * Optional Characteristics:
 * - Remaining Duration
 * - Name
 * - Status Fault
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.21 Irrigation System
 */
/**@{*/
#define kHAPServiceDebugDescription_IrrigationSystem "irrigation-system"

extern const HAPUUID kHAPServiceType_IrrigationSystem;
/**@}*/

/**
 * Valve.
 *
 * This service describes accessories like irrigation valves or water outlets. A valve is set to "In Use" when there are
 * fluid flowing through the valve.
 *
 * If an accessory has this service with `Valve Type` set to Irrigation it must include the `Set Duration` and
 * `Remaining Duration` characteristic on the `Valve` service.
 *
 * `Service Label Index` must be present on each instance of this service if the accessory consists of:
 *
 * - a bridge accessory (the `Service Label` service must be included here) which includes multiple bridged accessories
 *   each with `Valve` service.
 *
 * - an accessory (the `Service Label` service must be included here) which includes multiple linked `Valve` services.
 *
 * If an accessory has this service with `Service Label Index` included, the default `Name` must be empty string unless
 * user has already assigned a name to this valve before accessory is HomeKit paired. In such a case, the default name
 * should be the user configured name for this valve.
 *
 * `Is Configured` must be present on each instance of this service if the accessory is used in an irrigation system or
 * shower system where all valves may not be configured to use (e.g., depending on physical wire connection).
 *
 * Setting the value of `Active` to "Active" on this service must result in `Irrigation System` bridge to be set to
 * "Active" if this service is used in context of an Irrigation system.
 *
 * This service requires iOS 11.2 or later.
 *
 * Required Characteristics:
 * - Active
 * - In Use
 * - Valve Type
 *
 * Optional Characteristics:
 * - Set Duration
 * - Remaining Duration
 * - Is Configured
 * - Service Label Index
 * - Status Fault
 * - Name
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.43 Valve
 */
/**@{*/
#define kHAPServiceDebugDescription_Valve "valve"

extern const HAPUUID kHAPServiceType_Valve;
/**@}*/

/**
 * Faucet.
 *
 * This service describes accessories like faucets or shower heads. This service must only be included when an accessory
 * has either a linked `Heater Cooler` with single linked `Valve` service or multiple linked `Valve` services
 * (with/without `Heater Cooler` service) to describe water outlets. This service serves as a top level service for such
 * accessories.
 *
 * A faucet which supports heating of water must include `Heater Cooler` and `Valve` service as linked services. An
 * accessory which supports one or multiple water outlets and heating of water through a common temperature control,
 * must include `Heater Cooler` and `Valve` service(s) as linked services to the faucet service.
 *
 * Setting the value of `Active` to "InActive" on this service must turn off the faucet accessory. The accessory must
 * retain the state of `Active` characteristics on any linked `Valve` services when the `Active` on this service is
 * toggled. The accessory must set the value of `Active` to "InActive" of any linked `Heater Cooler` service when the
 * `Active` on this service is set to "InActive".
 *
 * This service requires iOS 11.2 or later.
 *
 * Required Characteristics:
 * - Active
 *
 * Optional Characteristics:
 * - Status Fault
 * - Name
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 8.14 Faucet
 */
/**@{*/
#define kHAPServiceDebugDescription_Faucet "faucet"

extern const HAPUUID kHAPServiceType_Faucet;
/**@}*/

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
