// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_CHARACTERISTIC_TYPES_H
#define HAP_CHARACTERISTIC_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Administrator Only Access.
 *
 * When this mode is enabled, the accessory only accepts administrator access.
 *
 * - Format: Bool
 * - Permissions: Paired Read, Paired Write, Notify
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.5 Administrator Only Access
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_AdministratorOnlyAccess "administrator-only-access"

extern const HAPUUID kHAPCharacteristicType_AdministratorOnlyAccess;
/**@}*/

/**
 * Audio Feedback.
 *
 * This characteristic describes whether audio feedback (e.g. a beep, or other external sound mechanism) is enabled.
 *
 * - Format: Bool
 * - Permissions: Paired Read, Paired Write, Notify
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.6 Audio Feedback
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_AudioFeedback "audio-feedback"

extern const HAPUUID kHAPCharacteristicType_AudioFeedback;
/**@}*/

/**
 * Brightness.
 *
 * This characteristic describes a perceived level of brightness, e.g. for lighting, and can be used for backlights or
 * color. The value is expressed as a percentage (%) of the maximum level of supported brightness.
 *
 * - Format: Int
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 0
 * - Maximum Value: 100
 * - Step Value: 1
 * - Unit: Percentage
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.11 Brightness
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_Brightness "brightness"

extern const HAPUUID kHAPCharacteristicType_Brightness;
/**@}*/

/**
 * Cooling Threshold Temperature.
 *
 * This characteristic describes the cooling threshold in Celsius for accessories that support simultaneous heating and
 * cooling. The value of this characteristic represents the maximum temperature that must be reached before cooling is
 * turned on.
 *
 * For example, if the `Target Heating Cooling State` is set to "Auto" and the current temperature goes above the
 * maximum temperature, then the cooling mechanism should turn on to decrease the current temperature until the
 * minimum temperature is reached.
 *
 * - Format: Float
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 10
 * - Maximum Value: 35
 * - Step Value: 0.1
 * - Unit: Celsius
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.20 Cooling Threshold Temperature
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_CoolingThresholdTemperature "temperature.cooling-threshold"

extern const HAPUUID kHAPCharacteristicType_CoolingThresholdTemperature;
/**@}*/

/**
 * Current Door State.
 *
 * This characteristic describes the current state of a door.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 4
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.30 Current Door State
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_CurrentDoorState "door-state.current"

extern const HAPUUID kHAPCharacteristicType_CurrentDoorState;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_CurrentDoorState) {
    /** Open. The door is fully open. */
    kHAPCharacteristicValue_CurrentDoorState_Open = 0,

    /** Closed. The door is fully closed. */
    kHAPCharacteristicValue_CurrentDoorState_Closed = 1,

    /** Opening. The door is actively opening. */
    kHAPCharacteristicValue_CurrentDoorState_Opening = 2,

    /** Closing. The door is actively closing. */
    kHAPCharacteristicValue_CurrentDoorState_Closing = 3,

    /** Stopped. The door is not moving, and it is not fully open nor fully closed. */
    kHAPCharacteristicValue_CurrentDoorState_Stopped = 4
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_CurrentDoorState);
/**@}*/

/**
 * Current Heating Cooling State.
 *
 * This characteristic describes the current mode of an accessory that supports cooling or heating its environment,
 * e.g. a thermostat is "heating" a room to 75 degrees Fahrenheit.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 2
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.32 Current Heating Cooling State
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_CurrentHeatingCoolingState "heating-cooling.current"

extern const HAPUUID kHAPCharacteristicType_CurrentHeatingCoolingState;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_CurrentHeatingCoolingState) {
    /** Off. */
    kHAPCharacteristicValue_CurrentHeatingCoolingState_Off = 0,

    /** Heat. The Heater is currently on. */
    kHAPCharacteristicValue_CurrentHeatingCoolingState_Heat = 1,

    /** Cool. Cooler is currently on. */
    kHAPCharacteristicValue_CurrentHeatingCoolingState_Cool = 2
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_CurrentHeatingCoolingState);
/**@}*/

/**
 * Current Relative Humidity.
 *
 * This characteristic describes the current relative humidity of the accessory's environment.
 * The value is expressed as a percentage (%).
 *
 * - Format: Float
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 100
 * - Step Value: 1
 * - Unit: Percentage
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.34 Current Relative Humidity
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_CurrentRelativeHumidity "relative-humidity.current"

extern const HAPUUID kHAPCharacteristicType_CurrentRelativeHumidity;
/**@}*/

/**
 * Current Temperature.
 *
 * This characteristic describes the current temperature of the environment in Celsius irrespective of display units
 * chosen in `Temperature Display Units`.
 *
 * - Format: Float
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 100
 * - Step Value: 0.1
 * - Unit: Celsius
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.35 Current Temperature
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_CurrentTemperature "temperature.current"

extern const HAPUUID kHAPCharacteristicType_CurrentTemperature;
/**@}*/

/**
 * Heating Threshold Temperature.
 *
 * This characteristic describes the heating threshold in Celsius for accessories that support simultaneous heating and
 * cooling. The value of this characteristic represents the minimum temperature that must be reached before heating is
 * turned on.
 *
 * For example, if the `Target Heating Cooling State` is set to "Auto" and the current temperature goes below the
 * minimum temperature, then the heating mechanism should turn on to increase the current temperature until the
 * minimum temperature is reached.
 *
 * - Format: Float
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 0
 * - Maximum Value: 25
 * - Step Value: 0.1
 * - Unit: Celsius
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.42 Heating Threshold Temperature
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_HeatingThresholdTemperature "temperature.heating-threshold"

extern const HAPUUID kHAPCharacteristicType_HeatingThresholdTemperature;
/**@}*/

/**
 * Hue.
 *
 * This characteristic describes hue or color.
 *
 * - Format: Float
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 0
 * - Maximum Value: 360
 * - Step Value: 1
 * - Unit: Arcdegrees
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.44 Hue
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_Hue "hue"

extern const HAPUUID kHAPCharacteristicType_Hue;
/**@}*/

/**
 * Identify.
 *
 * This characteristic is used to cause the accessory to run its identify routine.
 *
 * Only the `Accessory Information` is allowed to contain the Identify characteristic.
 *
 * - Format: Bool
 * - Permissions: Paired Write
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.45 Identify
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_Identify "identify"

extern const HAPUUID kHAPCharacteristicType_Identify;
/**@}*/

/**
 * Lock Control Point.
 *
 * The accessory accepts writes to this characteristic to perform vendor-specific actions as well as those defined by
 * the `Lock Management` of the `Lock`. For example, user management related functions should be defined and performed
 * using this characteristic.
 *
 * - Format: TLV
 * - Permissions: Paired Write
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.51 Lock Control Point
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_LockControlPoint "lock-management.control-point"

extern const HAPUUID kHAPCharacteristicType_LockControlPoint;
/**@}*/

/**
 * Lock Management Auto Security Timeout.
 *
 * A value greater than 0 indicates if the lock mechanism enters the unsecured state, it will automatically attempt to
 * enter the secured state after n seconds, where n is the value provided in the write. A value of 0 indicates this
 * feature is disabled.
 *
 * - Format: UInt32
 * - Permissions: Paired Read, Paired Write, Notify
 * - Unit: Seconds
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.54 Lock Management Auto Security Timeout
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_LockManagementAutoSecurityTimeout "lock-management.auto-secure-timeout"

extern const HAPUUID kHAPCharacteristicType_LockManagementAutoSecurityTimeout;
/**@}*/

/**
 * Lock Last Known Action.
 *
 * The last known action of the lock mechanism (e.g. deadbolt).
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 8
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.53 Lock Last Known Action
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_LockLastKnownAction "lock-mechanism.last-known-action"

extern const HAPUUID kHAPCharacteristicType_LockLastKnownAction;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_LockLastKnownAction) {
    /** Secured using physical movement, interior. */
    kHAPCharacteristicValue_LockLastKnownAction_SecuredInterior = 0,

    /** Unsecured using physical movement, interior. */
    kHAPCharacteristicValue_LockLastKnownAction_UnsecuredInterior = 1,

    /** Secured using physical movement, exterior. */
    kHAPCharacteristicValue_LockLastKnownAction_SecuredExterior = 2,

    /** Unsecured using physical movement, exterior. */
    kHAPCharacteristicValue_LockLastKnownAction_UnsecuredExterior = 3,

    /** Secured with keypad. */
    kHAPCharacteristicValue_LockLastKnownAction_SecuredKeypad = 4,

    /** Unsecured with keypad. */
    kHAPCharacteristicValue_LockLastKnownAction_UnsecuredKeypad = 5,

    /** Secured remotely. */
    kHAPCharacteristicValue_LockLastKnownAction_SecuredRemote = 6,

    /** Unsecured remotely. */
    kHAPCharacteristicValue_LockLastKnownAction_UnsecuredRemote = 7,

    /** Secured with Automatic Secure timeout. */
    kHAPCharacteristicValue_LockLastKnownAction_SecuredTimeout = 8
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_LockLastKnownAction);
/**@}*/

