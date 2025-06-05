// main.hpp

#include "logger.hpp"
#include <cstdlib>
#include <ctime>
#include <string>

const char* ct[16] = {"r", "g", "y", "b", "p", "a", "l", "s",
                      "m", "o", "t", "n", "f", "z", "w", "k"};

const char* colorString(const std::string& str) {
    static std::string out;
    out.clear();
    out.reserve(str.length() * 10);

    for (char c : str) {
        out += ct[rand() % 16];
        out += '|';
        out += c;
        out += '|';
    }
    return out.c_str();
}

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    LOG_INFO("Logger color tag test: p|red| g|green| t|yellow| b|blue|.");

    const char* msg = colorString("Hello, world! testing colorfull text!");
    LOG_INFO("String test: %s", msg);

    for (int i = 0; i < 10; ++i) {
        float value = static_cast<float>(std::rand()) / RAND_MAX * 100.0f;
        const char* ct;

        if (value < 25.0f) {
            ct = "b";
        } else if (value < 50.0f) {
            ct = "g";
        } else if (value < 75.0f) {
            ct = "y";
        } else {
            ct = "r";
        }

        LOG_INFO("senser value: %s|%.2f|", ct, value);
    }

    LOG_DEBUG("DEBUG test message.");
    LOG_WARN("WARN_ test message.");
    LOG_ERROR("ERROR_ test message.");

    return 0;
}
