#include <nos/NuggetClient.h>
#include <nos/device.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <limits>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using nos::NuggetClient;

namespace {

constexpr const char *DEV_CITADEL = "/dev/citadel0";
constexpr const char *DEV_DAUNTLESS = "/dev/gsc0";
constexpr size_t MAX_RESPONSE_SIZE = 2048;

const char *defaultDevice() {
  struct stat statbuf;

  if (stat(DEV_CITADEL, &statbuf) == 0) {
    return DEV_CITADEL;
  }

  if (stat(DEV_DAUNTLESS, &statbuf) == 0) {
    return DEV_DAUNTLESS;
  }

  return DEV_CITADEL;
}

class CitadelProxy {
 public:
  explicit CitadelProxy(NuggetClient &client) : client_(client) {}

  uint32_t callApp(int32_t app_id, int32_t arg,
                   const std::vector<uint8_t> &request,
                   std::vector<uint8_t> *response) {
    if (app_id < 0 || app_id > kMaxAppId) {
      std::cerr << "App ID " << app_id << " is outside the app ID range"
                << std::endl;
      return EINVAL;
    }
    if (arg < 0 || arg > std::numeric_limits<uint16_t>::max()) {
      std::cerr << "Argument " << arg
                << " is outside the unsigned 16-bit range" << std::endl;
      return EINVAL;
    }

    const uint8_t app = static_cast<uint8_t>(app_id);
    const uint16_t command = static_cast<uint16_t>(arg);

    std::unique_lock<std::mutex> lock(app_locks_[app]);
    return client_.CallApp(app, command, request, response);
  }

  bool reset() {
    const nos_device &device = *client_.Device();
    return device.ops.reset(device.ctx) == 0;
  }

 private:
  static constexpr int32_t kMaxAppId = std::numeric_limits<uint8_t>::max();

  NuggetClient &client_;
  std::array<std::mutex, kMaxAppId + 1> app_locks_;
};

[[noreturn]] void CitadelEventDispatcher(const nos_device &device) {
  std::cerr << "Event dispatcher startup." << std::endl;
  while (true) {
    if (device.ops.wait_for_interrupt(device.ctx, -1) > 0) {
      std::cerr << "Citadel has dispatched an event" << std::endl;
    } else {
      std::cerr << "Citadel did something unexpected" << std::endl;
    }

    sleep(1);
  }
}

bool hexValue(char c, uint8_t *value) {
  if (c >= '0' && c <= '9') {
    *value = c - '0';
    return true;
  }
  if (c >= 'a' && c <= 'f') {
    *value = c - 'a' + 10;
    return true;
  }
  if (c >= 'A' && c <= 'F') {
    *value = c - 'A' + 10;
    return true;
  }
  return false;
}

bool parseHex(const char *hex, std::vector<uint8_t> *out) {
  std::string input(hex);
  if (input.rfind("0x", 0) == 0 || input.rfind("0X", 0) == 0) {
    input.erase(0, 2);
  }
  if (input.size() % 2 != 0) {
    std::cerr << "Hex request must contain an even number of digits"
              << std::endl;
    return false;
  }

  out->clear();
  out->reserve(input.size() / 2);
  for (size_t i = 0; i < input.size(); i += 2) {
    uint8_t hi;
    uint8_t lo;
    if (!hexValue(input[i], &hi) || !hexValue(input[i + 1], &lo)) {
      std::cerr << "Invalid hex digit in request" << std::endl;
      return false;
    }
    out->push_back(static_cast<uint8_t>((hi << 4) | lo));
  }

  return true;
}

void printHex(const std::vector<uint8_t> &data) {
  for (uint8_t byte : data) {
    printf("%02x", byte);
  }
  printf("\n");
}

void printHelp(const char *arg) {
  std::cerr << "Usage:" << std::endl;
  std::cerr << arg << " reset" << std::endl;
  std::cerr << arg << " call <app_id> <arg> [hex_request]" << std::endl;
  std::cerr << arg << " daemon" << std::endl;
}

bool parseInt32(const char *value, int32_t *out) {
  char *end = nullptr;
  errno = 0;
  const long parsed = strtol(value, &end, 0);
  if (errno != 0 || end == value || *end != '\0' ||
      parsed < std::numeric_limits<int32_t>::min() ||
      parsed > std::numeric_limits<int32_t>::max()) {
    return false;
  }
  *out = static_cast<int32_t>(parsed);
  return true;
}

}  // namespace

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printHelp(argv[0]);
    return 1;
  }

  const char *device = defaultDevice();
  NuggetClient citadel(device);
  citadel.Open();
  if (!citadel.IsOpen()) {
    std::cerr << "Failed to open Citadel client " << device << std::endl;
    return 2;
  }

  CitadelProxy proxy(citadel);
  const std::string command(argv[1]);

  if (command == "reset") {
    const bool ok = proxy.reset();
    citadel.Close();
    std::cout << (ok ? "reset ok" : "reset failed") << std::endl;
    return ok ? 0 : 1;
  }

  if (command == "call") {
    if (argc < 4 || argc > 5) {
      printHelp(argv[0]);
      citadel.Close();
      return 1;
    }

    int32_t app_id;
    int32_t arg;
    if (!parseInt32(argv[2], &app_id) || !parseInt32(argv[3], &arg)) {
      std::cerr << "app_id and arg must be valid integers" << std::endl;
      citadel.Close();
      return 1;
    }

    std::vector<uint8_t> request;
    if (argc == 5 && !parseHex(argv[4], &request)) {
      citadel.Close();
      return 1;
    }

    std::vector<uint8_t> response(MAX_RESPONSE_SIZE);
    const uint32_t status = proxy.callApp(app_id, arg, request, &response);
    std::cout << "Return code: " << status << std::endl;
    std::cout << "Response:" << std::endl;
    printHex(response);
    citadel.Close();
    return status == 0 ? 0 : 1;
  }

  if (command == "daemon") {
    std::thread event_dispatcher(CitadelEventDispatcher,
                                 std::cref(*citadel.Device()));
    event_dispatcher.join();
  }

  printHelp(argv[0]);
  citadel.Close();
  return 1;
}
