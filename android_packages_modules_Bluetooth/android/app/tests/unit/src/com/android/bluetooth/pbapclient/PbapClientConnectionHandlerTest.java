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

package com.android.bluetooth.pbapclient;

import static com.google.common.truth.Truth.assertThat;

import static org.mockito.Mockito.anyInt;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.ContentResolver;
import android.content.res.Resources;
import android.os.HandlerThread;
import android.os.Looper;

import androidx.test.filters.SmallTest;
import androidx.test.runner.AndroidJUnit4;

import com.android.bluetooth.TestUtils;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoRule;

@SmallTest
@RunWith(AndroidJUnit4.class)
public class PbapClientConnectionHandlerTest {

    private static final String TAG = "ConnHandlerTest";
    private static final String REMOTE_DEVICE_ADDRESS = "00:00:00:00:00:00";

    // Normal supported features for our client
    private static final int SUPPORTED_FEATURES =
            PbapSdpRecord.FEATURE_DOWNLOADING | PbapSdpRecord.FEATURE_DEFAULT_IMAGE_FORMAT;

    private HandlerThread mThread;
    private Looper mLooper;
    private BluetoothDevice mRemoteDevice;

    @Rule public MockitoRule mockitoRule = MockitoJUnit.rule();

    private BluetoothAdapter mAdapter;

    @Mock private PbapClientService mService;

    @Mock private Resources mMockResources;

    @Mock private ContentResolver mMockContentResolver;

    @Mock private PbapClientStateMachineOld mStateMachine;

    private PbapClientConnectionHandler mHandler;

    @Before
    public void setUp() throws Exception {
        if (Looper.myLooper() == null) {
            Looper.prepare();
        }

        mAdapter = BluetoothAdapter.getDefaultAdapter();

        mThread = new HandlerThread("test_handler_thread");
        mThread.start();
        mLooper = mThread.getLooper();
        mRemoteDevice = mAdapter.getRemoteDevice(REMOTE_DEVICE_ADDRESS);

        doReturn(mService).when(mStateMachine).getContext();
        doReturn(mMockContentResolver).when(mService).getContentResolver();
        doReturn(mMockResources).when(mService).getResources();
        doReturn("com.android.bluetooth.pbapclient").when(mMockResources).getString(anyInt());

        mHandler =
                new PbapClientConnectionHandler.Builder()
                        .setLooper(mLooper)
                        .setLocalSupportedFeatures(SUPPORTED_FEATURES)
                        .setClientSM(mStateMachine)
                        .setService(mService)
                        .setRemoteDevice(mRemoteDevice)
                        .build();
    }

    @After
    public void tearDown() throws Exception {
        mLooper.quit();
    }

    @Test
    public void connectSocket_whenBluetoothIsNotEnabled_returnsFalse() {
        assertThat(mHandler.connectSocket()).isFalse();
    }

    @Test
    public void connectSocket_whenBluetoothIsNotEnabled_returnsFalse_withInvalidL2capPsm() {
        PbapSdpRecord record = mock(PbapSdpRecord.class);
        mHandler.setPseRecord(record);

        when(record.getL2capPsm()).thenReturn(PbapClientConnectionHandler.L2CAP_INVALID_PSM);
        assertThat(mHandler.connectSocket()).isFalse();
    }

    @Test
    public void connectSocket_whenBluetoothIsNotEnabled_returnsFalse_withValidL2capPsm() {
        PbapSdpRecord record = mock(PbapSdpRecord.class);
        mHandler.setPseRecord(record);

        when(record.getL2capPsm()).thenReturn(1); // Valid PSM ranges 1 to 30;
        assertThat(mHandler.connectSocket()).isFalse();
    }

    // TODO: Add connectObexSession_returnsTrue

    @Test
    public void connectObexSession_returnsFalse_withoutConnectingSocket() {
        assertThat(mHandler.connectObexSession()).isFalse();
    }

    @Test
    public void abort() {
        PbapSdpRecord record = mock(PbapSdpRecord.class);
        when(record.getL2capPsm()).thenReturn(1); // Valid PSM ranges 1 to 30;
        mHandler.setPseRecord(record);
        mHandler.connectSocket(); // Workaround for setting mSocket as non-null value
        assertThat(mHandler.getSocket()).isNotNull();

        mHandler.abort();

        assertThat(mThread.isInterrupted()).isTrue();
        assertThat(mHandler.getSocket()).isNull();
    }

    @Test
    public void removeCallLog_doesNotCrash() {
        mHandler.removeCallLog();

        // Also test when content resolver is null.
        when(mService.getContentResolver()).thenReturn(null);
        mHandler.removeCallLog();
    }

    @Test
    public void createAndDisconnectWithoutAddingAccount_doesNotCrash() {
        mHandler.obtainMessage(PbapClientConnectionHandler.MSG_DISCONNECT).sendToTarget();
        TestUtils.waitForLooperToFinishScheduledTask(mHandler.getLooper());
        verify(mStateMachine).sendMessage(PbapClientStateMachineOld.MSG_CONNECTION_CLOSED);
    }
}
