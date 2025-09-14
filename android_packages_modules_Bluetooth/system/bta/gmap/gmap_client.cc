/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "bta/le_audio/gmap_client.h"

#include <base/functional/bind.h>
#include <base/functional/callback.h>
#include <base/strings/string_number_conversions.h>
#include <bluetooth/log.h>
#include <com_android_bluetooth_flags.h>
#include <stdio.h>

#include <bitset>
#include <cstdint>
#include <sstream>

#include "bta_gatt_queue.h"
#include "osi/include/properties.h"
#include "stack/include/bt_types.h"
#include "types/raw_address.h"

using namespace bluetooth;
using bluetooth::le_audio::GmapClient;
bool GmapClient::is_offloader_support_gmap_ = false;

void GmapClient::AddFromStorage(const RawAddress &addr, const uint8_t role,
                                const uint16_t role_handle, const uint8_t UGT_feature,
                                const uint16_t UGT_feature_handle) {
  addr_ = addr;
  role_ = role;
  role_handle_ = role_handle;
  UGT_feature_ = UGT_feature;
  UGT_feature_handle_ = UGT_feature_handle;
}

void GmapClient::DebugDump(std::stringstream &stream) {
  if (!IsGmapClientEnabled()) {
    stream << "GmapClient not enabled\n";
    return;
  }
  stream << "GmapClient device: " << addr_ << ", Role: " << role_ << ", ";
  stream << "UGT Feature: " << UGT_feature_ << "\n";
}

bool GmapClient::IsGmapClientEnabled() {
  bool flag = com::android::bluetooth::flags::leaudio_gmap_client();
  bool system_prop = osi_property_get_bool("bluetooth.profile.gmap.enabled", false);

  bool result = flag && system_prop && is_offloader_support_gmap_;
  log::info("GmapClientEnabled={}, flag={}, system_prop={}, offloader_support={}", result,
            system_prop, flag, GmapClient::is_offloader_support_gmap_);
  return result;
}

void GmapClient::UpdateGmapOffloaderSupport(bool value) {
  GmapClient::is_offloader_support_gmap_ = value;
}

bool GmapClient::parseAndSaveGmapRole(uint16_t len, const uint8_t *value) {
  if (len != GmapClient::kGmapRoleLen) {
    log::error("device: {}, Wrong len of GMAP Role characteristic", addr_);
    return false;
  }

  STREAM_TO_UINT8(role_, value);
  log::info("GMAP device: {}, Role: {}", addr_, role_.to_string());
  return true;
}

bool GmapClient::parseAndSaveUGTFeature(uint16_t len, const uint8_t *value) {
  if (len != kGmapUGTFeatureLen) {
    log::error("device: {}, Wrong len of GMAP UGT Feature characteristic", addr_);
    return false;
  }
  STREAM_TO_UINT8(UGT_feature_, value);
  log::info("GMAP device: {}, Feature: {}", addr_, UGT_feature_.to_string());
  return true;
}

std::bitset<8> GmapClient::getRole() { return role_; }

uint16_t GmapClient::getRoleHandle() { return role_handle_; }

void GmapClient::setRoleHandle(uint16_t handle) { role_handle_ = handle; }

std::bitset<8> GmapClient::getUGTFeature() { return UGT_feature_; }

uint16_t GmapClient::getUGTFeatureHandle() { return UGT_feature_handle_; }

void GmapClient::setUGTFeatureHandle(uint16_t handle) { UGT_feature_handle_ = handle; }
