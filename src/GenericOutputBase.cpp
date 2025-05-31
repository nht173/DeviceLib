#include "GenericOutputBase.h"

#if defined(USE_LAST_STATE)
ENVFile GO_FS("/gpiols");
#endif // USE_LAST_STATE


#if defined(USE_FBRTDB) && FBRTDB_LIB_TYPE == 1
std::vector<stdGenericOutput::GenericOutputBase *> attachedDBDevices;
#endif // USE_FBRTDB && FBRTDB_LIB_TYPE == 1



/* =================== Contructor =====================*/

stdGenericOutput::GenericOutputBase::GenericOutputBase(uint8_t pin, bool activeState, startup_state_t startUpState) {
    _pin = pin;
    _startUpState = startUpState;
    _activeState = activeState;
    _state = false;
    _pinKey = "p" + String(_pin);
    pinMode(_pin, OUTPUT);
    // Set last state
    devlib_callback_t lastStateCB([this](){ begin(); });
    _execCallback(lastStateCB);
}

#if defined(USE_PCF)
stdGenericOutput::GenericOutputBase::GenericOutputBase(PCF_TYPE& pcf, uint8_t pin, bool activeState, startup_state_t startUpState) {
    _pin = pin;
    _activeState = activeState;
    _startUpState = startUpState;
    _state = false;
    // @WARNING: getAddress is not a member of PCF8574
    _pinKey = "p" + String(pcf.getAddress()) + String(_pin);
    _pcf = &pcf;
    pcf.pinMode(_pin, OUTPUT);
    // Set last state
    devlib_callback_t lastStateCB([this](){ begin(); });
    _execCallback(lastStateCB);
}
#endif

stdGenericOutput::GenericOutputBase::~GenericOutputBase() {
    #if defined(USE_FBRTDB) && FBRTDB_LIB_TYPE == 1
    // Remove from attachedDBDevices
    auto it = std::find(attachedDBDevices.begin(), attachedDBDevices.end(), this);
    if (it != attachedDBDevices.end()) {
        attachedDBDevices.erase(it);
    }
#endif // USE_FBRTDB && FBRTDB_LIB_TYPE == 1

    _onPowerOn.fn = nullptr;
    _onPowerOff.fn = nullptr;
    _onPowerChanged.fn = nullptr;
}



/* =================== Functional =====================*/

void stdGenericOutput::GenericOutputBase::begin() {
    if (_flag_set_startup_state) return;
    _flag_set_startup_state = true;
    switch (_startUpState) {
        case START_UP_NONE:
            return;
        case START_UP_OFF:
            off(true);
            break;
        case START_UP_ON:
            on(true);
            break;
        case START_UP_LAST_STATE:
#if defined(USE_LAST_STATE)
            setState(GO_FS.getBool(_pinKey, false), true);
#endif
            break;
        default:
            break;
    };
}

void stdGenericOutput::GenericOutputBase::_execCallback(devlib_callback_t &callback) {
    if (!callback.isValid()) return;
    if (callback.schedule) {
#if defined(ESP32)
        GPIO_Scheduler.addSchedule(callback.fn);
#else
        schedule_function(callback.fn);
#endif
    } else {
        callback();
    }
}

void stdGenericOutput::GenericOutputBase::_write() {
    /* GPIO set */
    if (_pin != UINT8_MAX) {
#if defined(USE_PCF)
        GO_PRINTF("[%s] write: %d\n", _pinKey.c_str(), _state ? _activeState : !_activeState);
        if (_pcf != nullptr) {
            _pcf->digitalWrite(_pin, _state ? _activeState : !_activeState);
        } else {
            digitalWrite(_pin, _state ? _activeState : !_activeState);
        }
#else
        GO_PRINTF("[%s] write: %d\n", _pinKey.c_str(), _state ? _activeState : !_activeState);
        digitalWrite(_pin, _state ? _activeState : !_activeState);
#endif
    }
    /* Store last state */
#if defined(USE_LAST_STATE)
    GO_FS.set(_pinKey, _state);
#endif
    /* Update state to cloud */
#if defined(USE_FBRTDB)
#if defined(ESP32)
    GPIO_Scheduler.addSchedule([this]() { _updateDB(); });
#else
    schedule_function([this]() {
        const_cast<GenericOutputBase*>(this)->_updateDB();
    });
#endif
#endif // USE_FBRTDB
}

void stdGenericOutput::GenericOutputBase::on(bool force) {
    if (_state && !force) return;
    GO_PRINTF("[%s] ON\n", _pinKey.c_str());
    _state = true;
    _write();
    _execCallback(_onPowerOn);
    _execCallback(_onPowerChanged);
}

void stdGenericOutput::GenericOutputBase::off(bool force) {
    if (!_state && !force) return;
    GO_PRINTF("[%s] OFF\n", _pinKey.c_str());
    _state = false;
    _write();
    _execCallback(_onPowerOff);
    _execCallback(_onPowerChanged);
}

