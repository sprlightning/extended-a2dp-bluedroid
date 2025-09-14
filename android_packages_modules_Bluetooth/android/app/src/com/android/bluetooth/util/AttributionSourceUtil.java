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

package com.android.bluetooth.util;

import android.annotation.Nullable;
import android.content.AttributionSource;

/** Class for general helper methods for AttributionSource operations. */
public final class AttributionSourceUtil {

    private AttributionSourceUtil() {}

    /** Gets the last available attribution tag in a chain of {@link AttributionSource}. */
    @Nullable
    public static String getLastAttributionTag(@Nullable AttributionSource source) {
        String attributionTag = null;
        while (source != null) {
            if (source.getAttributionTag() != null) {
                attributionTag = source.getAttributionTag();
            }
            source = source.getNext();
        }
        return attributionTag;
    }
}
