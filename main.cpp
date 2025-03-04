// MIT License
//
// Copyright (c) 2025 Pavel Konovalov
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <iostream>
#include <csignal>
#include <cs104_connection.h>
#include <atomic>
#include <hal_thread.h>

std::atomic commandConfirmed(false);
std::atomic startDTConfirmed(false);

void signalHandler(const int signum) {
    std::cout << "Interrupt signal (" << signum << ") received. Exiting.." << std::endl;
    exit(signum);
}

void connectionHandler(void *parameter, CS104_Connection connection, CS104_ConnectionEvent event) {
    switch (event) {
        case CS104_CONNECTION_OPENED:
            std::cout << "Connection established" << std::endl;
            break;
        case CS104_CONNECTION_CLOSED:
            std::cout << "Connection closed" << std::endl;
            break;
        case CS104_CONNECTION_STARTDT_CON_RECEIVED:
            startDTConfirmed = true;
            std::cout << "StartDT confirmation received" << std::endl;
            break;
        case CS104_CONNECTION_STOPDT_CON_RECEIVED:
            std::cout << "StopDT confirmation received" << std::endl;
            break;
        default:
            std::cout << "Unsupported connection event: " << event << std::endl;
    }
}

bool asduReceivedHandler(void *parameter, int address, CS101_ASDU asdu) {
    const int numberOfASDUElements = CS101_ASDU_getNumberOfElements(asdu);

    switch (CS101_ASDU_getCOT(asdu)) {
        case CS101_COT_ACTIVATION_CON:
            commandConfirmed = true;
            std::cout << "Control command confirmed by RTU" << std::endl;
            break;
        case CS101_COT_UNKNOWN_IOA:
            commandConfirmed = true;
            for (auto i = 0; i < numberOfASDUElements; i++) {
                const auto io = CS101_ASDU_getElement(asdu, i);
                std::cerr << "Control command error: unknown IOA " << std::to_string(InformationObject_getObjectAddress(io)) << std::endl;
                InformationObject_destroy(io);
            }
            break;
        default: ;
    }
    return true;
}

bool waitForCondition(const std::atomic<bool> &condition, const int timeoutMs) {
    const auto start = std::chrono::steady_clock::now();
    while (!condition) {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() > timeoutMs) {
            return false;
        }
        Thread_sleep(100);
    }
    return true;
}

int main(const int argc, char *argv[]) {
    if (argc != 6) {
        std::cerr << "Usage: " << argv[0] << " <RTU_IP> <RTU_Port> <IOA> <ASDU_Type [45|46]> <Value [0|1]>" << std::endl;
        return 1;
    }

    const std::string rtuAddress = argv[1];
    const int rtuPort = std::stoi(argv[2]);
    const int ioa = std::stoi(argv[3]);
    const int asdu = std::stoi(argv[4]);
    const int value = std::stoi(argv[5]);

    signal(SIGINT, signalHandler);

    CS104_Connection connection = CS104_Connection_create(rtuAddress.c_str(), rtuPort);

    if (!connection) {
        std::cerr << "Failed to create connection" << std::endl;
        return 1;
    }

    CS104_Connection_setConnectTimeout(connection, 5000);

    CS104_Connection_setConnectionHandler(connection, connectionHandler, nullptr);
    CS104_Connection_setASDUReceivedHandler(connection, asduReceivedHandler, nullptr);


    if (!CS104_Connection_connect(connection)) {
        std::cerr << "Failed to connect to RTU at " << rtuAddress << ":" << rtuPort << std::endl;
        Thread_sleep(1000);
        CS104_Connection_destroy(connection);
        return 1;
    }

    std::cout << "Connected to RTU at " << rtuAddress << ":" << rtuPort << std::endl;

    CS104_Connection_sendStartDT(connection);

    if (!waitForCondition(startDTConfirmed, 5000)) {
        std::cerr << "Timeout waiting for START_DT confirmation" << std::endl;
        CS104_Connection_destroy(connection);
        return 1;
    }

    switch (asdu) {
        case C_SC_NA_1: {
            const auto io = reinterpret_cast<InformationObject>(SingleCommand_create(nullptr, ioa, (value != 0), false, 0));

            if (!CS104_Connection_sendProcessCommandEx(connection, CS101_COT_ACTIVATION, ioa, io)) {
                std::cerr << "Failed to send single command" << std::endl;
            }
            InformationObject_destroy(io);
        }
        break;

        case C_DC_NA_1: {
            const auto io = reinterpret_cast<InformationObject>(DoubleCommand_create(nullptr, ioa, (value == 1) ? 2 : 1, false, 0));
            if (!CS104_Connection_sendProcessCommandEx(connection, CS101_COT_ACTIVATION, ioa, io)) {
                std::cerr << "Failed to send double command" << std::endl;
            }
            InformationObject_destroy(io);
        }
        break;

        default:
            std::cerr << "Invalid ASDU " + std::to_string(asdu) + " Use 45 for single command, 46 for double command" << std::endl;
    }

    if (!waitForCondition(commandConfirmed, 5000)) {
        std::cerr << "Timeout waiting for command confirmation" << std::endl;
        CS104_Connection_destroy(connection);
        return 1;
    }

    CS104_Connection_destroy(connection);

    return 0;
}