/**
 * Lock Current State.
 *
 * The current state of the physical security mechanism (e.g. deadbolt).
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 3
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.52 Lock Current State
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_LockCurrentState "lock-mechanism.current-state"

extern const HAPUUID kHAPCharacteristicType_LockCurrentState;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_LockCurrentState) {
    /** Unsecured. */
    kHAPCharacteristicValue_LockCurrentState_Unsecured = 0,

    /** Secured. */
    kHAPCharacteristicValue_LockCurrentState_Secured = 1,

    /** Jammed. */
    kHAPCharacteristicValue_LockCurrentState_Jammed = 2,

    /** Unknown. */
    kHAPCharacteristicValue_LockCurrentState_Unknown = 3
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_LockCurrentState);
/**@}*/

/**
 * Lock Target State.
 *
 * The target state of the physical security mechanism (e.g. deadbolt).
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.56 Lock Target State
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_LockTargetState "lock-mechanism.target-state"

extern const HAPUUID kHAPCharacteristicType_LockTargetState;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_LockTargetState) { /** Unsecured. */
                                                                  kHAPCharacteristicValue_LockTargetState_Unsecured = 0,

                                                                  /** Secured. */
                                                                  kHAPCharacteristicValue_LockTargetState_Secured = 1
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_LockTargetState);
/**@}*/

/**
 * Logs.
 *
 * Read from this characteristic to get timestamped logs from the device. The data is in TLV8 format as defined by the
 * associated service profile. The `Lock Management`, for example, defines its own specific structure for the log data.
 *
 * - Format: TLV
 * - Permissions: Paired Read, Notify
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.57 Logs
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_Logs "logs"

extern const HAPUUID kHAPCharacteristicType_Logs;
/**@}*/

/**
 * Manufacturer.
 *
 * This characteristic contains the name of the company whose brand will appear on the accessory, e.g., "Acme".
 *
 * - Format: String
 * - Permissions: Paired Read
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.58 Manufacturer
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_Manufacturer "manufacturer"

extern const HAPUUID kHAPCharacteristicType_Manufacturer;
/**@}*/

/**
 * Model.
 *
 * This characteristic contains the manufacturer-specific model of the accessory, e.g. "A1234". The minimum length of
 * this characteristic must be 1.
 *
 * - Format: String
 * - Permissions: Paired Read
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.59 Model
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_Model "model"

extern const HAPUUID kHAPCharacteristicType_Model;
/**@}*/

/**
 * Motion Detected.
 *
 * This characteristic indicates if motion (e.g. a person moving) was detected.
 *
 * - Format: Bool
 * - Permissions: Paired Read, Notify
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.60 Motion Detected
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_MotionDetected "motion-detected"

extern const HAPUUID kHAPCharacteristicType_MotionDetected;
/**@}*/

/**
 * Name.
 *
 * This characteristic describes a name and must not be a null value.
 *
 * - Format: String
 * - Permissions: Paired Read
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.62 Name
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_Name "name"

extern const HAPUUID kHAPCharacteristicType_Name;
/**@}*/

/**
 * Obstruction Detected.
 *
 * This characteristic describes the current state of an obstruction sensor, such as one that is used in a garage door.
 * If the state is true then there is an obstruction detected.
 *
 * - Format: Bool
 * - Permissions: Paired Read, Notify
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.65 Obstruction Detected
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_ObstructionDetected "obstruction-detected"

extern const HAPUUID kHAPCharacteristicType_ObstructionDetected;
/**@}*/

/**
 * On.
 *
 * This characteristic represents the states for "on" and "off".
 *
 * - Format: Bool
 * - Permissions: Paired Read, Paired Write, Notify
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.70 On
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_On "on"

extern const HAPUUID kHAPCharacteristicType_On;
/**@}*/

/**
 * Outlet In Use.
 *
 * This characteristic describes if the power outlet has an appliance e.g., a floor lamp, physically plugged in. This
 * characteristic is set to "True" even if the plugged-in appliance is off.
 *
 * - Format: Bool
 * - Permissions: Paired Read, Notify
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.69 Outlet In Use
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_OutletInUse "outlet-in-use"

extern const HAPUUID kHAPCharacteristicType_OutletInUse;
/**@}*/

/**
 * Rotation Direction.
 *
 * This characteristic describes the direction of rotation of a fan.
 *
 * - Format: Int
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.80 Rotation Direction
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_RotationDirection "rotation.direction"

extern const HAPUUID kHAPCharacteristicType_RotationDirection;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_RotationDirection) {
    /** Clockwise. */
    kHAPCharacteristicValue_RotationDirection_Clockwise = 0,

    /** Counter-clockwise. */
    kHAPCharacteristicValue_RotationDirection_CounterClockwise = 1
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_RotationDirection);
/**@}*/

/**
 * Rotation Speed.
 *
 * This characteristic describes the rotation speed of a fan.
 *
 * - Format: Float
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 0
 * - Maximum Value: 100
 * - Step Value: 1
 * - Unit: Percentage
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.81 Rotation Speed
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_RotationSpeed "rotation.speed"

extern const HAPUUID kHAPCharacteristicType_RotationSpeed;
/**@}*/

/**
 * Saturation.
 *
 * This characteristic describes color saturation.
 *
 * - Format: Float
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 0
 * - Maximum Value: 100
 * - Step Value: 1
 * - Unit: Percentage
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.82 Saturation
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_Saturation "saturation"

extern const HAPUUID kHAPCharacteristicType_Saturation;
/**@}*/

/**
 * Serial Number.
 *
 * This characteristic contains the manufacturer-specific serial number of the accessory, e.g. "1A2B3C4D5E6F". The
 * length must be greater than 1.
 *
 * - Format: String
 * - Permissions: Paired Read
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.87 Serial Number
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_SerialNumber "serial-number"

extern const HAPUUID kHAPCharacteristicType_SerialNumber;
/**@}*/

/**
 * Target Door State.
 *
 * This characteristic describes the target state of a door.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.118 Target Door State
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_TargetDoorState "door-state.target"

extern const HAPUUID kHAPCharacteristicType_TargetDoorState;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_TargetDoorState) { /** Open. */
                                                                  kHAPCharacteristicValue_TargetDoorState_Open = 0,

                                                                  /** Closed. */
                                                                  kHAPCharacteristicValue_TargetDoorState_Closed = 1
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_TargetDoorState);
/**@}*/

/**
 * Target Heating Cooling State.
 *
 * This characteristic describes the target mode of an accessory that supports heating/cooling, e.g. a thermostat.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 0
 * - Maximum Value: 3
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.119 Target Heating Cooling State
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_TargetHeatingCoolingState "heating-cooling.target"

extern const HAPUUID kHAPCharacteristicType_TargetHeatingCoolingState;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_TargetHeatingCoolingState) {
    /** Off. */
    kHAPCharacteristicValue_TargetHeatingCoolingState_Off = 0,

    /** Heat. If the current temperature is below the target temperature then turn on heating. */
    kHAPCharacteristicValue_TargetHeatingCoolingState_Heat = 1,

    /** Cool. If the current temperature is above the target temperature then turn on cooling. */
    kHAPCharacteristicValue_TargetHeatingCoolingState_Cool = 2,

    /**
     * Auto. Turn on heating or cooling to maintain temperature within the heating and cooling threshold
     * of the target temperature.
     */
    kHAPCharacteristicValue_TargetHeatingCoolingState_Auto = 3
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_TargetHeatingCoolingState);
/**@}*/

/**
 * Target Relative Humidity.
 *
 * This characteristic describes the target relative humidity that the accessory is actively attempting to reach.
 * The value is expressed as a percentage (%).
 *
 * - Format: Float
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 0
 * - Maximum Value: 100
 * - Step Value: 1
 * - Unit: Percentage
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.120 Target Relative Humidity
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_TargetRelativeHumidity "relative-humidity.target"

extern const HAPUUID kHAPCharacteristicType_TargetRelativeHumidity;
/**@}*/

/**
 * Target Temperature.
 *
 * This characteristic describes the target temperature in Celsius that the accessory is actively attempting to reach.
 * For example, a thermostat cooling a room to 75 degrees Fahrenheit would set the target temperature value to
 * 23.9 degrees Celsius.
 *
 * - Format: Float
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 10.0
 * - Maximum Value: 38.0
 * - Step Value: 0.1
 * - Unit: Celsius
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.121 Target Temperature
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_TargetTemperature "temperature.target"

extern const HAPUUID kHAPCharacteristicType_TargetTemperature;
/**@}*/

