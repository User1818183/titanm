#include "utils.h"

/* Protobuf */
#include <google/protobuf/util/json_util.h>

/* From Nugget OS */
#include <app_nugget.h>
#include <application.h>
#include <fcntl.h>
#include <nos/NuggetClient.h>
#include <nos/device.h>
#include <nostypes.h>
#include <nosutils.h>
#include <nugget/app/avb/avb.pb.h>
#include <rop.h>
#include <stdio.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <vector>

using namespace std;
using namespace nostypes;
using nos::NuggetClient;
using namespace google::protobuf::util;

/*
 * Get rollback index for a specific slot
 * AVBCmdsId::GetRollbackIndex
 */
int callAvbCmd_GetRollbackIndex(const char* slot) {
  NuggetClient client(CITADEL_DEV);
  client.Open();

  if (!client.IsOpen()) {
    std::cerr << "Error opening client" << std::endl;
    return 1;
  }

  App app_id = Avb;
  AvbCmdsId cmd_id = GetRollbackIndex;

  nugget::app::avb::GetRollbackIndexRequest* request =
      new nugget::app::avb::GetRollbackIndexRequest();

  // Parse slot parameter
  uint32_t slot_value = 0;
  try {
    slot_value = std::stoul(slot, nullptr, 10);
  } catch (std::exception& e) {
    std::cerr << "Invalid slot value: " << slot << std::endl;
    delete request;
    client.Close();
    return 1;
  }

  request->set_slot(slot_value);

  // Serialize request
  std::string serialized_request;
  request->SerializeToString(&serialized_request);
  std::vector<uint8_t> request_vector(serialized_request.begin(),
                                      serialized_request.end());

  // Create response buffer
  std::vector<uint8_t> response_vector;
  response_vector.resize(MAX_RESPONSE_SIZE);

  // Call the AVB app
  uint32_t ret = client.CallApp(app_id, cmd_id, request_vector, &response_vector);

  // Parse response
  std::string response_str(response_vector.begin(), response_vector.end());
  nugget::app::avb::GetRollbackIndexResponse* response =
      new nugget::app::avb::GetRollbackIndexResponse();
  response->ParseFromString(response_str);

  // Print results
  initNosPrinter();
  printNosCommand("Avb", "GetRollbackIndex", (Message&)*request,
                  (Message&)*response);

  cout << "Return code: " << ret << endl;

  delete request;
  delete response;

  client.Close();
  return ret;
}

/*
 * Set rollback index for a specific slot
 * AVBCmdsId::SetRollbackIndex
 */
int callAvbCmd_SetRollbackIndex(const char* slot, uint64_t index) {
  NuggetClient client(CITADEL_DEV);
  client.Open();

  if (!client.IsOpen()) {
    std::cerr << "Error opening client" << std::endl;
    return 1;
  }

  App app_id = Avb;
  AvbCmdsId cmd_id = SetRollbackIndex;

  nugget::app::avb::SetRollbackIndexRequest* request =
      new nugget::app::avb::SetRollbackIndexRequest();

  // Parse slot parameter
  uint32_t slot_value = 0;
  try {
    slot_value = std::stoul(slot, nullptr, 10);
  } catch (std::exception& e) {
    std::cerr << "Invalid slot value: " << slot << std::endl;
    delete request;
    client.Close();
    return 1;
  }

  request->set_slot(slot_value);
  request->set_rollback_index(index);

  // Serialize request
  std::string serialized_request;
  request->SerializeToString(&serialized_request);
  std::vector<uint8_t> request_vector(serialized_request.begin(),
                                      serialized_request.end());

  // Create response buffer
  std::vector<uint8_t> response_vector;
  response_vector.resize(MAX_RESPONSE_SIZE);

  // Call the AVB app
  uint32_t ret = client.CallApp(app_id, cmd_id, request_vector, &response_vector);

  // Parse response
  std::string response_str(response_vector.begin(), response_vector.end());
  nugget::app::avb::SetRollbackIndexResponse* response =
      new nugget::app::avb::SetRollbackIndexResponse();
  response->ParseFromString(response_str);

  // Print results
  initNosPrinter();
  printNosCommand("Avb", "SetRollbackIndex", (Message&)*request,
                  (Message&)*response);

  cout << "Return code: " << ret << endl;

  delete request;
  delete response;

  client.Close();
  return ret;
}

