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
package com.android.bluetooth.btservice;

import static android.bluetooth.BluetoothAdapter.SCAN_MODE_CONNECTABLE;
import static android.bluetooth.BluetoothAdapter.SCAN_MODE_NONE;

import static com.google.common.truth.Truth.assertThat;

import static org.mockito.Mockito.anyLong;
import static org.mockito.Mockito.clearInvocations;
import static org.mockito.Mockito.verify;

import android.content.Context;
import android.hardware.display.DisplayManager;
import android.os.test.TestLooper;

import androidx.test.InstrumentationRegistry;
import androidx.test.filters.SmallTest;
import androidx.test.runner.AndroidJUnit4;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoRule;

@SmallTest
@RunWith(AndroidJUnit4.class)
public class AdapterSuspendTest {
    private TestLooper mTestLooper;
    private DisplayManager mDisplayManager;
    private AdapterSuspend mAdapterSuspend;

    @Rule public MockitoRule mockitoRule = MockitoJUnit.rule();
    @Mock private AdapterNativeInterface mAdapterNativeInterface;

    @Before
    public void setUp() throws Exception {
        Context context = InstrumentationRegistry.getTargetContext();
        mTestLooper = new TestLooper();
        mDisplayManager = context.getSystemService(DisplayManager.class);

        mAdapterSuspend =
                new AdapterSuspend(
                        mAdapterNativeInterface, mTestLooper.getLooper(), mDisplayManager);
    }

    private void triggerSuspend() throws Exception {
        mAdapterSuspend.handleSuspend();
    }

    private void triggerResume() throws Exception {
        mAdapterSuspend.handleResume();
    }

    private boolean isSuspended() throws Exception {
        return mAdapterSuspend.isSuspended();
    }

    @Test
    public void testSuspend() throws Exception {
        assertThat(isSuspended()).isFalse();

        triggerSuspend();

        verify(mAdapterNativeInterface).setDefaultEventMaskExcept(anyLong(), anyLong());
        verify(mAdapterNativeInterface)
                .setScanMode(AdapterService.convertScanModeToHal(SCAN_MODE_NONE));
        verify(mAdapterNativeInterface).clearEventFilter();
        verify(mAdapterNativeInterface).clearFilterAcceptList();
        verify(mAdapterNativeInterface).disconnectAllAcls();
        assertThat(isSuspended()).isTrue();
    }

    @Test
    public void testResume() throws Exception {
        triggerSuspend();
        assertThat(isSuspended()).isTrue();

        clearInvocations(mAdapterNativeInterface);
        triggerResume();

        verify(mAdapterNativeInterface).setDefaultEventMaskExcept(0, 0);
        verify(mAdapterNativeInterface).clearEventFilter();
        verify(mAdapterNativeInterface).restoreFilterAcceptList();
        verify(mAdapterNativeInterface)
                .setScanMode(AdapterService.convertScanModeToHal(SCAN_MODE_CONNECTABLE));
        assertThat(isSuspended()).isFalse();
    }
}
