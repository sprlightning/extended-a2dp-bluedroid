/******************************************************************************
 *
 *  Copyright 2002-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

/******************************************************************************
 *
 *  This file contains the definition of the btm control block.
 *
 ******************************************************************************/

#include <bluetooth/log.h>

#include <memory>
#include <string>

#include "common/strings.h"
#include "main/shim/dumpsys.h"
#include "stack/btm/btm_int_types.h"
#include "stack/include/security_client_callbacks.h"
#include "types/raw_address.h"

// TODO(b/369381361) Enfore -Wmissing-prototypes
#pragma GCC diagnostic ignored "-Wmissing-prototypes"

using namespace bluetooth;

/* Global BTM control block structure
 */
tBTM_CB btm_cb;

/*******************************************************************************
 *
 * Function         btm_init
 *
 * Description      This function is called at BTM startup to allocate the
 *                  control block (if using dynamic memory), and initializes the
 *                  tracing level.  It then initializes the various components
 *                  of btm.
 *
 * Returns          void
 *
 ******************************************************************************/
void btm_init(void) {
  btm_cb.Init();
  get_security_client_interface().BTM_Sec_Init();
}

/** This function is called to free dynamic memory and system resource allocated by btm_init */
void btm_free(void) {
  get_security_client_interface().BTM_Sec_Free();
  btm_cb.Free();
}

constexpr size_t kMaxLogHistoryTagLength = 6;
constexpr size_t kMaxLogHistoryMsgLength = 25;

static void btm_log_history(const std::string& tag, const char* addr, const std::string& msg,
                            const std::string& extra) {
  if (btm_cb.history_ == nullptr) {
    log::error("BTM_LogHistory has not been constructed or already destroyed !");
    return;
  }

  btm_cb.history_->Push("%-6s %-25s: %s %s", tag.substr(0, kMaxLogHistoryTagLength).c_str(),
                        msg.substr(0, kMaxLogHistoryMsgLength).c_str(), addr, extra.c_str());
}

void BTM_LogHistory(const std::string& tag, const RawAddress& bd_addr, const std::string& msg,
                    const std::string& extra) {
  btm_log_history(tag, ADDRESS_TO_LOGGABLE_CSTR(bd_addr), msg, extra);
}

void BTM_LogHistory(const std::string& tag, const RawAddress& bd_addr, const std::string& msg) {
  BTM_LogHistory(tag, bd_addr, msg, std::string());
}

void BTM_LogHistory(const std::string& tag, const tBLE_BD_ADDR& ble_bd_addr, const std::string& msg,
                    const std::string& extra) {
  btm_log_history(tag, ADDRESS_TO_LOGGABLE_CSTR(ble_bd_addr), msg, extra);
}

void BTM_LogHistory(const std::string& tag, const tBLE_BD_ADDR& ble_bd_addr,
                    const std::string& msg) {
  BTM_LogHistory(tag, ble_bd_addr, msg, std::string());
}

bluetooth::common::TimestamperInMilliseconds timestamper_in_milliseconds;

using Record = common::TimestampedEntry<std::string>;
const std::string kTimeFormat("%Y-%m-%d %H:%M:%S");

#define DUMPSYS_TAG "shim::btm"
void DumpsysBtm(int fd) {
  LOG_DUMPSYS_TITLE(fd, DUMPSYS_TAG);
  if (btm_cb.history_ != nullptr) {
    std::vector<Record> history = btm_cb.history_->Pull();
    for (auto& record : history) {
      time_t then = record.timestamp / 1000;
      struct tm tm;
      localtime_r(&then, &tm);
      auto s2 = common::StringFormatTime(kTimeFormat, tm);
      LOG_DUMPSYS(fd, " %s.%03u %s", s2.c_str(), static_cast<unsigned int>(record.timestamp % 1000),
                  record.entry.c_str());
    }
  }
}
#undef DUMPSYS_TAG
