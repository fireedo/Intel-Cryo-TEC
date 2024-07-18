#pragma once

#include <string>
#include <map>
#include <nlohmann/json.hpp>

namespace intel_cryo_tec {

// Use a shorter alias for the json library
using json = nlohmann::json;

// OpCode enum used in both standalone and GUI versions
enum class OpCode {
    heartbeat = 0,
    getTecTemperature = 1,
    getHumidity = 2,
    getDewPoint = 3,
    getSetPointOffset = 4,
    getPCoefficient = 5,
    getICoefficient = 6,
    getDCoefficient = 7,
    getTecPowerLevel = 8,
    getHwVersion = 9,
    getFwVersion = 10,
    setSetPointOffset = 20,
    setPCoefficient = 21,
    setICoefficient = 22,
    setDCoefficient = 23,
    setLowPowerMode = 24,
    setCPUTemp = 25,
    setNtcCoefficient = 26,
    getNtcCoefficient = 27,
    setTempSensorMode = 28,
    setTecPowerLevel = 29,
    resetBoard = 30,
    getBoardTemp = 31,
    getVoltageAndCurrent = 34,
    getTecVoltage = 35,
    getTecCurrent = 36
};

// Heartbeat status map
const std::map<int, std::string> heartbeat_status = {
    {0, "Board initialisation completed"},
    {1, "Power supply OK"},
    {2, "TEC thermistor reading in range"},
    {3, "Humidity sensor reading in range"},
    {4, "Last received command OK"},
    {5, "Last received command had a bad CRC"},
    {6, "Last received command is incomplete"},
    {7, "Failsafe has been activated"},
    {8, "PID constants were loaded and accepted"},
    {9, "PID constants were rejected"},
    {10, "Set point for the PID is out of range"},
    {11, "Default set point was loaded"},
    {12, "PID is running"},
    {13, "Overcurrent protection has been triggered"},
    {14, "Board temperature is in range"},
    {15, "TEC connection OK"},
    {16, "Low power mode enabled"},
    {17, "Using NTC thermistor"}
};

// Common utility functions
inline float unpack_float(uint32_t x) {
    float f;
    std::memcpy(&f, &x, sizeof(f));
    return f;
}

inline uint32_t unpack_int(float x) {
    uint32_t i;
    std::memcpy(&i, &x, sizeof(i));
    return i;
}

// Constants
const std::string STATUS_FILE_PATH = "/var/run/intel_cryo_tec/status.json";
const std::string SERIAL_PORT = "/dev/ttyACM0";
const int BAUD_RATE = 115200;

// Structure to hold TEC status
struct TECStatus {
    std::vector<std::string> heartbeat;
    float voltage;
    float current;
    float dewpoint;
    float temperature;
    float power_level;
    float humidity;

    json to_json() const {
        return json{
            {"heartbeat", heartbeat},
            {"voltage", voltage},
            {"current", current},
            {"dewpoint", dewpoint},
            {"temperature", temperature},
            {"power_level", power_level},
            {"humidity", humidity}
        };
    }

    static TECStatus from_json(const json& j) {
        TECStatus status;
        status.heartbeat = j["heartbeat"].get<std::vector<std::string>>();
        status.voltage = j["voltage"].get<float>();
        status.current = j["current"].get<float>();
        status.dewpoint = j["dewpoint"].get<float>();
        status.temperature = j["temperature"].get<float>();
        status.power_level = j["power_level"].get<float>();
        status.humidity = j["humidity"].get<float>();
        return status;
    }
};

} // namespace intel_cryo_tec