/**
 * Temperature Display Units.
 *
 * This characteristic describes units of temperature used for presentation purposes (e.g. the units of temperature
 * displayed on the screen).
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.122 Temperature Display Units
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_TemperatureDisplayUnits "temperature.units"

extern const HAPUUID kHAPCharacteristicType_TemperatureDisplayUnits;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_TemperatureDisplayUnits) {
    /** Celsius. */
    kHAPCharacteristicValue_TemperatureDisplayUnits_Celsius = 0,

    /** Fahrenheit. */
    kHAPCharacteristicValue_TemperatureDisplayUnits_Fahrenheit = 1
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_TemperatureDisplayUnits);
/**@}*/

/**
 * Version.
 *
 * This characteristic contains a version string.
 *
 * - Format: String
 * - Permissions: Paired Read
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.125 Version
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_Version "version"

extern const HAPUUID kHAPCharacteristicType_Version;
/**@}*/

/**
 * Pair Setup.
 *
 * Accessories must accept reads and writes to this characteristic to perform Pair Setup.
 *
 * - Format: TLV
 * - Permissions: Read, Write
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 5.13.1.1 Pair Setup
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_PairSetup "pairing.pair-setup"

extern const HAPUUID kHAPCharacteristicType_PairSetup;
/**@}*/

/**
 * Pair Verify.
 *
 * Accessories must accept reads and writes to this characteristic to perform Pair Verify.
 *
 * - Format: TLV
 * - Permissions: Read, Write
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 5.13.1.2 Pair Verify
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_PairVerify "pairing.pair-verify"

extern const HAPUUID kHAPCharacteristicType_PairVerify;
/**@}*/

/**
 * Pairing Features.
 *
 * Read-only characteristic that exposes pairing features must be supported by the accessory.
 *
 * - Format: UInt8
 * - Permissions: Read
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 5.13.1.3 Pairing Features
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_PairingFeatures "pairing.features"

extern const HAPUUID kHAPCharacteristicType_PairingFeatures;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_PairingFeatures) {
    /** Supports Apple Authentication Coprocessor. */
    kHAPCharacteristicValue_PairingFeatures_SupportsAppleAuthenticationCoprocessor = 1U << 0U,

    /** Supports Software Authentication. */
    kHAPCharacteristicValue_PairingFeatures_SupportsSoftwareAuthentication = 1U << 1U,
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_PairingFeatures);
/**@}*/

/**
 * Pairing Pairings.
 *
 * Accessories must accept reads and writes to this characteristic to add, remove, and list pairings.
 *
 * - Format: TLV
 * - Permissions: Paired Read, Paired Write
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 5.13.1.4 Pairing Pairings
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_PairingPairings "pairing.pairings"

extern const HAPUUID kHAPCharacteristicType_PairingPairings;
/**@}*/

/**
 * Firmware Revision.
 *
 * This characteristic describes a firmware revision string x[.y[.z]] (e.g. "100.1.1"):
 * - \<x\> is the major version number, required.
 * - \<y\> is the minor version number, required if it is non-zero or if \<z\> is present.
 * - \<z\> is the revision version number, required if non-zero.
 *
 * The firmware revision must follow the below rules:
 * - \<x\> is incremented when there is significant change. e.g., 1.0.0, 2.0.0, 3.0.0, etc.
 * - \<y\> is incremented when minor changes are introduced such as 1.1.0, 2.1.0, 3.1.0 etc.
 * - \<z\> is incremented when bug-fixes are introduced such as 1.0.1, 2.0.1, 3.0.1 etc.
 * - Subsequent firmware updates can have a lower \<y\> version only if \<x\> is incremented
 * - Subsequent firmware updates can have a lower \<z\> version only if \<x\> or \<y\> is incremented
 * - Each number (major, minor and revision version) must not be greater than (2^32 -1)
 *
 * The characteristic value must change after every firmware update.
 *
 * - Format: String
 * - Permissions: Paired Read
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.40 Firmware Revision
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_FirmwareRevision "firmware.revision"

extern const HAPUUID kHAPCharacteristicType_FirmwareRevision;
/**@}*/

/**
 * Hardware Revision.
 *
 * This characteristic describes a hardware revision string x[.y[.z]] (e.g. "100.1.1") and tracked when the board or
 * components of the same accessory is changed:
 * - \<x\> is the major version number, required.
 * - \<y\> is the minor version number, required if it is non-zero or if \<z\> is present.
 * - \<z\> is the revision version number, required if non-zero.
 *
 * The hardware revision must follow the below rules:
 * - \<x\> is incremented when there is significant change, e.g., 1.0.0, 2.0.0, 3.0.0, etc.
 * - \<y\> is incremented when minor changes are introduced such as 1.1.0, 2.1.0, 3.1.0 etc.
 * - \<z\> is incremented when bug-fixes are introduced such as 1.0.1, 2.0.1, 3.0.1 etc.
 * - Subsequent hardware updates can have a lower \<y\> version only if \<x\> is incremented
 * - Subsequent hardware updates can have a lower \<z\> version only if \<x\> or \<y\> is incremented
 *
 * The characteristic value must change after every hardware update.
 *
 * - Format: String
 * - Permissions: Paired Read
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.41 Hardware Revision
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_HardwareRevision "hardware.revision"

extern const HAPUUID kHAPCharacteristicType_HardwareRevision;
/**@}*/

/**
 * Air Particulate Density.
 *
 * This characteristic indicates the current air particulate matter density in micrograms/m^3.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: Float
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1000
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.7 Air Particulate Density
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_AirParticulateDensity "air-particulate.density"

extern const HAPUUID kHAPCharacteristicType_AirParticulateDensity;
/**@}*/

/**
 * Air Particulate Size.
 *
 * This characteristic indicates the size of air particulate matter in micrometers.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.8 Air Particulate Size
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_AirParticulateSize "air-particulate.size"

extern const HAPUUID kHAPCharacteristicType_AirParticulateSize;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_AirParticulateSize) { /** 2.5 Micrometers. */
                                                                     kHAPCharacteristicValue_AirParticulateSize_2_5 = 0,

                                                                     /** 10 Micrometers. */
                                                                     kHAPCharacteristicValue_AirParticulateSize_10 = 1
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_AirParticulateSize);
/**@}*/

/**
 * Security System Current State.
 *
 * This characteristic describes the state of a security system.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 4
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.84 Security System Current State
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_SecuritySystemCurrentState "security-system-state.current"

extern const HAPUUID kHAPCharacteristicType_SecuritySystemCurrentState;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_SecuritySystemCurrentState) {
    /** Stay Arm. The home is occupied and the residents are active. e.g. morning or evenings. */
    kHAPCharacteristicValue_SecuritySystemCurrentState_StayArm = 0,

    /** Away Arm. The home is unoccupied. */
    kHAPCharacteristicValue_SecuritySystemCurrentState_AwayArm = 1,

    /** Night Arm. The home is occupied and the residents are sleeping. */
    kHAPCharacteristicValue_SecuritySystemCurrentState_NightArm = 2,

    /** Disarmed. */
    kHAPCharacteristicValue_SecuritySystemCurrentState_Disarmed = 3,

    /** Alarm Triggered. */
    kHAPCharacteristicValue_SecuritySystemCurrentState_AlarmTriggered = 4
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_SecuritySystemCurrentState);
/**@}*/

/**
 * Security System Target State.
 *
 * This characteristic describes the target state of the security system.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 0
 * - Maximum Value: 3
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.85 Security System Target State
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_SecuritySystemTargetState "security-system-state.target"

extern const HAPUUID kHAPCharacteristicType_SecuritySystemTargetState;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_SecuritySystemTargetState) {
    /** Stay Arm. The home is occupied and the residents are active. e.g. morning or evenings. */
    kHAPCharacteristicValue_SecuritySystemTargetState_StayArm = 0,

    /** Away Arm. The home is unoccupied. */
    kHAPCharacteristicValue_SecuritySystemTargetState_AwayArm = 1,

    /** Night Arm. The home is occupied and the residents are sleeping. */
    kHAPCharacteristicValue_SecuritySystemTargetState_NightArm = 2,

    /** Disarm. */
    kHAPCharacteristicValue_SecuritySystemTargetState_Disarm = 3
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_SecuritySystemTargetState);
/**@}*/

/**
 * Battery Level.
 *
 * This characteristic describes the current level of the battery.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 100
 * - Step Value: 1
 * - Unit: Percentage
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.10 Battery Level
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_BatteryLevel "battery-level"

extern const HAPUUID kHAPCharacteristicType_BatteryLevel;
/**@}*/

/**
 * Carbon Monoxide Detected.
 *
 * This characteristic indicates if a sensor detects abnormal levels of Carbon Monoxide. Value should revert to 0 after
 * the Carbon Monoxide levels drop to normal levels.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.18 Carbon Monoxide Detected
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_CarbonMonoxideDetected "carbon-monoxide.detected"

extern const HAPUUID kHAPCharacteristicType_CarbonMonoxideDetected;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_CarbonMonoxideDetected) {
    /** Carbon Monoxide levels are normal. */
    kHAPCharacteristicValue_CarbonMonoxideDetected_Normal = 0,

    /** Carbon Monoxide levels are abnormal. */
    kHAPCharacteristicValue_CarbonMonoxideDetected_Abnormal = 1
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_CarbonMonoxideDetected);
/**@}*/

