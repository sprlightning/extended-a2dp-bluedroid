/*
 * Copyright 2018 The Android Open Source Project
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

import static org.junit.Assert.assertThrows;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.anyInt;
import static org.mockito.Mockito.anyString;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.accounts.Account;
import android.accounts.AccountManager;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.BluetoothUuid;
import android.bluetooth.SdpPseRecord;
import android.content.Context;
import android.content.pm.PackageManager;
import android.content.res.Resources;
import android.net.Uri;
import android.os.Looper;
import android.os.UserManager;
import android.platform.test.annotations.DisableFlags;
import android.platform.test.annotations.EnableFlags;
import android.platform.test.flag.junit.FlagsParameterization;
import android.platform.test.flag.junit.SetFlagsRule;
import android.provider.CallLog;
import android.test.mock.MockContentProvider;
import android.test.mock.MockContentResolver;

import androidx.test.filters.MediumTest;

import com.android.bluetooth.TestUtils;
import com.android.bluetooth.btservice.AdapterService;
import com.android.bluetooth.btservice.storage.DatabaseManager;
import com.android.bluetooth.flags.Flags;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoRule;

import platform.test.runner.parameterized.ParameterizedAndroidJunit4;
import platform.test.runner.parameterized.Parameters;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

@MediumTest
@RunWith(ParameterizedAndroidJunit4.class)
public class PbapClientServiceTest {
    private static final String REMOTE_DEVICE_ADDRESS = "00:00:00:00:00:00";

    // Constants for SDP. Note that these values come from the native stack, but no centralized
    // constants exist for them as part of the various SDP APIs.
    public static final int SDP_SUCCESS = 0;
    public static final int SDP_FAILED = 1;
    public static final int SDP_BUSY = 2;

    // Constant for testing ACL disconnection events with a bad transport
    public static final int TRANSPORT_UNKNOWN = -1;

    @Rule public final SetFlagsRule mSetFlagsRule;

    private PbapClientService mService = null;
    private BluetoothAdapter mAdapter = null;
    private BluetoothDevice mRemoteDevice;

    @Rule public MockitoRule mockitoRule = MockitoJUnit.rule();
    @Mock private Context mMockContext;
    @Mock private AdapterService mMockAdapterService;
    @Mock private DatabaseManager mDatabaseManager;
    @Mock private PackageManager mMockPackageManager;
    private MockContentResolver mMockContentResolver;
    private MockCallLogProvider mMockCallLogProvider;
    @Mock private Resources mMockResources;
    @Mock private UserManager mMockUserManager;
    @Mock private AccountManager mMockAccountManager;
    @Mock private SdpPseRecord mMockSdpRecord;

    // NEW: Objects for new state machine implementation
    private PbapClientService.PbapClientStateMachineCallback mDeviceCallback;
    @Mock private PbapClientContactsStorage mMockStorage;
    private Map<BluetoothDevice, PbapClientStateMachine> mMockDeviceMap =
            new HashMap<BluetoothDevice, PbapClientStateMachine>();
    @Mock private PbapClientStateMachine mMockDeviceStateMachine;

    @Parameters(name = "{0}")
    public static List<FlagsParameterization> getParams() {
        return FlagsParameterization.progressionOf(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR);
    }

    public PbapClientServiceTest(FlagsParameterization flags) {
        mSetFlagsRule = new SetFlagsRule(flags);
    }

    @Before
    public void setUp() throws Exception {
        TestUtils.setAdapterService(mMockAdapterService);
        doReturn(mDatabaseManager).when(mMockAdapterService).getDatabase();
        doReturn(BluetoothProfile.CONNECTION_POLICY_ALLOWED)
                .when(mDatabaseManager)
                .getProfileConnectionPolicy(any(BluetoothDevice.class), anyInt());
        doReturn(true)
                .when(mDatabaseManager)
                .setProfileConnectionPolicy(any(BluetoothDevice.class), anyInt(), anyInt());

        doReturn("").when(mMockContext).getPackageName();
        doReturn(mMockPackageManager).when(mMockContext).getPackageManager();

        doReturn(mMockResources).when(mMockContext).getResources();
        doReturn(Utils.ACCOUNT_TYPE).when(mMockResources).getString(anyInt());

        mMockContentResolver = new MockContentResolver();
        mMockCallLogProvider = new MockCallLogProvider();
        mMockContentResolver.addProvider(CallLog.AUTHORITY, mMockCallLogProvider);
        doReturn(mMockContentResolver).when(mMockContext).getContentResolver();

        doReturn(AccountManager.VISIBILITY_VISIBLE)
                .when(mMockAccountManager)
                .getAccountVisibility(any(Account.class), anyString());
        doReturn(new Account[] {})
                .when(mMockAccountManager)
                .getAccountsByType(eq(Utils.ACCOUNT_TYPE));
        TestUtils.mockGetSystemService(
                mMockContext, Context.ACCOUNT_SERVICE, AccountManager.class, mMockAccountManager);

        doReturn(false).when(mMockUserManager).isUserUnlocked();
        TestUtils.mockGetSystemService(
                mMockContext, Context.USER_SERVICE, UserManager.class, mMockUserManager);

        // new for mock storage
        doAnswer(
                        invocation -> {
                            BluetoothDevice device = (BluetoothDevice) invocation.getArgument(0);
                            return Utils.getAccountForDevice(device);
                        })
                .when(mMockStorage)
                .getStorageAccountForDevice(any(BluetoothDevice.class));

        mAdapter = BluetoothAdapter.getDefaultAdapter();
        Assert.assertNotNull(mAdapter);
        mRemoteDevice = mAdapter.getRemoteDevice(REMOTE_DEVICE_ADDRESS);

        if (Looper.myLooper() == null) {
            Looper.prepare();
        }

        mService = new PbapClientService(mMockContext, mMockStorage, mMockDeviceMap);
        mService.start();
        mService.setAvailable(true);

        // new
        doReturn(BluetoothProfile.STATE_CONNECTED)
                .when(mMockDeviceStateMachine)
                .getConnectionState();
        mMockDeviceMap.put(mRemoteDevice, mMockDeviceStateMachine);
        mDeviceCallback = mService.new PbapClientStateMachineCallback(mRemoteDevice);
    }

    @After
    public void tearDown() throws Exception {
        try {
            if (mService != null) {
                mService.stop();
                mService = null;
            }
        } finally {
            TestUtils.clearAdapterService(mMockAdapterService);
        }
    }

    // *********************************************************************************************
    // * Initialize Service
    // *********************************************************************************************

    @Test
    public void testInitialize() {
        assertThat(PbapClientService.getPbapClientService()).isNotNull();
    }

    @Test
    public void testSetPbapClientService_withNull() {
        PbapClientService.setPbapClientService(null);

        assertThat(PbapClientService.getPbapClientService()).isNull();
    }

    // *********************************************************************************************
    // * Incoming Events
    // *********************************************************************************************

    // NEW: PbapClientStateMachineCallback events from devices

    @Test
    @EnableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void onConnectionStateChanged_DisconnectedToConnecting_eventIgnored() {
        doReturn(BluetoothProfile.STATE_CONNECTING)
                .when(mMockDeviceStateMachine)
                .getConnectionState();
        mDeviceCallback.onConnectionStateChanged(
                BluetoothProfile.STATE_DISCONNECTED, BluetoothProfile.STATE_CONNECTING);
        assertThat(mMockDeviceMap.containsKey(mRemoteDevice)).isTrue();
    }

    @Test
    @EnableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void onConnectionStateChanged_ConnectingToConnected_eventIgnored() {
        doReturn(BluetoothProfile.STATE_CONNECTED)
                .when(mMockDeviceStateMachine)
                .getConnectionState();
        mDeviceCallback.onConnectionStateChanged(
                BluetoothProfile.STATE_DISCONNECTED, BluetoothProfile.STATE_CONNECTING);
        assertThat(mMockDeviceMap.containsKey(mRemoteDevice)).isTrue();
    }

    @Test
    @EnableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void onConnectionStateChanged_ConnectingToDisconnected_deviceCleanedUp() {
        doReturn(BluetoothProfile.STATE_DISCONNECTED)
                .when(mMockDeviceStateMachine)
                .getConnectionState();
        mDeviceCallback.onConnectionStateChanged(
                BluetoothProfile.STATE_CONNECTING, BluetoothProfile.STATE_DISCONNECTED);
        assertThat(mMockDeviceMap.containsKey(mRemoteDevice)).isFalse();
    }

    @Test
    @EnableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void onConnectionStateChanged_ConnectedToDisonnecting_eventIgnored() {
        doReturn(BluetoothProfile.STATE_DISCONNECTING)
                .when(mMockDeviceStateMachine)
                .getConnectionState();
        mDeviceCallback.onConnectionStateChanged(
                BluetoothProfile.STATE_CONNECTED, BluetoothProfile.STATE_DISCONNECTING);
        assertThat(mMockDeviceMap.containsKey(mRemoteDevice)).isTrue();
    }

    @Test
    @EnableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void onConnectionStateChanged_DisconnectingToDisonnected_deviceCleanedUp() {
        doReturn(BluetoothProfile.STATE_DISCONNECTED)
                .when(mMockDeviceStateMachine)
                .getConnectionState();
        mDeviceCallback.onConnectionStateChanged(
                BluetoothProfile.STATE_DISCONNECTING, BluetoothProfile.STATE_DISCONNECTED);
        assertThat(mMockDeviceMap.containsKey(mRemoteDevice)).isFalse();
    }

    // OLD: Account state changes from PbapClientAccountManager

    @Test
    @DisableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void onAccountsChanged_fromNulltoEmpty_tryDownloadIfConnectedCalled() {
        PbapClientStateMachineOld sm = mock(PbapClientStateMachineOld.class);
        mService.mPbapClientStateMachineOldMap.put(mRemoteDevice, sm);

        PbapClientService.PbapClientAccountManagerCallback callback =
                mService.new PbapClientAccountManagerCallback();
        callback.onAccountsChanged(null, new ArrayList<Account>());

        verify(sm).tryDownloadIfConnected();
    }

    @Test
    @DisableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void onAccountsChanged_fromEmptyToOne_tryDownloadIfConnectedNotCalled() {
        PbapClientStateMachineOld sm = mock(PbapClientStateMachineOld.class);
        mService.mPbapClientStateMachineOldMap.put(mRemoteDevice, sm);

        PbapClientService.PbapClientAccountManagerCallback callback =
                mService.new PbapClientAccountManagerCallback();
        Account acc = mock(Account.class);
        callback.onAccountsChanged(new ArrayList<Account>(), new ArrayList<>(Arrays.asList(acc)));

        verify(sm, never()).tryDownloadIfConnected();
    }

    // BOTH: ACL state changes from AdapterService

    // old

    @Test
    @DisableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void aclDisconnected_withLeTransport_doesNotCallDisconnect() {
        PbapClientStateMachineOld sm = mock(PbapClientStateMachineOld.class);
        when(sm.getConnectionState(mRemoteDevice)).thenReturn(BluetoothProfile.STATE_CONNECTED);
        mService.mPbapClientStateMachineOldMap.put(mRemoteDevice, sm);

        mService.aclDisconnected(mRemoteDevice, BluetoothDevice.TRANSPORT_LE);
        TestUtils.waitForLooperToFinishScheduledTask(Looper.getMainLooper());

        verify(sm, never()).disconnect(mRemoteDevice);
    }

    @Test
    @DisableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void aclDisconnected_withBrEdrTransport_callsDisconnect() {
        PbapClientStateMachineOld sm = mock(PbapClientStateMachineOld.class);
        when(sm.getConnectionState(mRemoteDevice)).thenReturn(BluetoothProfile.STATE_CONNECTED);
        mService.mPbapClientStateMachineOldMap.put(mRemoteDevice, sm);

        mService.aclDisconnected(mRemoteDevice, BluetoothDevice.TRANSPORT_BREDR);
        TestUtils.waitForLooperToFinishScheduledTask(Looper.getMainLooper());

        verify(sm).disconnect(mRemoteDevice);
    }

    // new

    @Test
    @EnableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testOnBrEdrAclDisconnected_forConnectedDevice_deviceCleanedUp() {
        mService.aclDisconnected(mRemoteDevice, BluetoothDevice.TRANSPORT_BREDR);
        TestUtils.waitForLooperToFinishScheduledTask(Looper.getMainLooper());
        verify(mMockDeviceStateMachine, times(1)).disconnect();
    }

    @Test
    @EnableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testOnBrEdrAclDisconnected_forDisconnectedDevice_eventDropped() {
        mMockDeviceMap.clear();
        mService.aclDisconnected(mRemoteDevice, BluetoothDevice.TRANSPORT_BREDR);
        TestUtils.waitForLooperToFinishScheduledTask(Looper.getMainLooper());
        verify(mMockDeviceStateMachine, never()).disconnect();
    }

    @Test
    @EnableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testOnLeAclDisconnected_forConnectedDevice_eventDropped() {
        mService.aclDisconnected(mRemoteDevice, BluetoothDevice.TRANSPORT_LE);
        TestUtils.waitForLooperToFinishScheduledTask(Looper.getMainLooper());
        verify(mMockDeviceStateMachine, never()).disconnect();
    }

    @Test
    @EnableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testOnUnknownAclDisconnected_forConnectedDevice_deviceCleanedUp() {
        mService.aclDisconnected(mRemoteDevice, TRANSPORT_UNKNOWN);
        TestUtils.waitForLooperToFinishScheduledTask(Looper.getMainLooper());
        verify(mMockDeviceStateMachine, never()).disconnect();
    }

    // BOTH: HFP HF State changes

    // old

    @Test
    @DisableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void headsetClientConnectionStateChanged_hfpCallLogIsRemoved() {
        mService.handleHeadsetClientConnectionStateChanged(
                mRemoteDevice,
                BluetoothProfile.STATE_CONNECTED,
                BluetoothProfile.STATE_DISCONNECTED);

        assertThat(mMockCallLogProvider.getMostRecentlyDeletedDevice())
                .isEqualTo(mRemoteDevice.getAddress());
    }

    // new

    @Test
    @EnableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testOnHfpClientDisconnectedForConnectedDevice_callLogsCleanedUp() {
        mService.handleHeadsetClientConnectionStateChanged(
                mRemoteDevice,
                BluetoothProfile.STATE_DISCONNECTING,
                BluetoothProfile.STATE_DISCONNECTED);
        Account account = Utils.getAccountForDevice(mRemoteDevice);
        verify(mMockStorage, times(1)).removeCallHistory(eq(account));
    }

    @Test
    @EnableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testOnHfpClientDisconnectedForDisconnectedDevice_callLogsCleanedUp() {
        mMockDeviceMap.clear();
        mService.handleHeadsetClientConnectionStateChanged(
                mRemoteDevice,
                BluetoothProfile.STATE_DISCONNECTING,
                BluetoothProfile.STATE_DISCONNECTED);
        Account account = Utils.getAccountForDevice(mRemoteDevice);
        verify(mMockStorage, times(1)).removeCallHistory(eq(account));
    }

    // OLD: Device state machines cleans up

    @Test
    @DisableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void cleanUpDevice() {
        PbapClientStateMachineOld sm = mock(PbapClientStateMachineOld.class);
        mService.mPbapClientStateMachineOldMap.put(mRemoteDevice, sm);

        mService.cleanupDevice(mRemoteDevice);

        assertThat(mService.mPbapClientStateMachineOldMap).doesNotContainKey(mRemoteDevice);
    }

    // NEW: SDP Events from AdapterService

    @Test
    @EnableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testOnSdpRecordReceived_deviceConnected_eventForwarded() {
        mService.receiveSdpSearchRecord(
                mRemoteDevice, SDP_SUCCESS, mMockSdpRecord, BluetoothUuid.PBAP_PSE);
        verify(mMockDeviceStateMachine, times(1))
                .onSdpResultReceived(eq(SDP_SUCCESS), any(PbapSdpRecord.class));
    }

    @Test
    @EnableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testOnSdpResultReceived_deviceDisconnected_eventDropped() {
        mMockDeviceMap.clear();
        mService.receiveSdpSearchRecord(
                mRemoteDevice, SDP_SUCCESS, mMockSdpRecord, BluetoothUuid.PBAP_PSE);
        verify(mMockDeviceStateMachine, never())
                .onSdpResultReceived(anyInt(), any(PbapSdpRecord.class));
    }

    @Test
    @EnableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testOnSdpResultReceived_nullRecord_eventDropped() {
        mService.receiveSdpSearchRecord(mRemoteDevice, SDP_SUCCESS, null, BluetoothUuid.PBAP_PSE);
        verify(mMockDeviceStateMachine, never())
                .onSdpResultReceived(anyInt(), any(PbapSdpRecord.class));
    }

    @Test
    @EnableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testOnSdpResultReceived_wrongUuid_eventDropped() {
        mService.receiveSdpSearchRecord(
                mRemoteDevice, SDP_SUCCESS, mMockSdpRecord, /* wrong */ BluetoothUuid.MNS);
        verify(mMockDeviceStateMachine, never())
                .onSdpResultReceived(anyInt(), any(PbapSdpRecord.class));
    }

    @Test
    @EnableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testOnSdpResultReceived_statusFailed_eventForwarded() {
        mService.receiveSdpSearchRecord(
                mRemoteDevice, SDP_FAILED, mMockSdpRecord, /* wrong */ BluetoothUuid.PBAP_PSE);
        verify(mMockDeviceStateMachine, times(1))
                .onSdpResultReceived(eq(SDP_FAILED), any(PbapSdpRecord.class));
    }

    @Test
    @EnableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testOnSdpResultReceived_statusBusy_eventForwarded() {
        mService.receiveSdpSearchRecord(
                mRemoteDevice, SDP_BUSY, mMockSdpRecord, /* wrong */ BluetoothUuid.PBAP_PSE);
        verify(mMockDeviceStateMachine, times(1))
                .onSdpResultReceived(eq(SDP_BUSY), any(PbapSdpRecord.class));
    }

    // *********************************************************************************************
    // * API Methods
    // *********************************************************************************************

    // getPbapClientService (available) -> this
    @Test
    public void testGetService_serviceAvailable_returnsThis() {
        assertThat(PbapClientService.getPbapClientService()).isEqualTo(mService);
    }

    // getPbapClientService (unavailable) -> null
    @Test
    public void testGetService_serviceUnavailable_returnsNull() {
        mService.setAvailable(false);
        assertThat(PbapClientService.getPbapClientService()).isNull();
    }

    // getPbapClientService (unset after stop) -> null
    @Test
    public void testGetService_serviceStopped_returnsNull() {
        mService.stop();
        mService = null;
        assertThat(PbapClientService.getPbapClientService()).isNull();
    }

    // connect (policy allowed) -> connect/true

    // old

    @Test
    @DisableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testConnect_onOld_onAllowedAndUnconnectedDevice_deviceCreatedAndIsConnecting() {
        mService.mPbapClientStateMachineOldMap.clear();
        assertThat(mService.connect(mRemoteDevice)).isTrue();

        // Clean up and wait for it to complete
        PbapClientStateMachineOld smOld = mService.mPbapClientStateMachineOldMap.get(mRemoteDevice);
        assertThat(smOld).isNotNull();
        smOld.doQuit();
    }

    // new

    @Test
    @EnableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testConnect_onAllowedAndUnconnectedDevice_deviceCreatedAndIsConnecting() {
        mMockDeviceMap.clear();
        assertThat(mService.connect(mRemoteDevice)).isTrue();

        // Clean up and wait for it to complete
        PbapClientStateMachine sm = mMockDeviceMap.get(mRemoteDevice);
        assertThat(sm).isNotNull();
        sm.disconnect();
        TestUtils.waitForLooperToFinishScheduledTask(sm.getHandler().getLooper());
    }

    // connect (device null) -> false
    @Test
    public void testConnect_onNullDevice_throwsIllegalArgumentException() {
        assertThrows(IllegalArgumentException.class, () -> mService.connect(null));
    }

    // connect (policy forbidden) -> false
    @Test
    public void testConnect_onForbiddenAndUnconnectedDevice_deviceNotCreated() {
        mMockDeviceMap.clear();
        doReturn(BluetoothProfile.CONNECTION_POLICY_FORBIDDEN)
                .when(mDatabaseManager)
                .getProfileConnectionPolicy(any(BluetoothDevice.class), anyInt());
        assertThat(mService.connect(mRemoteDevice)).isFalse();
        assertThat(mService.getConnectionState(mRemoteDevice))
                .isEqualTo(BluetoothProfile.STATE_DISCONNECTED);
    }

    // connect (policy unknown) -> false
    @Test
    public void testConnect_onUnknownAndUnconnectedDevice_deviceNotCreated() {
        mMockDeviceMap.clear();
        doReturn(BluetoothProfile.CONNECTION_POLICY_UNKNOWN)
                .when(mDatabaseManager)
                .getProfileConnectionPolicy(any(BluetoothDevice.class), anyInt());
        assertThat(mService.connect(mRemoteDevice)).isFalse();
    }

    // connect (already connected) -> false

    // old

    @Test
    @DisableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testConnect_onOld_onAllowedAndConnectedDevice_connectNotCalled() {
        PbapClientStateMachineOld sm = mock(PbapClientStateMachineOld.class);
        mService.mPbapClientStateMachineOldMap.put(mRemoteDevice, sm);
        assertThat(mService.connect(mRemoteDevice)).isFalse();
    }

    // new

    @Test
    @EnableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testConnect_onAllowedAndConnectedDevice_connectNotCalled() {
        // existing/previous connection setup in setUp()
        assertThat(mService.connect(mRemoteDevice)).isFalse();
    }

    // connect (at device limit) -> false

    // old

    @Test
    @DisableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void
            testConnect_onOld_donAllowedAndUnconnectedDeviceWithTenConnected_connectNotCalled() {
        // Create 10 connected devices
        for (int i = 1; i <= 10; i++) {
            BluetoothDevice remoteDevice = TestUtils.getTestDevice(mAdapter, i);
            PbapClientStateMachineOld sm = mock(PbapClientStateMachineOld.class);
            mService.mPbapClientStateMachineOldMap.put(remoteDevice, sm);
        }

        assertThat(mService.connect(mRemoteDevice)).isFalse();
    }

    // new

    @Test
    @EnableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testConnect_onAllowedAndUnconnectedDeviceWithTenConnected_connectNotCalled() {
        // Create 10 connected devices
        for (int i = 1; i <= 10; i++) {
            BluetoothDevice remoteDevice = TestUtils.getTestDevice(mAdapter, i);
            mMockDeviceMap.put(remoteDevice, mMockDeviceStateMachine);
        }

        assertThat(mService.connect(mRemoteDevice)).isFalse();
    }

    // disconnect (device connected) -> disconnect/true

    // old

    @Test
    @DisableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testDisconnect_onOld_onConnectedDevice_deviceDisconnectRequested() {
        PbapClientStateMachineOld sm = mock(PbapClientStateMachineOld.class);
        when(sm.getConnectionState()).thenReturn(BluetoothProfile.STATE_CONNECTED);
        mService.mPbapClientStateMachineOldMap.put(mRemoteDevice, sm);
        assertThat(mService.disconnect(mRemoteDevice)).isTrue();
        verify(sm, times(1)).disconnect(eq(mRemoteDevice));
    }

    // new

    @Test
    @EnableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testDisconnect_onConnectedDevice_deviceDisconnectRequested() {
        assertThat(mService.disconnect(mRemoteDevice)).isTrue();
        verify(mMockDeviceStateMachine, times(1)).disconnect();
    }

    // disconnect (device DNE) -> false
    @Test
    public void testDisconnect_onUnknownDevice_deviceNotCreatedAndDisconnectNotCalled() {
        mMockDeviceMap.clear();
        assertThat(mService.disconnect(mRemoteDevice)).isFalse();
    }

    // disconnect (device null) -> false
    @Test
    public void testDisconnect_onNullDevice_throwsIllegalArgumentException() {
        assertThrows(IllegalArgumentException.class, () -> mService.disconnect(null));
    }

    // getConnectedDevices (device connected) -> has devices

    // old

    @Test
    @DisableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testGetConnectedDevices_onOld_oneDeviceConnected_returnsConnectedDevice() {
        PbapClientStateMachineOld sm = mock(PbapClientStateMachineOld.class);
        when(sm.getConnectionState()).thenReturn(BluetoothProfile.STATE_CONNECTED);
        mService.mPbapClientStateMachineOldMap.put(mRemoteDevice, sm);

        assertThat(mService.getConnectedDevices()).contains(mRemoteDevice);
    }

    // new

    @Test
    @EnableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testGetConnectedDevices_oneDeviceConnected_returnsConnectedDevice() {
        doReturn(BluetoothProfile.STATE_CONNECTED)
                .when(mMockDeviceStateMachine)
                .getConnectionState();
        assertThat(mService.getConnectedDevices())
                .isEqualTo(Arrays.asList(new BluetoothDevice[] {mRemoteDevice}));
    }

    // getConnectedDevices (no device connected) -> empty
    @Test
    public void testGetConnectedDevices_noDevicesConnected_returnsNoDevices() {
        doReturn(BluetoothProfile.STATE_DISCONNECTED)
                .when(mMockDeviceStateMachine)
                .getConnectionState();
        assertThat(mService.getConnectedDevices()).isEmpty();
    }

    // getDevicesMatchingConnectionStates (connected, one device connected)

    // old

    @Test
    @DisableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testGetDevicesMatchingConnectionStates_onOld_connectedWithDevice_returnsDevice() {
        PbapClientStateMachineOld sm = mock(PbapClientStateMachineOld.class);
        when(sm.getConnectionState()).thenReturn(BluetoothProfile.STATE_CONNECTED);
        mService.mPbapClientStateMachineOldMap.put(mRemoteDevice, sm);

        assertThat(
                        mService.getDevicesMatchingConnectionStates(
                                new int[] {BluetoothProfile.STATE_CONNECTED}))
                .isEqualTo(Arrays.asList(new BluetoothDevice[] {mRemoteDevice}));
    }

    // new

    @Test
    @EnableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testGetDevicesMatchingConnectionStates_connectedWithDevice_returnsDevice() {
        doReturn(BluetoothProfile.STATE_CONNECTED)
                .when(mMockDeviceStateMachine)
                .getConnectionState();
        assertThat(
                        mService.getDevicesMatchingConnectionStates(
                                new int[] {BluetoothProfile.STATE_CONNECTED}))
                .isEqualTo(Arrays.asList(new BluetoothDevice[] {mRemoteDevice}));
    }

    // getDevicesMatchingConnectionStates (connected, no device connected) -> empty
    @Test
    public void testGetDevicesMatchingConnectionStates_connectedWithNoDevice_returnsEmptyList() {
        doReturn(BluetoothProfile.STATE_DISCONNECTED)
                .when(mMockDeviceStateMachine)
                .getConnectionState();
        assertThat(
                        mService.getDevicesMatchingConnectionStates(
                                new int[] {BluetoothProfile.STATE_CONNECTED}))
                .isEmpty();
    }

    // getConnectionState (device connected) -> has device

    // old

    @Test
    @DisableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testGetConnectionState_onOld_onConnectedDevice_returnsConnected() {
        PbapClientStateMachineOld sm = mock(PbapClientStateMachineOld.class);
        when(sm.getConnectionState(eq(mRemoteDevice))).thenReturn(BluetoothProfile.STATE_CONNECTED);
        mService.mPbapClientStateMachineOldMap.put(mRemoteDevice, sm);

        assertThat(mService.getConnectionState(mRemoteDevice))
                .isEqualTo(BluetoothProfile.STATE_CONNECTED);
    }

    // new

    @Test
    @EnableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testGetConnectionState_onConnectedDevice_returnsConnected() {
        doReturn(BluetoothProfile.STATE_CONNECTED)
                .when(mMockDeviceStateMachine)
                .getConnectionState();
        assertThat(mService.getConnectionState(mRemoteDevice))
                .isEqualTo(BluetoothProfile.STATE_CONNECTED);
    }

    // getConnectionState (device null) -> exception
    @Test
    public void testGetConnectionState_onNullDevice_throwsIllegalArgumentException() {
        assertThrows(IllegalArgumentException.class, () -> mService.getConnectionState(null));
    }

    // getConnectionState (device DNE) -> disconnected
    @Test
    public void testGetConnectionState_onDeviceDoesNotExist_returnsDisconnected() {
        mMockDeviceMap.clear();
        assertThat(mService.getConnectionState(mRemoteDevice))
                .isEqualTo(BluetoothProfile.STATE_DISCONNECTED);
    }

    // setConnectionPolicy (allowed -> connect) -> connect/true

    @Test
    public void testSetConnectionPolicy_toAllowed_connectIssued() {
        assertThat(
                        mService.setConnectionPolicy(
                                mRemoteDevice, BluetoothProfile.CONNECTION_POLICY_ALLOWED))
                .isTrue();
    }

    // setConnectionPolicy (forbidden -> disconnect) -> discount/true

    // old

    @Test
    @DisableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testSetConnectionPolicy_onOld_toForbidden_disconnectIssued() {
        PbapClientStateMachineOld sm = mock(PbapClientStateMachineOld.class);
        mService.mPbapClientStateMachineOldMap.put(mRemoteDevice, sm);
        assertThat(
                        mService.setConnectionPolicy(
                                mRemoteDevice, BluetoothProfile.CONNECTION_POLICY_FORBIDDEN))
                .isTrue();
        verify(sm, times(1)).disconnect(eq(mRemoteDevice));
    }

    // new

    @Test
    @EnableFlags(Flags.FLAG_PBAP_CLIENT_STORAGE_REFACTOR)
    public void testSetConnectionPolicy_toForbidden_disconnectIssued() {
        assertThat(
                        mService.setConnectionPolicy(
                                mRemoteDevice, BluetoothProfile.CONNECTION_POLICY_FORBIDDEN))
                .isTrue();
        verify(mMockDeviceStateMachine, times(1)).disconnect();
    }

    // setConnectionPolicy (device null) -> exception
    @Test
    public void testSetConnectionPolicy_onNullDevice_throwsIllegalArgumentException() {
        assertThrows(
                IllegalArgumentException.class,
                () ->
                        mService.setConnectionPolicy(
                                null, BluetoothProfile.CONNECTION_POLICY_ALLOWED));
    }

    // setConnectionPolicy (database call fails) -> false
    @Test
    public void testSetConnectionPolicy_databaseCallFails_returnsFalse() {
        doReturn(false)
                .when(mDatabaseManager)
                .setProfileConnectionPolicy(any(BluetoothDevice.class), anyInt(), anyInt());
        assertThat(
                        mService.setConnectionPolicy(
                                mRemoteDevice, BluetoothProfile.CONNECTION_POLICY_ALLOWED))
                .isFalse();
    }

    // getConnectionPolicy -> returns what we set in setup() (allowed)
    @Test
    public void testGetConnectionPolicy_onKnownDevice_returnsAllowed() {
        assertThat(mService.getConnectionPolicy(mRemoteDevice))
                .isEqualTo(BluetoothProfile.CONNECTION_POLICY_ALLOWED);
    }

    // getConnectionPolicy (device null) -> exception
    @Test
    public void testGetConnectionPolicy_onNullDevice_throwsIllegalArgumentException() {
        assertThrows(IllegalArgumentException.class, () -> mService.getConnectionPolicy(null));
    }

    // *********************************************************************************************
    // * Debug/Dump/toString()
    // *********************************************************************************************

    @Test
    public void testDump() {
        StringBuilder sb = new StringBuilder();
        mService.dump(sb);
        String dumpContents = sb.toString();
        assertThat(dumpContents).isNotNull();
        assertThat(dumpContents.length()).isNotEqualTo(0);
    }

    // *********************************************************************************************
    // * Fake Call Log Provider
    // *********************************************************************************************

    private static class MockCallLogProvider extends MockContentProvider {
        private String mMostRecentlyDeletedDevice = null;

        @Override
        public int delete(Uri uri, String selection, String[] selectionArgs) {
            if (selectionArgs != null && selectionArgs.length > 0) {
                mMostRecentlyDeletedDevice = selectionArgs[0];
            }
            return 0;
        }

        public String getMostRecentlyDeletedDevice() {
            return mMostRecentlyDeletedDevice;
        }
    }
}
