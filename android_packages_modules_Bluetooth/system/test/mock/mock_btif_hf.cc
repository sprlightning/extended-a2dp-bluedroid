/*
 * Copyright 2024 The Android Open Source Project
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
/*
 * Generated mock file from original source file
 *   Functions generated:0
 *
 *  mockcify.pl ver 0.7.0
 */

// Mock include file to share data between tests and mock
#include "test/mock/mock_btif_hf.h"

#include "test/common/mock_functions.h"

// Original usings

// Mocked internal structures, if any

// TODO(b/369381361) Enfore -Wmissing-prototypes
#pragma GCC diagnostic ignored "-Wmissing-prototypes"

namespace test {
namespace mock {
namespace btif_hf {

// Function state capture and return values, if needed
struct GetInterface GetInterface;
struct IsCallIdle IsCallIdle;
struct IsDuringVoiceRecognition IsDuringVoiceRecognition;
}  // namespace btif_hf
}  // namespace mock
}  // namespace test

// Mocked functions, if any
namespace bluetooth {
namespace headset {
Interface* GetInterface() {
  inc_func_call_count(__func__);
  return test::mock::btif_hf::GetInterface();
}

bool IsCallIdle() {
  inc_func_call_count(__func__);
  return test::mock::btif_hf::IsCallIdle();
}

bool IsDuringVoiceRecognition(RawAddress* bd_addr) {
  inc_func_call_count(__func__);
  return test::mock::btif_hf::IsDuringVoiceRecognition(bd_addr);
}
}  // namespace headset
}  // namespace bluetooth

// Mocked functions complete
// END mockcify generation
