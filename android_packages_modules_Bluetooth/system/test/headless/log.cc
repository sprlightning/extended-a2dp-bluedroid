/*
 * Copyright 2022 The Android Open Source Project
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

#include <chrono>
#include <ctime>
#include <string>

#include "common/circular_buffer.h"  // TimestamperInMilliseconds
#include "internal_include/bt_trace.h"
#include "stack/btm/btm_int_types.h"

// TODO(b/369381361) Enfore -Wmissing-prototypes
#pragma GCC diagnostic ignored "-Wmissing-prototypes"

std::chrono::system_clock::time_point _prev = std::chrono::system_clock::now();

extern tBTM_CB btm_cb;

bluetooth::common::TimestamperInMilliseconds timestamper_in_ms;
uint64_t GetTimestampMs() { return timestamper_in_ms.GetTimestamp(); }
