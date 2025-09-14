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

#include "test/mock/mock_stack_l2cap_interface.h"

#include "stack/include/l2cap_interface.h"

namespace {
bluetooth::testing::stack::l2cap::Mock mock_l2cap_interface;
bluetooth::stack::l2cap::Interface* interface_ = &mock_l2cap_interface;
}  // namespace

void bluetooth::testing::stack::l2cap::reset_interface() { interface_ = &mock_l2cap_interface; }

void bluetooth::testing::stack::l2cap::set_interface(
        bluetooth::stack::l2cap::Interface* interface) {
  interface_ = interface;
}

bluetooth::stack::l2cap::Interface& bluetooth::stack::l2cap::get_interface() { return *interface_; }