/**
 * Contact Sensor State.
 *
 * This characteristic describes the state of a door/window contact sensor. A value of 0 indicates that the contact is
 * detected. A value of 1 indicates that the contact is not detected.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.22 Contact Sensor State
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_ContactSensorState "contact-state"

extern const HAPUUID kHAPCharacteristicType_ContactSensorState;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_ContactSensorState) {
    /** Contact is detected. */
    kHAPCharacteristicValue_ContactSensorState_Detected = 0,

    /** Contact is not detected. */
    kHAPCharacteristicValue_ContactSensorState_NotDetected = 1
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_ContactSensorState);
/**@}*/

/**
 * Current Ambient Light Level.
 *
 * This characteristic indicates the current light level. The value is expressed in Lux units (lumens / m^2).
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: Float
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0.0001
 * - Maximum Value: 100000
 * - Unit: Lux
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.23 Current Ambient Light Level
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_CurrentAmbientLightLevel "light-level.current"

extern const HAPUUID kHAPCharacteristicType_CurrentAmbientLightLevel;
/**@}*/

/**
 * Current Horizontal Tilt Angle.
 *
 * This characteristic describes the current angle of horizontal slats for accessories such as windows, fans, portable
 * heater/coolers etc. This characteristic takes values between -90 and 90. A value of 0 indicates that the slats are
 * rotated to a fully open position. A value of -90 indicates that the slats are rotated all the way in a direction
 * where the user-facing edge is higher than the window-facing edge.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: Int
 * - Permissions: Paired Read, Notify
 * - Minimum Value: -90
 * - Maximum Value: 90
 * - Step Value: 1
 * - Unit: Arcdegrees
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.24 Current Horizontal Tilt Angle
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_CurrentHorizontalTiltAngle "horizontal-tilt.current"

extern const HAPUUID kHAPCharacteristicType_CurrentHorizontalTiltAngle;
/**@}*/

/**
 * Current Position.
 *
 * This characteristic describes the current position of accessories. This characteristic can be used with doors,
 * windows, awnings or window coverings. For windows and doors, a value of 0 indicates that a window (or door) is fully
 * closed while a value of 100 indicates a fully open position. For blinds/shades/awnings, a value of 0 indicates a
 * position that permits the least light and a value of 100 indicates a position that allows most light.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 100
 * - Step Value: 1
 * - Unit: Percentage
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.27 Current Position
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_CurrentPosition "position.current"

extern const HAPUUID kHAPCharacteristicType_CurrentPosition;
/**@}*/

/**
 * Current Vertical Tilt Angle.
 *
 * This characteristic describes the current angle of vertical slats for accessories such as windows, fans, portable
 * heater/coolers etc. This characteristic takes values between -90 and 90. A value of 0 indicates that the slats are
 * rotated to be fully open. A value of -90 indicates that the slats are rotated all the way in a direction where the
 * user-facing edge is to the left of the window-facing edge.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: Int
 * - Permissions: Paired Read, Notify
 * - Minimum Value: -90
 * - Maximum Value: 90
 * - Step Value: 1
 * - Unit: Arcdegrees
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.28 Current Vertical Tilt Angle
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_CurrentVerticalTiltAngle "vertical-tilt.current"

extern const HAPUUID kHAPCharacteristicType_CurrentVerticalTiltAngle;
/**@}*/

/**
 * Hold Position.
 *
 * This characteristic causes the service such as door or window covering to stop at its current position. A value of 1
 * must hold the state of the accessory. For e.g, the window must stop moving when this characteristic is written a
 * value of 1. A value of 0 should be ignored.
 *
 * A write to `Target Position` characteristic will release the hold.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: Bool
 * - Permissions: Paired Write
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.43 Hold Position
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_HoldPosition "position.hold"

extern const HAPUUID kHAPCharacteristicType_HoldPosition;
/**@}*/

/**
 * Leak Detected.
 *
 * This characteristic indicates if a sensor detected a leak (e.g. water leak, gas leak). A value of 1 indicates that a
 * leak is detected. Value should return to 0 when leak stops.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.50 Leak Detected
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_LeakDetected "leak-detected"

extern const HAPUUID kHAPCharacteristicType_LeakDetected;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_LeakDetected) { /** Leak is not detected. */
                                                               kHAPCharacteristicValue_LeakDetected_NotDetected = 0,

                                                               /** Leak is detected. */
                                                               kHAPCharacteristicValue_LeakDetected_Detected = 1
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_LeakDetected);
/**@}*/

/**
 * Occupancy Detected.
 *
 * This characteristic indicates if occupancy was detected (e.g. a person present). A value of 1 indicates occupancy is
 * detected. Value should return to 0 when occupancy is not detected.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.67 Occupancy Detected
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_OccupancyDetected "occupancy-detected"

extern const HAPUUID kHAPCharacteristicType_OccupancyDetected;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_OccupancyDetected) {
    /** Occupancy is not detected. */
    kHAPCharacteristicValue_OccupancyDetected_NotDetected = 0,

    /** Occupancy is detected. */
    kHAPCharacteristicValue_OccupancyDetected_Detected = 1
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_OccupancyDetected);
/**@}*/

/**
 * Position State.
 *
 * This characteristic describes the state of the position of accessories. This characteristic can be used with doors,
 * windows, awnings or window coverings for presentation purposes.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 2
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.73 Position State
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_PositionState "position.state"

extern const HAPUUID kHAPCharacteristicType_PositionState;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_PositionState) {
    /** Going to the minimum value specified in metadata. */
    kHAPCharacteristicValue_PositionState_GoingToMinimum = 0,

    /** Going to the maximum value specified in metadata. */
    kHAPCharacteristicValue_PositionState_GoingToMaximum = 1,

    /** Stopped. */
    kHAPCharacteristicValue_PositionState_Stopped = 2
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_PositionState);
/**@}*/

/**
 * Programmable Switch Event.
 *
 * This characteristic describes an event generated by a programmable switch. Reading this characteristic must return
 * the last event triggered for BLE. For IP accessories, the accessory must set the value of Paired Read to
 * null(i.e. "value" : null) in the attribute database. A read of this characteristic must always return a null value
 * for IP accessories. The value must only be reported in the events ("ev") property.
 *
 * This characteristic requires iOS 10.3 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 2
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.75 Programmable Switch Event
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_ProgrammableSwitchEvent "input-event"

extern const HAPUUID kHAPCharacteristicType_ProgrammableSwitchEvent;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_ProgrammableSwitchEvent) {
    /** Single Press. */
    kHAPCharacteristicValue_ProgrammableSwitchEvent_SinglePress = 0,

    /** Double Press. */
    kHAPCharacteristicValue_ProgrammableSwitchEvent_DoublePress = 1,

    /** Long Press. */
    kHAPCharacteristicValue_ProgrammableSwitchEvent_LongPress = 2
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_ProgrammableSwitchEvent);
/**@}*/

/**
 * Status Active.
 *
 * This characteristic describes an accessory's current working status. A value of true indicates that the accessory is
 * active and is functioning without any errors.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: Bool
 * - Permissions: Paired Read, Notify
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.96 Status Active
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_StatusActive "status-active"

extern const HAPUUID kHAPCharacteristicType_StatusActive;
/**@}*/

/**
 * Smoke Detected.
 *
 * This characteristic indicates if a sensor detects abnormal levels of smoke. A value of 1 indicates that smoke levels
 * are abnormal. Value should return to 0 when smoke levels are normal.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.95 Smoke Detected
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_SmokeDetected "smoke-detected"

extern const HAPUUID kHAPCharacteristicType_SmokeDetected;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_SmokeDetected) { /** Smoke is not detected. */
                                                                kHAPCharacteristicValue_SmokeDetected_NotDetected = 0,

                                                                /** Smoke is detected. */
                                                                kHAPCharacteristicValue_SmokeDetected_Detected = 1
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_SmokeDetected);
/**@}*/

/**
 * Status Fault.
 *
 * This characteristic describes an accessory which has a fault. A non-zero value indicates that the accessory has
 * experienced a fault that may be interfering with its intended functionality. A value of 0 indicates that there is no
 * fault.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.97 Status Fault
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_StatusFault "status-fault"

extern const HAPUUID kHAPCharacteristicType_StatusFault;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_StatusFault) { /** No Fault. */
                                                              kHAPCharacteristicValue_StatusFault_None = 0,

                                                              /** General Fault. */
                                                              kHAPCharacteristicValue_StatusFault_General = 1
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_StatusFault);
/**@}*/