void stdGenericOutput::GenericOutputBase::toggle() {
    GO_PRINTF("[%s] TOGGLE - Current: %s\n", _pinKey.c_str(), _state ? "ON" : "OFF");
    _state ? off(false) : on(false);
}

void stdGenericOutput::GenericOutputBase::setState(bool state, bool force) {
    state ? on(force) : off(force);
}

void stdGenericOutput::GenericOutputBase::setState(const String &state, bool force) {
    String st = state;
    st.trim();
    st.toUpperCase();
    if (st == "ON" || st == "1" || st == "TRUE") {
        on(force);
    } else if (st == "OFF" || st == "0" || st == "FALSE") {
        off(force);
    }
}



#if defined(USE_FBRTDB)

void stdGenericOutput::GenericOutputBase::_updateDB() {
    if (_flag_ignore_update_db) {
        _flag_ignore_update_db = false;
        return;
    }
    if (_fbRTDBconfig == nullptr) {
        Serial.printf("[Err][Update] %s: DB config not found\n", _dbSubPath.c_str());
        return;
    }
    if (_fbRTDBconfig->path.length() == 0) {
        Serial.printf("[Err][Update] %s: DB path not found\n", _dbSubPath.c_str());
        return;
    }
    Serial.printf("[Update] %s: %s\n", _dbSubPath.c_str(), getStateString().c_str());
    FirebaseJson json;
    json.set(_dbSubPath, _state);
    Firebase.RTDB.updateNodeSilent(_fbRTDBconfig->fbdo, _fbRTDBconfig->path, &json);
}

void stdGenericOutput::GenericOutputBase::attachDatabase(fbrtdb_config_t *dbconfig, String subPath) {
    _fbRTDBconfig = dbconfig;
    if (subPath.startsWith("/"))
        subPath = subPath.substring(1);
    _dbSubPath = subPath;
#if defined(USE_FBRTDB) && FBRTDB_LIB_TYPE == 1
    if (std::find(attachedDBDevices.begin(), attachedDBDevices.end(), this) == attachedDBDevices.end()) {
        attachedDBDevices.push_back(this);
    }
#endif
}

void stdGenericOutput::GenericOutputBase::detachDatabase() {
    _fbRTDBconfig = nullptr;
    _dbSubPath = "";
}

void stdGenericOutput::GenericOutputBase::syncState(bool state) {
    Serial.printf("[Sync] %s: %s\n", _dbSubPath.c_str(), state ? "ON": "OFF");
    _flag_ignore_update_db = true;
    setState(state);
    _flag_ignore_update_db = false;
}

void stdGenericOutput::GenericOutputBase::syncState(FirebaseStream *data, bool onlyProcessWithPutMethod) {
    if (_dbSubPath.length() == 0) return;
    if (onlyProcessWithPutMethod && data->eventType() != "put") return;
    bool newState;
    if (data->dataPath() == "/") {
        FirebaseJsonData json;
        data->jsonObjectPtr()->get(json, _dbSubPath);
        if (!json.success) return;
        newState = json.boolValue;
    } else if (data->dataPath() == ("/" + _dbSubPath) && data->dataTypeEnum() == d_boolean) {
        newState = data->boolData();
    } else {
        return;
    }
    syncState(newState);
}

#endif // USE_FBRTDB



#ifdef USE_ESPNOW_NODE
String stdGenericOutput::GenericOutputBase::getStateBoolString() const {
    return _state ? "true" : "false";
}

void stdGenericOutput::GenericOutputBase::attachESPNOW(const String& propName, ENNodeInfo *nodeInfo) {
    _propName = propName;
    _nodeInfo = nodeInfo;
    nodeInfo->addProp(propName, getStateBoolString(),
        [this](){
            return getStateBoolString();
        },
        [this](const String& value){
            if (value == "true") {
                on();
            } else if (value == "false") {
                off();
            } else if (value == "toggle") {
                toggle();
                schedule_function([this](){
                    Node.sendSyncProp(_propName, getStateBoolString());
                });
            } else {
                return false;
            }
            return true;
        });
}
#endif // USE_ESPNOW_NODE





/* =================== Getter/Setter =====================*/

void stdGenericOutput::GenericOutputBase::setActiveState(bool activeState) {
    _activeState = activeState;
}

bool stdGenericOutput::GenericOutputBase::getActiveState() const {
    return _activeState;
}

bool stdGenericOutput::GenericOutputBase::getState() const {
    return _state;
}

String stdGenericOutput::GenericOutputBase::getStateString() const {
    return _state ? "ON" : "OFF";
}




/* =================== Callback =====================*/

void stdGenericOutput::GenericOutputBase::onPowerOn(std::function<void()> onPowerOn, bool schedule) {
    _onPowerOn.assign(onPowerOn, schedule);
}

void stdGenericOutput::GenericOutputBase::onPowerOff(std::function<void()> onPowerOff, bool schedule) {
    _onPowerOff.assign(onPowerOff, schedule);
}

void stdGenericOutput::GenericOutputBase::onPowerChanged(std::function<void()> onPowerChanged, bool schedule) {
    _onPowerChanged.assign(onPowerChanged, schedule);
}