/*
 * Read lock state
 * AVBCmdsId::ReadLockState
 */
int callAvbCmd_ReadLockState() {
  NuggetClient client(CITADEL_DEV);
  client.Open();

  if (!client.IsOpen()) {
    std::cerr << "Error opening client" << std::endl;
    return 1;
  }

  App app_id = Avb;
  AvbCmdsId cmd_id = ReadLockState;

  nugget::app::avb::ReadLockStateRequest* request =
      new nugget::app::avb::ReadLockStateRequest();

  // Serialize request
  std::string serialized_request;
  request->SerializeToString(&serialized_request);
  std::vector<uint8_t> request_vector(serialized_request.begin(),
                                      serialized_request.end());

  // Create response buffer
  std::vector<uint8_t> response_vector;
  response_vector.resize(MAX_RESPONSE_SIZE);

  // Call the AVB app
  uint32_t ret = client.CallApp(app_id, cmd_id, request_vector, &response_vector);

  // Parse response
  std::string response_str(response_vector.begin(), response_vector.end());
  nugget::app::avb::ReadLockStateResponse* response =
      new nugget::app::avb::ReadLockStateResponse();
  response->ParseFromString(response_str);

  // Print results
  initNosPrinter();
  printNosCommand("Avb", "ReadLockState", (Message&)*request,
                  (Message&)*response);

  cout << "Return code: " << ret << endl;

  delete request;
  delete response;

  client.Close();
  return ret;
}

/*
 * Write lock state
 * AVBCmdsId::WriteLockState
 */
int callAvbCmd_WriteLockState(bool locked) {
  NuggetClient client(CITADEL_DEV);
  client.Open();

  if (!client.IsOpen()) {
    std::cerr << "Error opening client" << std::endl;
    return 1;
  }

  App app_id = Avb;
  AvbCmdsId cmd_id = WriteLockState;

  nugget::app::avb::WriteLockStateRequest* request =
      new nugget::app::avb::WriteLockStateRequest();

  request->set_locked(locked);

  // Serialize request
  std::string serialized_request;
  request->SerializeToString(&serialized_request);
  std::vector<uint8_t> request_vector(serialized_request.begin(),
                                      serialized_request.end());

  // Create response buffer
  std::vector<uint8_t> response_vector;
  response_vector.resize(MAX_RESPONSE_SIZE);

  // Call the AVB app
  uint32_t ret = client.CallApp(app_id, cmd_id, request_vector, &response_vector);

  // Parse response
  std::string response_str(response_vector.begin(), response_vector.end());
  nugget::app::avb::WriteLockStateResponse* response =
      new nugget::app::avb::WriteLockStateResponse();
  response->ParseFromString(response_str);

  // Print results
  initNosPrinter();
  printNosCommand("Avb", "WriteLockState", (Message&)*request,
                  (Message&)*response);

  cout << "Return code: " << ret << endl;

  delete request;
  delete response;

  client.Close();
  return ret;
}

/*
 * Get AVB version
 * AVBCmdsId::GetVersion
 */
int callAvbCmd_GetVersion() {
  NuggetClient client(CITADEL_DEV);
  client.Open();

  if (!client.IsOpen()) {
    std::cerr << "Error opening client" << std::endl;
    return 1;
  }

  App app_id = Avb;
  AvbCmdsId cmd_id = GetVersion;

  nugget::app::avb::GetVersionRequest* request =
      new nugget::app::avb::GetVersionRequest();

  // Serialize request
  std::string serialized_request;
  request->SerializeToString(&serialized_request);
  std::vector<uint8_t> request_vector(serialized_request.begin(),
                                      serialized_request.end());

  // Create response buffer
  std::vector<uint8_t> response_vector;
  response_vector.resize(MAX_RESPONSE_SIZE);

  // Call the AVB app
  uint32_t ret = client.CallApp(app_id, cmd_id, request_vector, &response_vector);

  // Parse response
  std::string response_str(response_vector.begin(), response_vector.end());
  nugget::app::avb::GetVersionResponse* response =
      new nugget::app::avb::GetVersionResponse();
  response->ParseFromString(response_str);

  // Print results
  initNosPrinter();
  printNosCommand("Avb", "GetVersion", (Message&)*request,
                  (Message&)*response);

  cout << "Return code: " << ret << endl;

  delete request;
  delete response;

  client.Close();
  return ret;
}