/**
 * Status Jammed.
 *
 * This characteristic describes an accessory which is in a jammed state. A status of 1 indicates that an accessory's
 * mechanisms are jammed prevents it from functionality normally. Value should return to 0 when conditions that jam the
 * accessory are rectified.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.98 Status Jammed
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_StatusJammed "status-jammed"

extern const HAPUUID kHAPCharacteristicType_StatusJammed;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_StatusJammed) { /** Not Jammed. */
                                                               kHAPCharacteristicValue_StatusJammed_NotJammed = 0,

                                                               /** Jammed. */
                                                               kHAPCharacteristicValue_StatusJammed_Jammed = 1
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_StatusJammed);
/**@}*/

/**
 * Status Low Battery.
 *
 * This characteristic describes an accessory's battery status. A status of 1 indicates that the battery level of the
 * accessory is low. Value should return to 0 when the battery charges to a level thats above the low threshold.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.99 Status Low Battery
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_StatusLowBattery "status-lo-batt"

extern const HAPUUID kHAPCharacteristicType_StatusLowBattery;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_StatusLowBattery) { /** Battery level is normal. */
                                                                   kHAPCharacteristicValue_StatusLowBattery_Normal = 0,

                                                                   /** Battery level is low. */
                                                                   kHAPCharacteristicValue_StatusLowBattery_Low = 1
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_StatusLowBattery);
/**@}*/

/**
 * Status Tampered.
 *
 * This characteristic describes an accessory which has been tampered with. A status of 1 indicates that the accessory
 * has been tampered with. Value should return to 0 when the accessory has been reset to a non-tampered state.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.100 Status Tampered
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_StatusTampered "status-tampered"

extern const HAPUUID kHAPCharacteristicType_StatusTampered;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_StatusTampered) { /** Accessory is not tampered. */
                                                                 kHAPCharacteristicValue_StatusTampered_NotTampered = 0,

                                                                 /** Accessory is tampered with. */
                                                                 kHAPCharacteristicValue_StatusTampered_Tampered = 1
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_StatusTampered);
/**@}*/

/**
 * Target Horizontal Tilt Angle.
 *
 * This characteristic describes the target angle of horizontal slats for accessories such as windows, fans, portable
 * heater/coolers etc.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: Int
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: -90
 * - Maximum Value: 90
 * - Step Value: 1
 * - Unit: Arcdegrees
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.115 Target Horizontal Tilt Angle
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_TargetHorizontalTiltAngle "horizontal-tilt.target"

extern const HAPUUID kHAPCharacteristicType_TargetHorizontalTiltAngle;
/**@}*/

/**
 * Target Position.
 *
 * This characteristic describes the target position of accessories. This characteristic can be used with doors,
 * windows, awnings or window coverings. For windows and doors, a value of 0 indicates that a window (or door) is fully
 * closed while a value of 100 indicates a fully open position. For blinds/shades/awnings, a value of 0 indicates a
 * position that permits the least light and a value of 100 indicates a position that allows most light.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 0
 * - Maximum Value: 100
 * - Step Value: 1
 * - Unit: Percentage
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.117 Target Position
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_TargetPosition "position.target"

extern const HAPUUID kHAPCharacteristicType_TargetPosition;
/**@}*/

/**
 * Target Vertical Tilt Angle.
 *
 * This characteristic describes the target angle of vertical slats for accessories such as windows, fans, portable
 * heater/coolers etc.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: Int
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: -90
 * - Maximum Value: 90
 * - Step Value: 1
 * - Unit: Arcdegrees
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.123 Target Vertical Tilt Angle
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_TargetVerticalTiltAngle "vertical-tilt.target"

extern const HAPUUID kHAPCharacteristicType_TargetVerticalTiltAngle;
/**@}*/

/**
 * Security System Alarm Type.
 *
 * This characteristic describes the type of alarm triggered by a security system. A value of 1 indicates an 'unknown'
 * cause. Value should revert to 0 when the alarm conditions are cleared.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.83 Security System Alarm Type
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_SecuritySystemAlarmType "security-system.alarm-type"

extern const HAPUUID kHAPCharacteristicType_SecuritySystemAlarmType;
/**@}*/

/**
 * Charging State.
 *
 * This characteristic describes the charging state of a battery or an accessory.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 2
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.19 Charging State
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_ChargingState "charging-state"

extern const HAPUUID kHAPCharacteristicType_ChargingState;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_ChargingState) { /** Not Charging. */
                                                                kHAPCharacteristicValue_ChargingState_NotCharging = 0,

                                                                /** Charging. */
                                                                kHAPCharacteristicValue_ChargingState_Charging = 1,

                                                                /** Not Chargeable. */
                                                                kHAPCharacteristicValue_ChargingState_NotChargeable = 2
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_ChargingState);
/**@}*/

/**
 * Carbon Monoxide Level.
 *
 * This characteristic indicates the Carbon Monoxide levels detected in parts per million (ppm).
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: Float
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 100
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.13 Carbon Monoxide Level
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_CarbonMonoxideLevel "carbon-monoxide.level"

extern const HAPUUID kHAPCharacteristicType_CarbonMonoxideLevel;
/**@}*/

/**
 * Carbon Monoxide Peak Level.
 *
 * This characteristic indicates the highest detected level (ppm) of Carbon Monoxide detected by a sensor.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: Float
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 100
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.14 Carbon Monoxide Peak Level
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_CarbonMonoxidePeakLevel "carbon-monoxide.peak-level"

extern const HAPUUID kHAPCharacteristicType_CarbonMonoxidePeakLevel;
/**@}*/

/**
 * Carbon Dioxide Detected.
 *
 * This characteristic indicates if a sensor detects abnormal levels of Carbon Dioxide. Value should revert to 0 after
 * the Carbon Dioxide levels drop to normal levels.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.15 Carbon Dioxide Detected
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_CarbonDioxideDetected "carbon-dioxide.detected"

extern const HAPUUID kHAPCharacteristicType_CarbonDioxideDetected;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_CarbonDioxideDetected) {
    /** Carbon Dioxide levels are normal. */
    kHAPCharacteristicValue_CarbonDioxideDetected_Normal = 0,

    /** Carbon Dioxide levels are abnormal. */
    kHAPCharacteristicValue_CarbonDioxideDetected_Abnormal = 1
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_CarbonDioxideDetected);
/**@}*/

/**
 * Carbon Dioxide Level.
 *
 * This characteristic indicates the detected level of Carbon Dioxide in parts per million (ppm).
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: Float
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 100000
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.16 Carbon Dioxide Level
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_CarbonDioxideLevel "carbon-dioxide.level"

extern const HAPUUID kHAPCharacteristicType_CarbonDioxideLevel;
/**@}*/

/**
 * Carbon Dioxide Peak Level.
 *
 * This characteristic indicates the highest detected level (ppm) of carbon dioxide detected by a sensor.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: Float
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 100000
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.17 Carbon Dioxide Peak Level
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_CarbonDioxidePeakLevel "carbon-dioxide.peak-level"

extern const HAPUUID kHAPCharacteristicType_CarbonDioxidePeakLevel;
/**@}*/

/**
 * Air Quality.
 *
 * This characteristic describes the subject assessment of air quality by an accessory.
 *
 * This characteristic requires iOS 9 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 5
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.9 Air Quality
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_AirQuality "air-quality"

extern const HAPUUID kHAPCharacteristicType_AirQuality;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_AirQuality) { /** Unknown. */
                                                             kHAPCharacteristicValue_AirQuality_Unknown = 0,

                                                             /** Excellent. */
                                                             kHAPCharacteristicValue_AirQuality_Excellent = 1,

                                                             /** Good. */
                                                             kHAPCharacteristicValue_AirQuality_Good = 2,

                                                             /** Fair. */
                                                             kHAPCharacteristicValue_AirQuality_Fair = 3,

                                                             /** Inferior. */
                                                             kHAPCharacteristicValue_AirQuality_Inferior = 4,

                                                             /** Poor. */
                                                             kHAPCharacteristicValue_AirQuality_Poor = 5
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_AirQuality);
/**@}*/

/**
 * Service Signature.
 *
 * - Format: Data
 * - Permissions: Paired Read
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.4.4.5.4 Service Signature Characteristic
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_ServiceSignature "service-signature"

extern const HAPUUID kHAPCharacteristicType_ServiceSignature;
/**@}*/

/**
 * Accessory Flags.
 *
 * When set indicates accessory requires additional setup. Use of Accessory Flags requires written approval by Apple in
 * advance.
 *
 * - Format: UInt32
 * - Permissions: Paired Read, Notify
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.2 Accessory Flags
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_AccessoryFlags "accessory-properties"

extern const HAPUUID kHAPCharacteristicType_AccessoryFlags;

HAP_OPTIONS_BEGIN(uint8_t, HAPCharacteristicValue_AccessoryFlags) {
    /** Requires additional setup. */
    kHAPCharacteristicValue_AccessoryFlags_RequiresAdditionalSetup = 1U << 0U
} HAP_OPTIONS_END(uint8_t, HAPCharacteristicValue_AccessoryFlags);
/**@}*/

