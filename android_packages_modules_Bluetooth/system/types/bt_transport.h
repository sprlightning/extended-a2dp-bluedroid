/*
 * Copyright 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <cstdint>

enum tBT_TRANSPORT : uint8_t {
  BT_TRANSPORT_AUTO = 0,
  BT_TRANSPORT_BR_EDR = 1,
  BT_TRANSPORT_LE = 2,
};

#if __has_include(<bluetooth/log.h>)
#include <bluetooth/log.h>

#include <string>

#include "macros.h"

inline std::string bt_transport_text(const tBT_TRANSPORT& transport) {
  switch (transport) {
    CASE_RETURN_TEXT(BT_TRANSPORT_AUTO);
    CASE_RETURN_TEXT(BT_TRANSPORT_BR_EDR);
    CASE_RETURN_TEXT(BT_TRANSPORT_LE);
  }
  RETURN_UNKNOWN_TYPE_STRING(tBT_TRANSPORT, transport);
}

namespace std {
template <>
struct formatter<tBT_TRANSPORT> : enum_formatter<tBT_TRANSPORT> {};
}  // namespace std

#endif
