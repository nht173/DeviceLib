#ifndef GENERIC_OUTPUT_BASE_H
#define GENERIC_OUTPUT_BASE_H

#include <Arduino.h>
#include "GPIO_helper.h"


#define USE_LAST_STATE

/* ======== Last state ======== */
#if defined(USE_LAST_STATE)

#include "ENVFile.h"
extern ENVFile GO_FS;

#endif // USE_LAST_STATE


/* ======== I2C GPIO Expander ======== */
#if __has_include(<PCF8574.h>)
#include <PCF8574.h>
#ifndef USE_PCF
#define USE_PCF
#define PCF_TYPE PCF8574
#endif // USE_PCF
#endif // __has_include_next(<PCF8574.h>)


/* ======== ESPNOW ======== */
#if __has_include(<espnow-node.h>)
#ifndef USE_ESPNOW_NODE
#define USE_ESPNOW_NODE
#include <espnow-node.h>
#endif // USE_ESPNOW_NODE
#endif // __has_include_next(<espnow-node.h>)

/* ======== Firebase RTDB ======== */
#if __has_include(<Firebase_ESP_Client.h>)
#include <vector>
#include <Firebase_ESP_Client.h>

#ifndef USE_FBRTDB
#define USE_FBRTDB
#define FBRTDB_LIB_TYPE 1
// 1 = Firebase_ESP_Client (mobizt/Firebase Arduino Client Library for ESP8266 and ESP32)

namespace stdGenericOutput {
    struct fbrtdb_config_t {
        FirebaseData *fbdo;
        String path;
    };
};

#endif // USE_FBRTDB

#ifndef USE_TIMESTAMP
#define USE_TIMESTAMP
#endif // USE_TIMESTAMP

#endif // __has_include_next(<Firebase_ESP_Client.h>)

namespace stdGenericOutput {

    typedef enum {
        START_UP_NONE = 0xFF,
        START_UP_OFF = 0x00,
        START_UP_ON = 0x01,
        START_UP_LAST_STATE = 0x02,
    } startup_state_t;

    class GenericOutputBase;
}



#if defined(USE_FBRTDB) && FBRTDB_LIB_TYPE == 1
extern std::vector<stdGenericOutput::GenericOutputBase *> attachedDBDevices;
#endif



class stdGenericOutput::GenericOutputBase {

public:

    GenericOutputBase() = default;

    /**
     * @brief Construct a new GenericOutputBase object
     * 
     * @param pin pin number
     * @param activeState LOW or HIGH. Default is LOW
     */
    explicit GenericOutputBase(uint8_t pin, bool activeState = LOW, startup_state_t startUpState = START_UP_NONE);

#if defined(USE_PCF)
    /**
     * @brief Construct a new GenericOutputBase object
     *
     * @param pcf PCF object
     * @param pin pin number
     * @param activeState LOW or HIGH. Default is LOW
     */
    explicit GenericOutputBase(PCF_TYPE& pcf, uint8_t pin, bool activeState = LOW, startup_state_t startUpState = START_UP_NONE);
#endif

    /**
     * @brief Destroy the GenericOutputBase object
     */
    ~GenericOutputBase();

    /**
     * @brief call this function to set the startup state otherwise it will be set on first loop
     */
    virtual void begin();

    /**
     * @brief Set powe to ON
     * 
     */
    virtual void on(bool force);

    /**
     * @brief Set power to ON
     *
     */
    virtual void on() {
        on(false);
    }

    /**
     * @brief Set power to OFF
     * 
     */
    virtual void off(bool force);

    /**
     * @brief Set power to OFF
     *
     */
    virtual void off() {
        off(false);
    }

    /**
     * @brief Toggle power
     * 
     */
    void toggle();

    /**
     * @brief Set the power state
     * @param state
     */
    void setState(bool state, bool force = false);

    /**
     * @brief Set the power state from string "ON" or "OFF"
     * 
     * @param state 
     */
    void setState(const String &state, bool force = false);

    /**
     * @brief Set the Active State object
     * 
     * @param activeState 
     */
    void setActiveState(bool activeState);

    /**
     * @brief Get the Active State object
     * 
     * @return true 
     * @return false 
     */
    bool getActiveState() const;

    /**
     * @brief Get current state of the device
     * 
     * @return true when ON
     * @return false when OFF
     */
    bool getState() const;

    /**
     * @brief Get current state of the device as string "ON" or "OFF"
     *
     * @return String
     
     */
    virtual String getStateString() const;

    /**
     * @brief Set callback function to be called when power is on
     *
     * @param onPowerOn
     */
    void onPowerOn(std::function<void()> onPowerOn);

    /**
     * @brief Set callback function to be called when power is off
     *
     * @param onPowerOff
     */
    void onPowerOff(std::function<void()> onPowerOff);

    /**
     * @brief Set callback function to be called when power is changed
     *
     * @param onPowerChanged
     */
    void onPowerChanged(std::function<void()> onPowerChanged);


#if defined(USE_FBRTDB)

    /**
     * @brief Attach Firebase RTDB to the device
     * @param dbconfig
     * @param subPath
     */
    void attachDatabase(fbrtdb_config_t *dbconfig, String subPath);

    /**
     * @brief Detach Firebase RTDB from the device
     */
    void detachDatabase();

    /**
     * @brief Set state to device without updating the database
     * @param state
     */
    void syncState(bool state);

    /**
     * @brief Sync the state from the database to the device.
     *
     * Call this function in the stream callback. It will be parsed and set the state to the device.
     * @param data FirebaseStream
     */
    void syncState(FirebaseStream *data, bool onlyProcessWithPutMethod = true);

#endif // USE_FBRTDB


#ifdef USE_ESPNOW_NODE
    String getStateBoolString() const;

    void attachESPNOW(const String& propName, ENNodeInfo *nodeInfo);
#endif

protected:
    uint8_t _pin = UINT8_MAX;
    bool _activeState;
    startup_state_t _startUpState = START_UP_NONE;
    bool _state;
    std::function<void()> _onPowerOn = nullptr;
    std::function<void()> _onPowerOff = nullptr;
    std::function<void()> _onPowerChanged = nullptr;
#ifdef USE_LAST_STATE
    String _pinKey = "";
    bool _flag_set_startup_state = false;
#endif // USE_LAST_STATE

    /**
     * @brief Schedule run callback function
     * @param callback
     */
    static void _execCallback(const std::function<void()> &callback);

#if defined(USE_PCF)
    PCF_TYPE* _pcf = nullptr;
#endif


#if defined(USE_FBRTDB)
    fbrtdb_config_t* _fbRTDBconfig = nullptr;
    String _dbSubPath = "";
    bool _flag_ignore_update_db = false;

    void _updateDB();
#endif


#ifdef USE_ESPNOW_NODE
    String _propName;
    ENNodeInfo* _nodeInfo = nullptr;
    // ENNode* _node = nullptr;
#endif

    /**
     * @brief digitalWrite wrapper
     */
    void _write();
};

#endif // GENERIC_OUTPUT_BASE_H