/**@}*/

/**
 * Lock Physical Controls.
 *
 * This characteristic describes a way to lock a set of physical controls on an accessory (eg. child lock).
 *
 * This characteristic requires iOS 10.3 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.55 Lock Physical Controls
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_LockPhysicalControls "lock-physical-controls"

extern const HAPUUID kHAPCharacteristicType_LockPhysicalControls;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_LockPhysicalControls) {
    /** Control lock disabled. */
    kHAPCharacteristicValue_LockPhysicalControls_Disabled = 0,

    /** Control lock enabled. */
    kHAPCharacteristicValue_LockPhysicalControls_Enabled = 1
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_LockPhysicalControls);
/**@}*/

/**
 * Target Air Purifier State.
 *
 * This characteristic describes the target state of the air purifier.
 *
 * This characteristic requires iOS 10.3 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.108 Target Air Purifier State
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_TargetAirPurifierState "air-purifier.state.target"

extern const HAPUUID kHAPCharacteristicType_TargetAirPurifierState;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_TargetAirPurifierState) {
    /** Manual. */
    kHAPCharacteristicValue_TargetAirPurifierState_Manual = 0,

    /** Auto. */
    kHAPCharacteristicValue_TargetAirPurifierState_Auto = 1
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_TargetAirPurifierState);
/**@}*/

/**
 * Current Air Purifier State.
 *
 * This characteristic describes the current state of the air purifier.
 *
 * This characteristic requires iOS 10.3 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 2
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.25 Current Air Purifier State
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_CurrentAirPurifierState "air-purifier.state.current"

extern const HAPUUID kHAPCharacteristicType_CurrentAirPurifierState;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_CurrentAirPurifierState) {
    /** Inactive. */
    kHAPCharacteristicValue_CurrentAirPurifierState_Inactive = 0,

    /** Idle. */
    kHAPCharacteristicValue_CurrentAirPurifierState_Idle = 1,

    /** Purifying Air. */
    kHAPCharacteristicValue_CurrentAirPurifierState_PurifyingAir = 2
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_CurrentAirPurifierState);
/**@}*/

/**
 * Current Slat State.
 *
 * This characteristic describes the current state of the slats.
 *
 * This characteristic requires iOS 10.3 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 2
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.26 Current Slat State
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_CurrentSlatState "slat.state.current"

extern const HAPUUID kHAPCharacteristicType_CurrentSlatState;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_CurrentSlatState) { /** Fixed. */
                                                                   kHAPCharacteristicValue_CurrentSlatState_Fixed = 0,

                                                                   /** Jammed. */
                                                                   kHAPCharacteristicValue_CurrentSlatState_Jammed = 1,

                                                                   /** Swinging. */
                                                                   kHAPCharacteristicValue_CurrentSlatState_Swinging = 2
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_CurrentSlatState);
/**@}*/

/**
 * Filter Life Level.
 *
 * This characteristic describes the current filter life level.
 *
 * This characteristic requires iOS 10.3 or later.
 *
 * - Format: Float
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 100
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.38 Filter Life Level
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_FilterLifeLevel "filter.life-level"

extern const HAPUUID kHAPCharacteristicType_FilterLifeLevel;
/**@}*/

/**
 * Filter Change Indication.
 *
 * This characteristic describes if a filter needs to be changed.
 *
 * This characteristic requires iOS 10.3 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.39 Filter Change Indication
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_FilterChangeIndication "filter.change-indication"

extern const HAPUUID kHAPCharacteristicType_FilterChangeIndication;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_FilterChangeIndication) {
    /** Filter does not need to be changed. */
    kHAPCharacteristicValue_FilterChangeIndication_Ok = 0,

    /** Filter needs to be changed. */
    kHAPCharacteristicValue_FilterChangeIndication_Change = 1
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_FilterChangeIndication);
/**@}*/

/**
 * Reset Filter Indication.
 *
 * This characteristic allows a user to reset the filter indication. When the value of 1 is written to this
 * characteristic by the user, the accessory should reset it to 0 once the relevant action to reset the filter
 * indication is executed. If the accessory supports Filter Change Indication, the value of that characteristic should
 * also reset back to 0.
 *
 * This characteristic requires iOS 10.3 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Write
 * - Minimum Value: 1
 * - Maximum Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.79 Reset Filter Indication
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_ResetFilterIndication "filter.reset-indication"

extern const HAPUUID kHAPCharacteristicType_ResetFilterIndication;
/**@}*/

/**
 * Current Fan State.
 *
 * This characteristic describes the current state of the fan.
 *
 * This characteristic requires iOS 10.3 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 2
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.31 Current Fan State
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_CurrentFanState "fan.state.current"

extern const HAPUUID kHAPCharacteristicType_CurrentFanState;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_CurrentFanState) { /** Inactive. */
                                                                  kHAPCharacteristicValue_CurrentFanState_Inactive = 0,

                                                                  /** Idle. */
                                                                  kHAPCharacteristicValue_CurrentFanState_Idle = 1,

                                                                  /** Blowing Air. */
                                                                  kHAPCharacteristicValue_CurrentFanState_BlowingAir = 2
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_CurrentFanState);
/**@}*/

/**
 * Active.
 *
 * This characteristic indicates whether the service is currently active.
 *
 * This characteristic requires iOS 10.3 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.3 Active
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_Active "active"

extern const HAPUUID kHAPCharacteristicType_Active;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_Active) { /** Inactive. */
                                                         kHAPCharacteristicValue_Active_Inactive = 0,

                                                         /** Active. */
                                                         kHAPCharacteristicValue_Active_Active = 1
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_Active);
/**@}*/

/**
 * Current Heater Cooler State.
 *
 * This characteristic describes the current state of a heater cooler.
 *
 * This characteristic requires iOS 11 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 3
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.33 Current Heater Cooler State
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_CurrentHeaterCoolerState "heater-cooler.state.current"

extern const HAPUUID kHAPCharacteristicType_CurrentHeaterCoolerState;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_CurrentHeaterCoolerState) {
    /** Inactive. */
    kHAPCharacteristicValue_CurrentHeaterCoolerState_Inactive = 0,

    /** Idle. */
    kHAPCharacteristicValue_CurrentHeaterCoolerState_Idle = 1,

    /** Heating. */
    kHAPCharacteristicValue_CurrentHeaterCoolerState_Heating = 2,

    /** Cooling. */
    kHAPCharacteristicValue_CurrentHeaterCoolerState_Cooling = 3
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_CurrentHeaterCoolerState);
/**@}*/

/**
 * Target Heater Cooler State.
 *
 * This characteristic describes the target state of heater cooler.
 *
 * "Heat or Cool" state must only be included for accessories which include both a cooler and a heater.
 * "Heat or Cool" state (see `Target Heater Cooler State`) implies that the accessory will always try to maintain the
 * `Current Temperature` between `Heating Threshold Temperature` and `Cooling Threshold Temperature` and if the
 * `Current Temperature` increases above/falls below the threshold temperatures the equipment will start
 * heating or cooling respectively.
 *
 * In "Heat" state the accessory will start heating if the current temperature is below the
 * `Heating Threshold Temperature`. In "Cool" state the accessory will start cooling if the current temperature
 * is greater than the `Cooling Threshold Temperature`.
 *
 * This characteristic requires iOS 11 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 0
 * - Maximum Value: 2
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.111 Target Heater Cooler State
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_TargetHeaterCoolerState "heater-cooler.state.target"

extern const HAPUUID kHAPCharacteristicType_TargetHeaterCoolerState;

/**
 * @see HomeKit Accessory Protocol Specification R13
 *      Section 10.109 Target Heater Cooler State
 */
HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_TargetHeaterCoolerState) {
    /** Heat or Cool. */
    kHAPCharacteristicValue_TargetHeaterCoolerState_HeatOrCool = 0,

    /** Heat. */
    kHAPCharacteristicValue_TargetHeaterCoolerState_Heat = 1,

    /** Cool. */
    kHAPCharacteristicValue_TargetHeaterCoolerState_Cool = 2
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_TargetHeaterCoolerState);
/**@}*/

/**
 * Current Humidifier Dehumidifier State.
 *
 * This characteristic describes the current state of a humidifier or/and a dehumidifier.
 *
 * This characteristic requires iOS 11 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 3
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.29 Current Humidifier Dehumidifier State
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_CurrentHumidifierDehumidifierState "humidifier-dehumidifier.state.current"

extern const HAPUUID kHAPCharacteristicType_CurrentHumidifierDehumidifierState;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_CurrentHumidifierDehumidifierState) {
    /** Inactive. */
    kHAPCharacteristicValue_CurrentHumidifierDehumidifierState_Inactive = 0,

    /** Idle. */
    kHAPCharacteristicValue_CurrentHumidifierDehumidifierState_Idle = 1,

    /** Humidifying. */
    kHAPCharacteristicValue_CurrentHumidifierDehumidifierState_Humidifying = 2,

    /** Dehumidifying. */
    kHAPCharacteristicValue_CurrentHumidifierDehumidifierState_Dehumidifying = 3
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_CurrentHumidifierDehumidifierState);
/**@}*/

