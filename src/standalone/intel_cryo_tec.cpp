#include "common/intel_cryo_tec_common.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <filesystem>
#include <boost/asio.hpp>
#include <boost/crc.hpp>

namespace fs = std::filesystem;
using namespace intel_cryo_tec;

class SerialPort {
public:
    SerialPort(const std::string& port, unsigned int baud_rate)
    : io(), serial(io, port) {
        serial.set_option(boost::asio::serial_port_base::baud_rate(baud_rate));
    }

    void send_data(OpCode opcode, uint32_t operand) {
        std::vector<uint8_t> payload = {0xAA, static_cast<uint8_t>(opcode)};
        payload.push_back((operand >> 24) & 0xFF);
        payload.push_back((operand >> 16) & 0xFF);
        payload.push_back((operand >> 8) & 0xFF);
        payload.push_back(operand & 0xFF);

        uint16_t crc = crc16(payload);
        payload.push_back(crc & 0xFF);
        payload.push_back((crc >> 8) & 0xFF);

        boost::asio::write(serial, boost::asio::buffer(payload));
    }

    std::pair<OpCode, uint32_t> read_data() {
        std::vector<uint8_t> data(8);
        boost::asio::read(serial, boost::asio::buffer(data));

        OpCode opcode = static_cast<OpCode>(data[1] - 127);
        uint32_t operand = (data[2] << 24) | (data[3] << 16) | (data[4] << 8) | data[5];

        return {opcode, operand};
    }

private:
    boost::asio::io_service io;
    boost::asio::serial_port serial;

    uint16_t crc16(const std::vector<uint8_t>& data) {
        boost::crc_16_type result;
        result.process_bytes(data.data(), data.size());
        return result.checksum();
    }
};

class IntelCryoTEC {
public:
    IntelCryoTEC() : serial(SERIAL_PORT, BAUD_RATE) {}

    void monitor_loop() {
        fs::create_directories(fs::path(STATUS_FILE_PATH).parent_path());

        while (true) {
            TECStatus status;
            status.heartbeat = submit_command<std::vector<std::string>>(OpCode::heartbeat, 0);
            auto voltage_current = submit_command<std::pair<float, float>>(OpCode::getVoltageAndCurrent, 0);
            status.voltage = voltage_current.first;
            status.current = voltage_current.second;
            status.dewpoint = submit_command<float>(OpCode::getDewPoint, 0);
            status.temperature = submit_command<float>(OpCode::getTecTemperature, 0);
            status.power_level = submit_command<float>(OpCode::getTecPowerLevel, 0);
            status.humidity = submit_command<float>(OpCode::getHumidity, 0);

            json status_json = status.to_json();
            status_json["timestamp"] = std::chrono::system_clock::now().time_since_epoch().count();

            std::ofstream status_file(STATUS_FILE_PATH);
            status_file << status_json.dump(4);

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void set_cryo_mode() {
        submit_command<void>(OpCode::setSetPointOffset, unpack_int(2.0f));
        submit_command<void>(OpCode::setPCoefficient, unpack_int(100.0f));
        submit_command<void>(OpCode::setICoefficient, unpack_int(1.0f));
        submit_command<void>(OpCode::setDCoefficient, 0);
        submit_command<void>(OpCode::setLowPowerMode, 0);
    }

    void reset_board() {
        submit_command<void>(OpCode::resetBoard, 0);
    }

private:
    SerialPort serial;

    std::pair<float, float> getVoltageAndCurrent() {
        auto [opcode, data] = serial.read_data();
        float voltage = static_cast<float>(data & 0xFF) / 21.25f;
        float current = static_cast<float>((data >> 8) & 0xFF) / 4.6545f;
        return {voltage, current};
    }

    template<typename T>
    T submit_command(OpCode opcode, uint32_t data) {
        serial.send_data(opcode, data);
        if constexpr (std::is_same_v<T, void>) {
            serial.read_data(); // Discard the result
            return;
        } else if constexpr (std::is_same_v<T, std::pair<float, float>>) {
            return getVoltageAndCurrent();
        } else {
            auto [received_opcode, received_data] = serial.read_data();
            return handle_result<T>(received_opcode, received_data);
        }
    }

    template<typename T>
    T handle_result(OpCode opcode, uint32_t data) {
        if constexpr (std::is_same_v<T, float>) {
            switch (opcode) {
                case OpCode::getTecVoltage:
                    return static_cast<float>(data) / 21.25f;
                case OpCode::getTecCurrent:
                    return static_cast<float>(data) / 4.6545f;
                case OpCode::getTecTemperature:
                case OpCode::getDewPoint:
                case OpCode::getHumidity:
                    return unpack_float(data);
                default:
                    return static_cast<float>(data);
            }
        } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
            std::vector<std::string> result;
            for (int i = 0; i < 18; ++i) {
                if (data & (1 << i)) {
                    result.push_back(heartbeat_status.at(i));
                }
            }
            return result;
        } else {
            return static_cast<T>(data);
        }
    }
};

int main() {
    try {
        IntelCryoTEC tec;
        std::cout << "Resetting board..." << std::endl;
        tec.reset_board();
        std::cout << "Waiting for 5 seconds..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::cout << "Setting cryo mode..." << std::endl;
        tec.set_cryo_mode();
        std::cout << "Starting monitor loop..." << std::endl;
        tec.monitor_loop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