/**
 * Target Humidifier Dehumidifier State.
 *
 * This characteristic describes the target state of a humidifier or/and a dehumidifier.
 *
 * "Humidifier or Dehumidifier" state must only be included for accessories which include both a humidifier and a
 * dehumidifier. "Humidifier or Dehumidifier" state implies that the accessory will always try to maintain the
 * `Current Relative Humidity` between `Relative Humidity Humidifier Threshold` and
 * `Relative Humidity Dehumidifier Threshold` and if the `Current Relative Humidity` increases above/falls below the
 * threshold relative humidity the equipment will start dehumidifying or humidifying respectively.
 *
 * In "Humidifier" state the accessory will start humidifying if the current humidity is below the
 * `Relative Humidity Humidifier Threshold`. In "Dehumidifier" mode the accessory will start dehumidifying if the
 * current humidity is greater than the `Relative Humidity Dehumidifier Threshold`.
 *
 * This characteristic requires iOS 11 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 0
 * - Maximum Value: 2
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.116 Target Humidifier Dehumidifier State
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_TargetHumidifierDehumidifierState "humidifier-dehumidifier.state.target"

extern const HAPUUID kHAPCharacteristicType_TargetHumidifierDehumidifierState;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_TargetHumidifierDehumidifierState) {
    /** Humidifier or Dehumidifier. */
    kHAPCharacteristicValue_TargetHumidifierDehumidifierState_HumidifierOrDehumidifier = 0,

    /** Humidifier. */
    kHAPCharacteristicValue_TargetHumidifierDehumidifierState_Humidifier = 1,

    /** Dehumidifier. */
    kHAPCharacteristicValue_TargetHumidifierDehumidifierState_Dehumidifier = 2
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_TargetHumidifierDehumidifierState);
/**@}*/

/**
 * Water Level.
 *
 * This characteristic describes the current water level.
 *
 * This characteristic requires iOS 11 or later.
 *
 * - Format: Float
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 100
 * - Step Value: 1
 * - Unit: Percentage
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.128 Water Level
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_WaterLevel "water-level"

extern const HAPUUID kHAPCharacteristicType_WaterLevel;
/**@}*/

/**
 * Swing Mode.
 *
 * This characteristic describes if swing mode is enabled.
 *
 * This characteristic requires iOS 10.3 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.107 Swing Mode
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_SwingMode "swing-mode"

extern const HAPUUID kHAPCharacteristicType_SwingMode;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_SwingMode) { /** Swing disabled. */
                                                            kHAPCharacteristicValue_SwingMode_Disabled = 0,

                                                            /** Swing enabled. */
                                                            kHAPCharacteristicValue_SwingMode_Enabled = 1
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_SwingMode);
/**@}*/

/**
 * Target Fan State.
 *
 * This characteristic describes the target state of the fan.
 *
 * This characteristic requires iOS 10.3 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.109 Target Fan State
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_TargetFanState "fan.state.target"

extern const HAPUUID kHAPCharacteristicType_TargetFanState;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_TargetFanState) { /** Manual. */
                                                                 kHAPCharacteristicValue_TargetFanState_Manual = 0,

                                                                 /** Auto. */
                                                                 kHAPCharacteristicValue_TargetFanState_Auto = 1
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_TargetFanState);
/**@}*/

/**
 * Slat Type.
 *
 * This characteristic describes the type of the slats.
 * If the slats can tilt on a horizontal axis, the value of this characteristic must be set to "Horizontal".
 * If the slats can tilt on a vertical axis, the value of this characteristic must be set to "Vertical".
 *
 * This characteristic requires iOS 10.3 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.94 Slat Type
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_SlatType "type.slat"

extern const HAPUUID kHAPCharacteristicType_SlatType;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_SlatType) { /** Horizontal. */
                                                           kHAPCharacteristicValue_SlatType_Horizontal = 0,

                                                           /** Vertical. */
                                                           kHAPCharacteristicValue_SlatType_Vertical = 1
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_SlatType);
/**@}*/

/**
 * Current Tilt Angle.
 *
 * This characteristic describes the current angle of slats for accessories such as windows, fans, portable
 * heater/coolers etc. This characteristic takes values between -90 and 90. A value of 0 indicates that the slats are
 * rotated to be fully open. At value 0 the user-facing edge and the window-facing edge are perpendicular to the window.
 *
 * For "Horizontal" slat (see `Slat Type`):
 * A value of -90 indicates that the slats are rotated all the way in a direction where the user-facing edge is to the
 * left of the window-facing edge.
 *
 * For "Vertical" slat (see `Slat Type`):
 * A value of -90 indicates that the slats are rotated all the way in a direction where the user-facing edge is higher
 * than the window-facing edge.
 *
 * This characteristic requires iOS 10.3 or later.
 *
 * - Format: Int
 * - Permissions: Paired Read, Notify
 * - Minimum Value: -90
 * - Maximum Value: 90
 * - Step Value: 1
 * - Unit: Arcdegrees
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.36 Current Tilt Angle
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_CurrentTiltAngle "tilt.current"

extern const HAPUUID kHAPCharacteristicType_CurrentTiltAngle;
/**@}*/

/**
 * Target Tilt Angle.
 *
 * This characteristic describes the target angle of slats for accessories such as windows, fans, portable
 * heater/coolers etc.
 *
 * This characteristic requires iOS 10.3 or later.
 *
 * - Format: Int
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: -90
 * - Maximum Value: 90
 * - Step Value: 1
 * - Unit: Arcdegrees
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.110 Target Tilt Angle
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_TargetTiltAngle "tilt.target"

extern const HAPUUID kHAPCharacteristicType_TargetTiltAngle;
/**@}*/

/**
 * Ozone Density.
 *
 * This characteristic indicates the current ozone density in micrograms/m^3.
 *
 * This characteristic requires iOS 10.3 or later.
 *
 * - Format: Float
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1000
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.71 Ozone Density
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_OzoneDensity "density.ozone"

extern const HAPUUID kHAPCharacteristicType_OzoneDensity;
/**@}*/

/**
 * Nitrogen Dioxide Density.
 *
 * This characteristic indicates the current NO2 density in micrograms/m^3.
 *
 * This characteristic requires iOS 10.3 or later.
 *
 * - Format: Float
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1000
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.64 Nitrogen Dioxide Density
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_NitrogenDioxideDensity "density.no2"

extern const HAPUUID kHAPCharacteristicType_NitrogenDioxideDensity;
/**@}*/

/**
 * Sulphur Dioxide Density.
 *
 * This characteristic indicates the current SO2 density in micrograms/m^3.
 *
 * This characteristic requires iOS 10.3 or later.
 *
 * - Format: Float
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1000
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.106 Sulphur Dioxide Density
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_SulphurDioxideDensity "density.so2"

extern const HAPUUID kHAPCharacteristicType_SulphurDioxideDensity;
/**@}*/

/**
 * PM2.5 Density.
 *
 * This characteristic indicates the current PM2.5 micrometer particulate density in micrograms/m^3.
 *
 * This characteristic requires iOS 10.3 or later.
 *
 * - Format: Float
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1000
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.66 PM2.5 Density
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_PM2_5Density "density.pm2_5"

extern const HAPUUID kHAPCharacteristicType_PM2_5Density;
/**@}*/

/**
 * PM10 Density.
 *
 * This characteristic indicates the current PM10 micrometer particulate density in micrograms/m^3.
 *
 * This characteristic requires iOS 10.3 or later.
 *
 * - Format: Float
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1000
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.72 PM10 Density
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_PM10Density "density.pm10"

extern const HAPUUID kHAPCharacteristicType_PM10Density;
/**@}*/

/**
 * VOC Density.
 *
 * This characteristic indicates the current volatile organic compound density in micrograms/m^3.
 *
 * This characteristic requires iOS 10.3 or later.
 *
 * - Format: Float
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1000
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.126 VOC Density
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_VOCDensity "density.voc"

extern const HAPUUID kHAPCharacteristicType_VOCDensity;
/**@}*/

/**
 * Relative Humidity Dehumidifier Threshold.
 *
 * This characteristic describes the relative humidity dehumidifier threshold. The value of this characteristic
 * represents the 'maximum relative humidity' that must be reached before dehumidifier is turned on.
 *
 * This characteristic requires iOS 11 or later.
 *
 * - Format: Float
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 0
 * - Maximum Value: 100
 * - Step Value: 1
 * - Unit: Percentage
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.76 Relative Humidity Dehumidifier Threshold
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_RelativeHumidityDehumidifierThreshold \
    "relative-humidity.dehumidifier-threshold"

extern const HAPUUID kHAPCharacteristicType_RelativeHumidityDehumidifierThreshold;
/**@}*/

/**
 * Relative Humidity Humidifier Threshold.
 *
 * This characteristic describes the relative humidity humidifier threshold. The value of this characteristic represents
 * the 'minimum relative humidity' that must be reached before humidifier is turned on.
 *
 * This characteristic requires iOS 11 or later.
 *
 * - Format: Float
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 0
 * - Maximum Value: 100
 * - Step Value: 1
 * - Unit: Percentage
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.77 Relative Humidity Humidifier Threshold
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_RelativeHumidityHumidifierThreshold "relative-humidity.humidifier-threshold"

extern const HAPUUID kHAPCharacteristicType_RelativeHumidityHumidifierThreshold;
/**@}*/

/**
 * Service Label Index.
 *
 * This characteristic should be used identify the index of the label that maps to `Service Label Namespace` used by the
 * accessory.
 *
 * This characteristic requires iOS 10.3 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read
 * - Minimum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.88 Service Label Index
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_ServiceLabelIndex "service-label-index"

extern const HAPUUID kHAPCharacteristicType_ServiceLabelIndex;
/**@}*/

/**
 * Service Label Namespace.
 *
 * This characteristic describes the naming schema for an accessory. For example, this characteristic can be used to
 * describe the type of labels used to identify individual services of an accessory.
 *
 * This characteristic requires iOS 10.3 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.89 Service Label Namespace
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_ServiceLabelNamespace "service-label-namespace"

extern const HAPUUID kHAPCharacteristicType_ServiceLabelNamespace;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_ServiceLabelNamespace) {
    /** Dots. For example, "." ".." "..." "....". */
    kHAPCharacteristicValue_ServiceLabelNamespace_Dots = 0,

    /** Arabic numerals. For e.g. 0,1,2,3. */
    kHAPCharacteristicValue_ServiceLabelNamespace_ArabicNumerals = 1
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_ServiceLabelNamespace);
/**@}*/

/**
 * Color Temperature.
 *
 * This characteristic describes color temperature which is represented in reciprocal megaKelvin (MK^-1) or mirek scale.
 * (M = 1,000,000 / K where M is the desired mirek value and K is temperature in Kelvin).
 *
 * If this characteristic is included in the `Light Bulb`, `Hue` and `Saturation` must not be included as optional
 * characteristics in `Light Bulb`. This characteristic must not be used for lamps which support color.
 *
 * This characteristic requires iOS 10.3 or later.
 *
 * - Format: UInt32
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 50
 * - Maximum Value: 400
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.21 Color Temperature
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_ColorTemperature "color-temperature"

extern const HAPUUID kHAPCharacteristicType_ColorTemperature;
/**@}*/

/**
 * Program Mode.
 *
 * This characteristic describes if there are programs scheduled on the accessory. If there are Programs scheduled on
 * the accessory and the accessory is used for manual operation, the value of this characteristic must be
 * "Program Scheduled, currently overridden to manual mode".
 *
 * This characteristic requires iOS 11.2 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 2
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.74 Program Mode
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_ProgramMode "program-mode"

extern const HAPUUID kHAPCharacteristicType_ProgramMode;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_ProgramMode) {
    /** No Programs Scheduled. */
    kHAPCharacteristicValue_ProgramMode_NotScheduled = 0,

    /** Program Scheduled. */
    kHAPCharacteristicValue_ProgramMode_Scheduled = 1,

    /** Program Scheduled, currently overridden to manual mode. */
    kHAPCharacteristicValue_ProgramMode_ScheduleOverriddenToManual = 2
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_ProgramMode);
/**@}*/

/**
 * In Use.
 *
 * This characteristic describes if the service is in use. The service must be "Active" before the value of this
 * characteristic can be set to in use.
 *
 * This characteristic requires iOS 11.2 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.48 In Use
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_InUse "in-use"

extern const HAPUUID kHAPCharacteristicType_InUse;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_InUse) { /** Not in use. */
                                                        kHAPCharacteristicValue_InUse_NotInUse = 0,

                                                        /** In use. */
                                                        kHAPCharacteristicValue_InUse_InUse = 1
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_InUse);
/**@}*/

/**
 * Set Duration.
 *
 * This characteristic describes the set duration. For a `Valve` this duration defines how long a valve should be set to
 * "In Use". Once the valve is "In Use", any changes to this characteristic take affect in the next operation when the
 * `Valve` is "Active".
 *
 * This duration is defined in seconds.
 *
 * This characteristic requires iOS 11.2 or later.
 *
 * - Format: UInt32
 * - Permissions: Paired Read, Paired Write, Notify
 * - Minimum Value: 0
 * - Maximum Value: 3600
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.112 Set Duration
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_SetDuration "set-duration"

extern const HAPUUID kHAPCharacteristicType_SetDuration;
/**@}*/

/**
 * Remaining Duration.
 *
 * This characteristic describes the remaining duration on the accessory. Notifications on this characteristic must only
 * be used if the remaining duration increases/decreases from the accessory's usual countdown of remaining duration and
 * when the duration reaches 0. e.g. It must not send notifications when the remaining duration is ticking down from
 * 100,99,98... if 100 was the initial Set duration. However, if the remaining duration changes to 95 from 92 (increase)
 * or 85 from 92 (decrease which is not part of the usual duration countdown), it must send a notification.
 *
 * This duration is defined in seconds.
 *
 * This characteristic requires iOS 11.2 or later.
 *
 * - Format: UInt32
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 3600
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.78 Remaining Duration
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_RemainingDuration "remaining-duration"

extern const HAPUUID kHAPCharacteristicType_RemainingDuration;
/**@}*/

/**
 * Valve Type.
 *
 * This characteristic describes the type of valve.
 *
 * This characteristic requires iOS 11.2 or later.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 3
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.124 Valve Type
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_ValveType "valve-type"

extern const HAPUUID kHAPCharacteristicType_ValveType;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_ValveType) { /** Generic valve. */
                                                            kHAPCharacteristicValue_ValveType_GenericValve = 0,

                                                            /** Irrigation. */
                                                            kHAPCharacteristicValue_ValveType_Irrigation = 1,

                                                            /** Shower head. */
                                                            kHAPCharacteristicValue_ValveType_ShowerHead = 2,

                                                            /** Water faucet. */
                                                            kHAPCharacteristicValue_ValveType_WaterFaucet = 3
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_ValveType);
/**@}*/

/**
 * Is Configured.
 *
 * This characteristic describes if the service is configured for use. For example, all of the valves in an irrigation
 * system may not be configured depending on physical wire connection.
 *
 * If the accessory supports updating through HAP, then it must also advertise Paired Write in the permissions.
 *
 * This characteristic requires iOS 12.x.
 *
 * - Format: UInt8
 * - Permissions: Paired Read, Notify
 * - Minimum Value: 0
 * - Maximum Value: 1
 * - Step Value: 1
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.49 Is Configured
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_IsConfigured "is-configured"

extern const HAPUUID kHAPCharacteristicType_IsConfigured;

HAP_ENUM_BEGIN(uint8_t, HAPCharacteristicValue_IsConfigured) { /** Not Configured. */
                                                               kHAPCharacteristicValue_IsConfigured_NotConfigured = 0,

                                                               /** Configured. */
                                                               kHAPCharacteristicValue_IsConfigured_Configured = 1
} HAP_ENUM_END(uint8_t, HAPCharacteristicValue_IsConfigured);
/**@}*/

/**
 * Active Identifier.
 *
 * This HAP characteristic allows the accessory to indicate the target that is currently selected in the UI of the
 * accessory (e.g. remote accessory which can control multiple Apple TVs).
 *
 * This characteristic requires iOS 12 or later.
 *
 * - Format: UInt32
 * - Permissions: Paired Read, Notify
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 9.4 Active Identifier
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_ActiveIdentifier "active-identifier"

extern const HAPUUID kHAPCharacteristicType_ActiveIdentifier;
/**@}*/

/**
 * ADK Version.
 *
 * This characteristic describes a ADK version string x[.y[.z]];b (e.g. "100.1.1;1A1").
 *
 * - \<x\> is the major version number, required.
 * - \<y\> is the minor version number, required if it is non-zero or if \<z\> is present.
 * - \<z\> is the revision version number, required if non-zero.
 * - \<b\> is the build version string, required.
 *
 * The characteristic value must change after every firmware update that uses a new ADK version.
 *
 * - Format: String
 * - Permissions: Paired Read
 */
/**@{*/
#define kHAPCharacteristicDebugDescription_ADKVersion "adk-version"

extern const HAPUUID kHAPCharacteristicType_ADKVersion;
/**@}*/

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
