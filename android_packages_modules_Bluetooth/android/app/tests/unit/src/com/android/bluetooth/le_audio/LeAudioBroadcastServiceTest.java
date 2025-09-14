/*
 * Copyright 2021 HIMSA II K/S - www.himsa.com.
 * Represented by EHIMA - www.ehima.com
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

package com.android.bluetooth.le_audio;

import static android.bluetooth.IBluetoothLeAudio.LE_AUDIO_GROUP_ID_INVALID;

import static com.android.bluetooth.bass_client.BassConstants.INVALID_BROADCAST_ID;

import static org.mockito.Mockito.*;

import android.annotation.Nullable;
import android.bluetooth.*;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioManager;
import android.media.BluetoothProfileConnectionInfo;
import android.os.IBinder;
import android.os.Looper;
import android.os.ParcelUuid;
import android.os.RemoteException;
import android.platform.test.annotations.DisableFlags;
import android.platform.test.annotations.EnableFlags;
import android.platform.test.flag.junit.SetFlagsRule;

import androidx.test.filters.MediumTest;
import androidx.test.platform.app.InstrumentationRegistry;
import androidx.test.runner.AndroidJUnit4;

import com.android.bluetooth.TestUtils;
import com.android.bluetooth.Utils;
import com.android.bluetooth.bass_client.BassClientService;
import com.android.bluetooth.btservice.ActiveDeviceManager;
import com.android.bluetooth.btservice.AdapterService;
import com.android.bluetooth.btservice.MetricsLogger;
import com.android.bluetooth.btservice.ServiceFactory;
import com.android.bluetooth.btservice.storage.DatabaseManager;
import com.android.bluetooth.flags.Flags;
import com.android.bluetooth.tbs.TbsService;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.Spy;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoRule;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeoutException;

@MediumTest
@RunWith(AndroidJUnit4.class)
public class LeAudioBroadcastServiceTest {
    private static final int TIMEOUT_MS = 1000;
    private static final int CREATE_BROADCAST_TIMEOUT_MS = 6000;

    @Rule public final SetFlagsRule mSetFlagsRule = new SetFlagsRule();

    private BluetoothAdapter mAdapter;
    private BluetoothDevice mDevice;
    private BluetoothDevice mDevice2;
    private BluetoothDevice mBroadcastDevice;

    private Context mTargetContext;
    private LeAudioService mService;
    private LeAudioIntentReceiver mLeAudioIntentReceiver;
    private LinkedBlockingQueue<Intent> mIntentQueue;
    private boolean onBroadcastToUnicastFallbackGroupChangedCallbackCalled = false;
    @Rule public MockitoRule mockitoRule = MockitoJUnit.rule();

    @Mock private ActiveDeviceManager mActiveDeviceManager;
    @Mock private AdapterService mAdapterService;
    @Mock private DatabaseManager mDatabaseManager;
    @Mock private AudioManager mAudioManager;
    @Mock private LeAudioBroadcasterNativeInterface mLeAudioBroadcasterNativeInterface;
    @Mock private LeAudioNativeInterface mLeAudioNativeInterface;
    @Mock private LeAudioTmapGattServer mTmapGattServer;
    @Mock private BassClientService mBassClientService;
    @Mock private TbsService mTbsService;
    @Mock private MetricsLogger mMetricsLogger;
    @Mock private IBluetoothLeBroadcastCallback mCallbacks;
    @Mock private IBinder mBinder;

    @Spy private LeAudioObjectsFactory mObjectsFactory = LeAudioObjectsFactory.getInstance();
    @Spy private ServiceFactory mServiceFactory = new ServiceFactory();

    private static final String TEST_MAC_ADDRESS = "00:11:22:33:44:55";
    private static final int TEST_BROADCAST_ID = 42;
    private static final int TEST_ADVERTISER_SID = 1234;
    private static final int TEST_PA_SYNC_INTERVAL = 100;
    private static final int TEST_PRESENTATION_DELAY_MS = 345;

    private static final int TEST_CODEC_ID = 42;
    private static final int TEST_CHANNEL_INDEX = 56;

    // For BluetoothLeAudioCodecConfigMetadata
    private static final long TEST_AUDIO_LOCATION_FRONT_LEFT = 0x01;
    private static final long TEST_AUDIO_LOCATION_FRONT_RIGHT = 0x02;

    // For BluetoothLeAudioContentMetadata
    private static final String TEST_PROGRAM_INFO = "Test";
    // German language code in ISO 639-3
    private static final String TEST_LANGUAGE = "deu";
    private static final String TEST_BROADCAST_NAME = "Name Test";

    private static final BluetoothLeAudioCodecConfig LC3_16KHZ_CONFIG =
            new BluetoothLeAudioCodecConfig.Builder()
                    .setCodecType(BluetoothLeAudioCodecConfig.SOURCE_CODEC_TYPE_LC3)
                    .setSampleRate(BluetoothLeAudioCodecConfig.SAMPLE_RATE_16000)
                    .build();
    private static final BluetoothLeAudioCodecConfig LC3_48KHZ_CONFIG =
            new BluetoothLeAudioCodecConfig.Builder()
                    .setCodecType(BluetoothLeAudioCodecConfig.SOURCE_CODEC_TYPE_LC3)
                    .setSampleRate(BluetoothLeAudioCodecConfig.SAMPLE_RATE_48000)
                    .build();

    private static final List<BluetoothLeAudioCodecConfig> INPUT_SELECTABLE_CONFIG_STANDARD =
            List.of(LC3_16KHZ_CONFIG);
    private static final List<BluetoothLeAudioCodecConfig> OUTPUT_SELECTABLE_CONFIG_STANDARD =
            List.of(LC3_16KHZ_CONFIG);

    private static final List<BluetoothLeAudioCodecConfig> INPUT_SELECTABLE_CONFIG_HIGH =
            List.of(LC3_48KHZ_CONFIG);
    private static final List<BluetoothLeAudioCodecConfig> OUTPUT_SELECTABLE_CONFIG_HIGH =
            List.of(LC3_48KHZ_CONFIG);

    @Before
    public void setUp() throws Exception {
        mTargetContext = InstrumentationRegistry.getInstrumentation().getTargetContext();

        doReturn(mBinder).when(mCallbacks).asBinder();
        doNothing().when(mBinder).linkToDeath(any(), eq(0));

        // Use spied objects factory
        doNothing().when(mTmapGattServer).start(anyInt());
        doNothing().when(mTmapGattServer).stop();
        LeAudioObjectsFactory.setInstanceForTesting(mObjectsFactory);
        doReturn(mTmapGattServer).when(mObjectsFactory).getTmapGattServer(any());

        if (Looper.myLooper() == null) {
            Looper.prepare();
        }

        TestUtils.setAdapterService(mAdapterService);
        doReturn(mDatabaseManager).when(mAdapterService).getDatabase();
        doReturn(true).when(mAdapterService).isLeAudioBroadcastSourceSupported();
        doReturn(
                        (long) (1 << BluetoothProfile.LE_AUDIO_BROADCAST)
                                | (1 << BluetoothProfile.LE_AUDIO))
                .when(mAdapterService)
                .getSupportedProfilesBitMask();
        doReturn(mActiveDeviceManager).when(mAdapterService).getActiveDeviceManager();

        mAdapter = BluetoothAdapter.getDefaultAdapter();
        MetricsLogger.setInstanceForTesting(mMetricsLogger);

        LeAudioBroadcasterNativeInterface.setInstance(mLeAudioBroadcasterNativeInterface);
        LeAudioNativeInterface.setInstance(mLeAudioNativeInterface);
        startService();

        mService.mAudioManager = mAudioManager;
        mService.mServiceFactory = mServiceFactory;
        mService.mTbsService = mTbsService;
        when(mServiceFactory.getBassClientService()).thenReturn(mBassClientService);
        // Set up the State Changed receiver
        IntentFilter filter = new IntentFilter();
        filter.setPriority(IntentFilter.SYSTEM_HIGH_PRIORITY);

        mLeAudioIntentReceiver = new LeAudioIntentReceiver();
        filter.addAction(BluetoothLeAudio.ACTION_LE_AUDIO_CONNECTION_STATE_CHANGED);
        mTargetContext.registerReceiver(mLeAudioIntentReceiver, filter);

        mDevice = TestUtils.getTestDevice(mAdapter, 0);
        mDevice2 = TestUtils.getTestDevice(mAdapter, 1);
        mBroadcastDevice = TestUtils.getTestDevice(mAdapter, 1);
        when(mAdapterService.getDeviceFromByte(Utils.getBytesFromAddress("FF:FF:FF:FF:FF:FF")))
                .thenReturn(mBroadcastDevice);

        mIntentQueue = new LinkedBlockingQueue<Intent>();
    }

    @After
    public void tearDown() throws Exception {
        if (mService == null || mAdapter == null) {
            return;
        }
        if (mLeAudioIntentReceiver != null) {
            mTargetContext.unregisterReceiver(mLeAudioIntentReceiver);
        }

        stopService();
        LeAudioBroadcasterNativeInterface.setInstance(null);
        LeAudioNativeInterface.setInstance(null);
        MetricsLogger.setInstanceForTesting(null);
        TestUtils.clearAdapterService(mAdapterService);
        reset(mAudioManager);
    }

    private void startService() throws TimeoutException {
        mService = new LeAudioService(mTargetContext);
        mService.start();
        mService.setAvailable(true);
    }

    private void stopService() throws TimeoutException {
        mService.stop();
        mService = LeAudioService.getLeAudioService();
        Assert.assertNull(mService);
    }

    /** Test getting LeAudio Service */
    @Test
    public void testGetLeAudioService() {
        Assert.assertEquals(mService, LeAudioService.getLeAudioService());
    }

    void verifyBroadcastStarted(int broadcastId, BluetoothLeBroadcastSettings settings) {
        mService.createBroadcast(settings);

        List<BluetoothLeBroadcastSubgroupSettings> settingsList = settings.getSubgroupSettings();

        int[] expectedQualityArray =
                settingsList.stream().mapToInt(setting -> setting.getPreferredQuality()).toArray();
        byte[][] expectedDataArray =
                settingsList.stream()
                        .map(setting -> setting.getContentMetadata().getRawMetadata())
                        .toArray(byte[][]::new);

        verify(mLeAudioBroadcasterNativeInterface)
                .createBroadcast(
                        eq(true),
                        eq(TEST_BROADCAST_NAME),
                        eq(settings.getBroadcastCode()),
                        eq(settings.getPublicBroadcastMetadata().getRawMetadata()),
                        eq(expectedQualityArray),
                        eq(expectedDataArray));

        // Check if broadcast is started automatically when created
        LeAudioStackEvent create_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_CREATED);
        create_event.valueInt1 = broadcastId;
        create_event.valueBool1 = true;
        mService.messageFromNative(create_event);

        verify(mLeAudioBroadcasterNativeInterface).startBroadcast(eq(broadcastId));

        // Notify initial paused state
        LeAudioStackEvent state_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_STATE);
        state_event.valueInt1 = broadcastId;
        state_event.valueInt2 = LeAudioStackEvent.BROADCAST_STATE_PAUSED;
        mService.messageFromNative(state_event);

        // Switch to active streaming
        state_event = new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_STATE);
        state_event.valueInt1 = broadcastId;
        state_event.valueInt2 = LeAudioStackEvent.BROADCAST_STATE_STREAMING;
        mService.messageFromNative(state_event);

        // Check if metadata is requested when the broadcast starts to stream
        verify(mLeAudioBroadcasterNativeInterface).getBroadcastMetadata(eq(broadcastId));
        TestUtils.waitForLooperToFinishScheduledTask(mService.getMainLooper());

        try {
            verify(mCallbacks, times(0)).onBroadcastStartFailed(anyInt());
            verify(mCallbacks, times(1))
                    .onBroadcastStarted(
                            eq(BluetoothStatusCodes.REASON_LOCAL_APP_REQUEST), anyInt());
        } catch (RemoteException e) {
            throw e.rethrowFromSystemServer();
        }
    }

    void verifyBroadcastStopped(int broadcastId) {
        Mockito.clearInvocations(mMetricsLogger);

        mService.stopBroadcast(broadcastId);
        verify(mLeAudioBroadcasterNativeInterface).stopBroadcast(eq(broadcastId));

        LeAudioStackEvent state_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_STATE);
        state_event.valueInt1 = broadcastId;
        state_event.valueInt2 = LeAudioStackEvent.BROADCAST_STATE_STOPPED;
        mService.messageFromNative(state_event);

        // Verify if broadcast is auto-destroyed on stop
        verify(mLeAudioBroadcasterNativeInterface).destroyBroadcast(eq(broadcastId));

        state_event = new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_DESTROYED);
        state_event.valueInt1 = broadcastId;
        mService.messageFromNative(state_event);

        TestUtils.waitForLooperToFinishScheduledTask(mService.getMainLooper());

        // Verify broadcast audio session is logged when session stopped
        verify(mMetricsLogger)
                .logLeAudioBroadcastAudioSession(
                        eq(broadcastId),
                        eq(new int[] {0x2}), // STATS_SESSION_AUDIO_QUALITY_HIGH
                        eq(0),
                        anyLong(),
                        anyLong(),
                        anyLong(),
                        eq(0x3)); // STATS_SESSION_SETUP_STATUS_STREAMING
        try {
            verify(mCallbacks, times(1))
                    .onBroadcastStopped(
                            eq(BluetoothStatusCodes.REASON_LOCAL_APP_REQUEST), anyInt());
            verify(mCallbacks, times(0)).onBroadcastStopFailed(anyInt());
        } catch (RemoteException e) {
            throw e.rethrowFromSystemServer();
        }
    }

    @Test
    public void testCreateBroadcastNative() {
        int broadcastId = 243;
        byte[] code = {0x00, 0x01, 0x00, 0x02};

        synchronized (mService.mBroadcastCallbacks) {
            mService.mBroadcastCallbacks.register(mCallbacks);
        }

        BluetoothLeAudioContentMetadata.Builder meta_builder =
                new BluetoothLeAudioContentMetadata.Builder();
        meta_builder.setLanguage("deu");
        meta_builder.setProgramInfo("Subgroup broadcast info");
        BluetoothLeAudioContentMetadata meta = meta_builder.build();

        verifyBroadcastStarted(broadcastId, buildBroadcastSettingsFromMetadata(meta, code, 1));
    }

    @Test
    public void testCreateBroadcastNativeMultiGroups() {
        int broadcastId = 243;
        byte[] code = {0x00, 0x01, 0x00, 0x02};

        synchronized (mService.mBroadcastCallbacks) {
            mService.mBroadcastCallbacks.register(mCallbacks);
        }

        BluetoothLeAudioContentMetadata.Builder meta_builder =
                new BluetoothLeAudioContentMetadata.Builder();
        meta_builder.setLanguage("deu");
        meta_builder.setProgramInfo("Subgroup broadcast info");
        BluetoothLeAudioContentMetadata meta = meta_builder.build();

        verifyBroadcastStarted(broadcastId, buildBroadcastSettingsFromMetadata(meta, code, 3));
    }

    @Test
    public void testCreateBroadcastNativeFailed() {
        int broadcastId = 243;
        byte[] code = {0x00, 0x01, 0x00, 0x02};

        synchronized (mService.mBroadcastCallbacks) {
            mService.mBroadcastCallbacks.register(mCallbacks);
        }
        Mockito.clearInvocations(mMetricsLogger);

        BluetoothLeAudioContentMetadata.Builder meta_builder =
                new BluetoothLeAudioContentMetadata.Builder();
        meta_builder.setLanguage("deu");
        meta_builder.setProgramInfo("Public broadcast info");
        BluetoothLeAudioContentMetadata meta = meta_builder.build();
        BluetoothLeBroadcastSettings settings = buildBroadcastSettingsFromMetadata(meta, code, 1);
        mService.createBroadcast(settings);

        // Test data with only one subgroup
        int[] expectedQualityArray = {settings.getSubgroupSettings().get(0).getPreferredQuality()};
        byte[][] expectedDataArray = {
            settings.getSubgroupSettings().get(0).getContentMetadata().getRawMetadata()
        };

        verify(mLeAudioBroadcasterNativeInterface)
                .createBroadcast(
                        eq(true),
                        eq(TEST_BROADCAST_NAME),
                        eq(code),
                        eq(settings.getPublicBroadcastMetadata().getRawMetadata()),
                        eq(expectedQualityArray),
                        eq(expectedDataArray));

        LeAudioStackEvent create_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_CREATED);
        create_event.valueInt1 = broadcastId;
        create_event.valueBool1 = false;
        mService.messageFromNative(create_event);

        TestUtils.waitForLooperToFinishScheduledTask(mService.getMainLooper());

        // Verify broadcast audio session is logged when session failed to create
        verify(mMetricsLogger)
                .logLeAudioBroadcastAudioSession(
                        eq(INVALID_BROADCAST_ID),
                        eq(new int[] {0x2}), // STATS_SESSION_AUDIO_QUALITY_HIGH
                        eq(0),
                        eq(0L),
                        eq(0L),
                        eq(0L),
                        eq(0x4)); // STATS_SESSION_SETUP_STATUS_CREATED_FAILED

        try {
            verify(mCallbacks, times(0)).onBroadcastStarted(anyInt(), anyInt());
            verify(mCallbacks, times(1))
                    .onBroadcastStartFailed(eq(BluetoothStatusCodes.ERROR_UNKNOWN));
        } catch (RemoteException e) {
            throw e.rethrowFromSystemServer();
        }
    }

    @Test
    public void testCreateBroadcastTimeout() {
        mSetFlagsRule.enableFlags(Flags.FLAG_LEAUDIO_BROADCAST_DESTROY_AFTER_TIMEOUT);

        byte[] code = {0x00, 0x01, 0x00, 0x02};

        synchronized (mService.mBroadcastCallbacks) {
            mService.mBroadcastCallbacks.register(mCallbacks);
        }
        Mockito.clearInvocations(mMetricsLogger);

        BluetoothLeAudioContentMetadata.Builder meta_builder =
                new BluetoothLeAudioContentMetadata.Builder();
        meta_builder.setLanguage("deu");
        meta_builder.setProgramInfo("Public broadcast info");
        BluetoothLeAudioContentMetadata meta = meta_builder.build();
        BluetoothLeBroadcastSettings settings = buildBroadcastSettingsFromMetadata(meta, code, 1);
        mService.createBroadcast(settings);

        if (Flags.leaudioBigDependsOnAudioState()) {
            try {
                verify(mCallbacks, timeout(CREATE_BROADCAST_TIMEOUT_MS).times(1))
                        .onBroadcastStartFailed(eq(BluetoothStatusCodes.ERROR_TIMEOUT));
            } catch (RemoteException e) {
                throw e.rethrowFromSystemServer();
            }
        } else {
            int broadcastId = 243;
            // Test data with only one subgroup
            int[] expectedQualityArray = {
                settings.getSubgroupSettings().get(0).getPreferredQuality()
            };
            byte[][] expectedDataArray = {
                settings.getSubgroupSettings().get(0).getContentMetadata().getRawMetadata()
            };

            verify(mLeAudioBroadcasterNativeInterface)
                    .createBroadcast(
                            eq(true),
                            eq(TEST_BROADCAST_NAME),
                            eq(code),
                            eq(settings.getPublicBroadcastMetadata().getRawMetadata()),
                            eq(expectedQualityArray),
                            eq(expectedDataArray));

            // Check if broadcast is started automatically when created
            LeAudioStackEvent create_event =
                    new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_CREATED);
            create_event.valueInt1 = broadcastId;
            create_event.valueBool1 = true;
            mService.messageFromNative(create_event);

            // Verify if broadcast is auto-started on start
            verify(mLeAudioBroadcasterNativeInterface).startBroadcast(eq(broadcastId));
            TestUtils.waitForLooperToFinishScheduledTask(mService.getMainLooper());

            try {
                verify(mCallbacks, times(1))
                        .onBroadcastStarted(
                                eq(BluetoothStatusCodes.REASON_LOCAL_APP_REQUEST), anyInt());
            } catch (RemoteException e) {
                throw e.rethrowFromSystemServer();
            }

            // Notify initial paused state
            LeAudioStackEvent state_event =
                    new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_STATE);
            state_event.valueInt1 = broadcastId;
            state_event.valueInt2 = LeAudioStackEvent.BROADCAST_STATE_PAUSED;
            mService.messageFromNative(state_event);

            // Check if broadcast is destroyed after timeout
            verify(mLeAudioBroadcasterNativeInterface, timeout(CREATE_BROADCAST_TIMEOUT_MS))
                    .destroyBroadcast(eq(broadcastId));

            state_event = new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_DESTROYED);
            state_event.valueInt1 = broadcastId;
            mService.messageFromNative(state_event);

            // Verify broadcast audio session is logged when session failed to stream
            verify(mMetricsLogger)
                    .logLeAudioBroadcastAudioSession(
                            eq(broadcastId),
                            eq(new int[] {0x2}), // STATS_SESSION_AUDIO_QUALITY_HIGH
                            eq(0),
                            anyLong(),
                            anyLong(),
                            eq(0L),
                            eq(0x5)); // STATS_SESSION_SETUP_STATUS_STREAMING_FAILED
        }
    }

    @Test
    public void testCreateBroadcast_noAudioQualityUpdate() {
        byte[] code = {0x00, 0x01, 0x00, 0x02};
        int groupId = 1;

        initializeNative();
        prepareConnectedUnicastDevice(groupId, mDevice);

        synchronized (mService.mBroadcastCallbacks) {
            mService.mBroadcastCallbacks.register(mCallbacks);
        }

        BluetoothLeAudioContentMetadata.Builder meta_builder =
                new BluetoothLeAudioContentMetadata.Builder();
        BluetoothLeAudioContentMetadata meta = meta_builder.build();
        BluetoothLeBroadcastSettings settings = buildBroadcastSettingsFromMetadata(meta, code, 1);

        when(mBassClientService.getConnectedDevices()).thenReturn(List.of(mDevice));
        // update input selectable configs to be STANDARD quality
        // keep output selectable configs as HIGH quality
        injectGroupSelectableCodecConfigChanged(
                groupId, INPUT_SELECTABLE_CONFIG_STANDARD, OUTPUT_SELECTABLE_CONFIG_HIGH);
        injectGroupCurrentCodecConfigChanged(groupId, LC3_16KHZ_CONFIG, LC3_48KHZ_CONFIG);

        mService.createBroadcast(settings);

        // Test data with only one subgroup
        // Verify quality is  not updated to standard because HIGH is selectable in output
        int[] expectedQualityArray = {BluetoothLeBroadcastSubgroupSettings.QUALITY_HIGH};
        byte[][] expectedDataArray = {
            settings.getSubgroupSettings().get(0).getContentMetadata().getRawMetadata()
        };

        verify(mLeAudioBroadcasterNativeInterface)
                .createBroadcast(
                        eq(true),
                        eq(TEST_BROADCAST_NAME),
                        eq(code),
                        eq(settings.getPublicBroadcastMetadata().getRawMetadata()),
                        eq(expectedQualityArray),
                        eq(expectedDataArray));
    }

    @Test
    public void testCreateBroadcast_updateAudioQualityToStandard() {
        byte[] code = {0x00, 0x01, 0x00, 0x02};
        int groupId = 1;

        initializeNative();
        prepareConnectedUnicastDevice(groupId, mDevice);

        synchronized (mService.mBroadcastCallbacks) {
            mService.mBroadcastCallbacks.register(mCallbacks);
        }

        BluetoothLeAudioContentMetadata.Builder meta_builder =
                new BluetoothLeAudioContentMetadata.Builder();
        BluetoothLeAudioContentMetadata meta = meta_builder.build();
        BluetoothLeBroadcastSettings settings = buildBroadcastSettingsFromMetadata(meta, code, 1);

        when(mBassClientService.getConnectedDevices()).thenReturn(List.of(mDevice));
        // update output selectable configs to be STANDARD quality
        injectGroupSelectableCodecConfigChanged(
                groupId, INPUT_SELECTABLE_CONFIG_HIGH, OUTPUT_SELECTABLE_CONFIG_STANDARD);
        injectGroupCurrentCodecConfigChanged(groupId, LC3_16KHZ_CONFIG, LC3_48KHZ_CONFIG);

        mService.createBroadcast(settings);

        // Test data with only one subgroup
        // Verify quality is updated to standard per sinks capabilities
        int[] expectedQualityArray = {BluetoothLeBroadcastSubgroupSettings.QUALITY_STANDARD};
        byte[][] expectedDataArray = {
            settings.getSubgroupSettings().get(0).getContentMetadata().getRawMetadata()
        };

        verify(mLeAudioBroadcasterNativeInterface)
                .createBroadcast(
                        eq(true),
                        eq(TEST_BROADCAST_NAME),
                        eq(code),
                        eq(settings.getPublicBroadcastMetadata().getRawMetadata()),
                        eq(expectedQualityArray),
                        eq(expectedDataArray));
    }

    @Test
    public void testStartStopBroadcastNative() {
        int broadcastId = 243;
        byte[] code = {0x00, 0x01, 0x00, 0x02};

        synchronized (mService.mBroadcastCallbacks) {
            mService.mBroadcastCallbacks.register(mCallbacks);
        }

        BluetoothLeAudioContentMetadata.Builder meta_builder =
                new BluetoothLeAudioContentMetadata.Builder();
        meta_builder.setLanguage("deu");
        meta_builder.setProgramInfo("Subgroup broadcast info");
        BluetoothLeAudioContentMetadata meta = meta_builder.build();

        verifyBroadcastStarted(broadcastId, buildBroadcastSettingsFromMetadata(meta, code, 1));
        verifyBroadcastStopped(broadcastId);
    }

    @Test
    public void testBroadcastInvalidBroadcastIdRequest() {
        int broadcastId = 243;

        synchronized (mService.mBroadcastCallbacks) {
            mService.mBroadcastCallbacks.register(mCallbacks);
        }

        // Stop non-existing broadcast
        mService.stopBroadcast(broadcastId);

        TestUtils.waitForLooperToFinishScheduledTask(mService.getMainLooper());

        try {
            verify(mCallbacks, times(0)).onBroadcastStopped(anyInt(), anyInt());
            verify(mCallbacks, times(1))
                    .onBroadcastStopFailed(
                            eq(BluetoothStatusCodes.ERROR_LE_BROADCAST_INVALID_BROADCAST_ID));
        } catch (RemoteException e) {
            throw e.rethrowFromSystemServer();
        }

        // Update metadata for non-existing broadcast
        BluetoothLeAudioContentMetadata.Builder meta_builder =
                new BluetoothLeAudioContentMetadata.Builder();
        meta_builder.setLanguage("eng");
        meta_builder.setProgramInfo("Public broadcast info");
        mService.updateBroadcast(
                broadcastId, buildBroadcastSettingsFromMetadata(meta_builder.build(), null, 1));

        TestUtils.waitForLooperToFinishScheduledTask(mService.getMainLooper());

        try {
            verify(mCallbacks, times(0)).onBroadcastUpdated(anyInt(), anyInt());
            verify(mCallbacks, times(1)).onBroadcastUpdateFailed(anyInt(), anyInt());
        } catch (RemoteException e) {
            throw e.rethrowFromSystemServer();
        }
    }

    private BluetoothLeBroadcastSubgroup createBroadcastSubgroup() {
        BluetoothLeAudioCodecConfigMetadata codecMetadata =
                new BluetoothLeAudioCodecConfigMetadata.Builder()
                        .setAudioLocation(TEST_AUDIO_LOCATION_FRONT_LEFT)
                        .build();
        BluetoothLeAudioContentMetadata contentMetadata =
                new BluetoothLeAudioContentMetadata.Builder()
                        .setProgramInfo(TEST_PROGRAM_INFO)
                        .setLanguage(TEST_LANGUAGE)
                        .build();
        BluetoothLeBroadcastSubgroup.Builder builder =
                new BluetoothLeBroadcastSubgroup.Builder()
                        .setCodecId(TEST_CODEC_ID)
                        .setCodecSpecificConfig(codecMetadata)
                        .setContentMetadata(contentMetadata);

        BluetoothLeAudioCodecConfigMetadata channelCodecMetadata =
                new BluetoothLeAudioCodecConfigMetadata.Builder()
                        .setAudioLocation(TEST_AUDIO_LOCATION_FRONT_RIGHT)
                        .build();

        // builder expect at least one channel
        BluetoothLeBroadcastChannel channel =
                new BluetoothLeBroadcastChannel.Builder()
                        .setSelected(true)
                        .setChannelIndex(TEST_CHANNEL_INDEX)
                        .setCodecMetadata(channelCodecMetadata)
                        .build();
        builder.addChannel(channel);
        return builder.build();
    }

    private BluetoothLeBroadcastMetadata createBroadcastMetadata() {
        BluetoothDevice testDevice =
                mAdapter.getRemoteLeDevice(TEST_MAC_ADDRESS, BluetoothDevice.ADDRESS_TYPE_RANDOM);

        BluetoothLeBroadcastMetadata.Builder builder =
                new BluetoothLeBroadcastMetadata.Builder()
                        .setEncrypted(false)
                        .setSourceDevice(testDevice, BluetoothDevice.ADDRESS_TYPE_RANDOM)
                        .setSourceAdvertisingSid(TEST_ADVERTISER_SID)
                        .setBroadcastId(TEST_BROADCAST_ID)
                        .setBroadcastCode(null)
                        .setPaSyncInterval(TEST_PA_SYNC_INTERVAL)
                        .setPresentationDelayMicros(TEST_PRESENTATION_DELAY_MS);
        // builder expect at least one subgroup
        builder.addSubgroup(createBroadcastSubgroup());
        return builder.build();
    }

    @Test
    public void testGetAllBroadcastMetadata() {
        int broadcastId = 243;
        byte[] code = {0x00, 0x01, 0x00, 0x02};

        BluetoothLeAudioContentMetadata.Builder meta_builder =
                new BluetoothLeAudioContentMetadata.Builder();
        meta_builder.setLanguage("ENG");
        meta_builder.setProgramInfo("Public broadcast info");
        BluetoothLeAudioContentMetadata meta = meta_builder.build();
        mService.createBroadcast(buildBroadcastSettingsFromMetadata(meta, code, 1));

        LeAudioStackEvent create_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_CREATED);
        create_event.valueInt1 = broadcastId;
        create_event.valueBool1 = true;
        mService.messageFromNative(create_event);

        TestUtils.waitForLooperToFinishScheduledTask(mService.getMainLooper());

        // Inject metadata stack event and verify if getter API works as expected
        LeAudioStackEvent state_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_METADATA_CHANGED);
        state_event.valueInt1 = broadcastId;
        state_event.broadcastMetadata = createBroadcastMetadata();
        mService.messageFromNative(state_event);

        TestUtils.waitForLooperToFinishScheduledTask(mService.getMainLooper());

        List<BluetoothLeBroadcastMetadata> meta_list = mService.getAllBroadcastMetadata();
        Assert.assertNotNull(meta_list);
        Assert.assertNotEquals(meta_list.size(), 0);
        Assert.assertEquals(meta_list.get(0), state_event.broadcastMetadata);
    }

    @Test
    public void testIsBroadcastActive() {
        int broadcastId = 243;
        byte[] code = {0x00, 0x01, 0x00, 0x02};

        BluetoothLeAudioContentMetadata.Builder meta_builder =
                new BluetoothLeAudioContentMetadata.Builder();
        meta_builder.setLanguage("ENG");
        meta_builder.setProgramInfo("Public broadcast info");
        BluetoothLeAudioContentMetadata meta = meta_builder.build();
        mService.createBroadcast(buildBroadcastSettingsFromMetadata(meta, code, 1));

        LeAudioStackEvent create_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_CREATED);
        create_event.valueInt1 = broadcastId;
        create_event.valueBool1 = true;
        mService.messageFromNative(create_event);

        // Inject metadata stack event and verify if getter API works as expected
        LeAudioStackEvent state_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_METADATA_CHANGED);
        state_event.valueInt1 = broadcastId;
        state_event.broadcastMetadata = createBroadcastMetadata();
        mService.messageFromNative(state_event);

        // Verify if broadcast is active
        Assert.assertTrue(mService.isBroadcastActive());

        mService.stopBroadcast(broadcastId);
        verify(mLeAudioBroadcasterNativeInterface).stopBroadcast(eq(broadcastId));

        state_event = new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_STATE);
        state_event.valueInt1 = broadcastId;
        state_event.valueInt2 = LeAudioStackEvent.BROADCAST_STATE_STOPPED;
        mService.messageFromNative(state_event);

        verify(mLeAudioBroadcasterNativeInterface).destroyBroadcast(eq(broadcastId));

        state_event = new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_DESTROYED);
        state_event.valueInt1 = broadcastId;
        mService.messageFromNative(state_event);

        // Verify if broadcast is not active
        Assert.assertFalse(mService.isBroadcastActive());
    }

    private void verifyConnectionStateIntent(
            int timeoutMs, BluetoothDevice device, int newState, int prevState) {
        Intent intent = TestUtils.waitForIntent(timeoutMs, mIntentQueue);
        Assert.assertNotNull(intent);
        Assert.assertEquals(
                BluetoothLeAudio.ACTION_LE_AUDIO_CONNECTION_STATE_CHANGED, intent.getAction());
        Assert.assertEquals(
                (BluetoothDevice) intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE), device);
        Assert.assertEquals(intent.getIntExtra(BluetoothProfile.EXTRA_STATE, -1), newState);
        Assert.assertEquals(
                intent.getIntExtra(BluetoothProfile.EXTRA_PREVIOUS_STATE, -1), prevState);

        if (newState == BluetoothProfile.STATE_CONNECTED) {
            // ActiveDeviceManager calls deviceConnected when connected.
            mService.deviceConnected(device);
        } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
            // ActiveDeviceManager calls deviceDisconnected when connected.
            mService.deviceDisconnected(device, false);
        }
    }

    private void initializeNative() {
        LeAudioStackEvent stackEvent =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_NATIVE_INITIALIZED);
        mService.messageFromNative(stackEvent);
        Assert.assertTrue(mService.mLeAudioNativeIsInitialized);
    }

    private void prepareConnectedUnicastDevice(int groupId, BluetoothDevice device) {
        int direction = 3;
        int snkAudioLocation = 3;
        int srcAudioLocation = 4;
        int availableContexts = 5 + BluetoothLeAudio.CONTEXT_TYPE_RINGTONE;

        /* Prepare active group to cause pending broadcast */
        doReturn(BluetoothDevice.BOND_BONDED)
                .when(mAdapterService)
                .getBondState(any(BluetoothDevice.class));
        doReturn(true).when(mLeAudioNativeInterface).connectLeAudio(any(BluetoothDevice.class));
        when(mDatabaseManager.getProfileConnectionPolicy(device, BluetoothProfile.LE_AUDIO))
                .thenReturn(BluetoothProfile.CONNECTION_POLICY_ALLOWED);
        doReturn(new ParcelUuid[] {BluetoothUuid.LE_AUDIO})
                .when(mAdapterService)
                .getRemoteUuids(any(BluetoothDevice.class));
        Assert.assertTrue(mService.connect(device));

        // Verify the connection state broadcast, and that we are in Connected state
        verifyConnectionStateIntent(
                TIMEOUT_MS,
                device,
                BluetoothProfile.STATE_CONNECTING,
                BluetoothProfile.STATE_DISCONNECTED);
        Assert.assertEquals(BluetoothProfile.STATE_CONNECTING, mService.getConnectionState(device));

        LeAudioStackEvent create_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_CONNECTION_STATE_CHANGED);
        create_event.device = device;
        create_event.valueInt1 = LeAudioStackEvent.CONNECTION_STATE_CONNECTED;
        mService.messageFromNative(create_event);

        verifyConnectionStateIntent(
                TIMEOUT_MS,
                device,
                BluetoothProfile.STATE_CONNECTED,
                BluetoothProfile.STATE_CONNECTING);
        Assert.assertEquals(BluetoothProfile.STATE_CONNECTED, mService.getConnectionState(device));

        create_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_GROUP_NODE_STATUS_CHANGED);
        create_event.device = device;
        create_event.valueInt1 = groupId;
        create_event.valueInt2 = LeAudioStackEvent.GROUP_NODE_ADDED;
        mService.messageFromNative(create_event);

        create_event = new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_AUDIO_CONF_CHANGED);
        create_event.device = device;
        create_event.valueInt1 = direction;
        create_event.valueInt2 = groupId;
        create_event.valueInt3 = snkAudioLocation;
        create_event.valueInt4 = srcAudioLocation;
        create_event.valueInt5 = availableContexts;
        mService.messageFromNative(create_event);

        // Set default codec config to HIGH quality
        injectGroupSelectableCodecConfigChanged(
                groupId, INPUT_SELECTABLE_CONFIG_HIGH, OUTPUT_SELECTABLE_CONFIG_HIGH);
        injectGroupCurrentCodecConfigChanged(groupId, LC3_16KHZ_CONFIG, LC3_48KHZ_CONFIG);
    }

    @Test
    public void testCreatePendingBroadcast() {
        int groupId = 1;
        int broadcastId = 243;
        byte[] code = {0x00, 0x01, 0x00, 0x02};

        mSetFlagsRule.enableFlags(Flags.FLAG_LEAUDIO_BIG_DEPENDS_ON_AUDIO_STATE);

        initializeNative();
        prepareConnectedUnicastDevice(groupId, mDevice);

        LeAudioStackEvent stackEvent =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_GROUP_STATUS_CHANGED);
        stackEvent.valueInt1 = groupId;
        stackEvent.valueInt2 = LeAudioStackEvent.GROUP_STATUS_ACTIVE;
        mService.messageFromNative(stackEvent);

        /* Prepare create broadcast */
        BluetoothLeAudioContentMetadata.Builder meta_builder =
                new BluetoothLeAudioContentMetadata.Builder();
        meta_builder.setLanguage("ENG");
        meta_builder.setProgramInfo("Public broadcast info");
        BluetoothLeAudioContentMetadata meta = meta_builder.build();

        BluetoothLeBroadcastSettings settings = buildBroadcastSettingsFromMetadata(meta, code, 1);
        mService.createBroadcast(settings);

        /* Successfully created audio session notification */
        stackEvent =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_AUDIO_SESSION_CREATED);
        stackEvent.valueBool1 = true;
        mService.messageFromNative(stackEvent);

        /* Check if broadcast is started automatically when created */
        stackEvent = new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_CREATED);
        stackEvent.valueInt1 = broadcastId;
        stackEvent.valueBool1 = true;
        mService.messageFromNative(stackEvent);

        /* Active group should become inactive */
        int activeGroup = mService.getActiveGroupId();
        Assert.assertEquals(activeGroup, LE_AUDIO_GROUP_ID_INVALID);

        /* Imitate group inactivity to cause create broadcast */
        stackEvent = new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_GROUP_STATUS_CHANGED);
        stackEvent.valueInt1 = groupId;
        stackEvent.valueInt2 = LeAudioStackEvent.GROUP_STATUS_INACTIVE;
        mService.messageFromNative(stackEvent);

        List<BluetoothLeBroadcastSubgroupSettings> settingsList = settings.getSubgroupSettings();

        int[] expectedQualityArray =
                settingsList.stream().mapToInt(setting -> setting.getPreferredQuality()).toArray();
        byte[][] expectedDataArray =
                settingsList.stream()
                        .map(setting -> setting.getContentMetadata().getRawMetadata())
                        .toArray(byte[][]::new);

        verify(mLeAudioBroadcasterNativeInterface)
                .createBroadcast(
                        eq(true),
                        eq(TEST_BROADCAST_NAME),
                        eq(settings.getBroadcastCode()),
                        eq(settings.getPublicBroadcastMetadata().getRawMetadata()),
                        eq(expectedQualityArray),
                        eq(expectedDataArray));

        activeGroup = mService.getActiveGroupId();
        Assert.assertEquals(-1, activeGroup);
    }

    @Test
    public void testCreateBroadcastMoreThanMaxFailed() {
        int broadcastId = 243;
        byte[] code = {0x00, 0x01, 0x00, 0x02};

        synchronized (mService.mBroadcastCallbacks) {
            mService.mBroadcastCallbacks.register(mCallbacks);
        }

        BluetoothLeAudioContentMetadata.Builder meta_builder =
                new BluetoothLeAudioContentMetadata.Builder();
        meta_builder.setLanguage("deu");
        meta_builder.setProgramInfo("Subgroup broadcast info");
        BluetoothLeAudioContentMetadata meta = meta_builder.build();
        BluetoothLeBroadcastSettings settings = buildBroadcastSettingsFromMetadata(meta, code, 1);

        TestUtils.waitForLooperToFinishScheduledTask(mService.getMainLooper());

        verifyBroadcastStarted(broadcastId, settings);
        Mockito.clearInvocations(mCallbacks);

        // verify creating another broadcast will fail
        mService.createBroadcast(settings);

        TestUtils.waitForLooperToFinishScheduledTask(mService.getMainLooper());

        try {
            verify(mCallbacks, times(0)).onBroadcastStarted(anyInt(), anyInt());
            verify(mCallbacks, times(1))
                    .onBroadcastStartFailed(
                            eq(BluetoothStatusCodes.ERROR_LOCAL_NOT_ENOUGH_RESOURCES));
        } catch (RemoteException e) {
            throw e.rethrowFromSystemServer();
        }
    }

    private void prepareHandoverStreamingBroadcast(int groupId, int broadcastId, byte[] code) {
        mSetFlagsRule.enableFlags(Flags.FLAG_LEAUDIO_BIG_DEPENDS_ON_AUDIO_STATE);

        synchronized (mService.mBroadcastCallbacks) {
            mService.mBroadcastCallbacks.register(mCallbacks);
        }

        initializeNative();
        prepareConnectedUnicastDevice(groupId, mDevice);

        LeAudioStackEvent create_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_GROUP_STATUS_CHANGED);
        create_event.valueInt1 = groupId;
        create_event.valueInt2 = LeAudioStackEvent.GROUP_STATUS_ACTIVE;
        mService.messageFromNative(create_event);

        verify(mTbsService).setInbandRingtoneSupport(eq(mDevice));

        /* Verify Unicast input and output devices changed from null to mDevice */
        verify(mAudioManager, times(2))
                .handleBluetoothActiveDeviceChanged(
                        eq(mDevice), eq(null), any(BluetoothProfileConnectionInfo.class));
        Mockito.clearInvocations(mAudioManager);

        mService.notifyActiveDeviceChanged(mDevice);

        /* Prepare create broadcast */
        BluetoothLeAudioContentMetadata.Builder meta_builder =
                new BluetoothLeAudioContentMetadata.Builder();
        meta_builder.setLanguage("ENG");
        meta_builder.setProgramInfo("Public broadcast info");
        BluetoothLeAudioContentMetadata meta = meta_builder.build();

        BluetoothLeBroadcastSettings settings = buildBroadcastSettingsFromMetadata(meta, code, 1);
        mService.createBroadcast(settings);

        /* Successfully created audio session notification */
        create_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_AUDIO_SESSION_CREATED);
        create_event.valueBool1 = true;
        mService.messageFromNative(create_event);

        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(mBroadcastDevice), eq(null), any(BluetoothProfileConnectionInfo.class));

        /* Active group should become inactive */
        int activeGroup = mService.getActiveGroupId();
        Assert.assertEquals(activeGroup, LE_AUDIO_GROUP_ID_INVALID);

        /* Imitate group inactivity to cause create broadcast */
        create_event = new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_GROUP_STATUS_CHANGED);
        create_event.valueInt1 = groupId;
        create_event.valueInt2 = LeAudioStackEvent.GROUP_STATUS_INACTIVE;
        mService.messageFromNative(create_event);

        /* Only one Unicast device should become inactive due to Sink monitor mode */
        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(null), eq(mDevice), any(BluetoothProfileConnectionInfo.class));
        Mockito.clearInvocations(mAudioManager);
        List<BluetoothLeBroadcastSubgroupSettings> settingsList = settings.getSubgroupSettings();

        int[] expectedQualityArray =
                settingsList.stream().mapToInt(setting -> setting.getPreferredQuality()).toArray();
        byte[][] expectedDataArray =
                settingsList.stream()
                        .map(setting -> setting.getContentMetadata().getRawMetadata())
                        .toArray(byte[][]::new);

        verify(mLeAudioBroadcasterNativeInterface)
                .createBroadcast(
                        eq(true),
                        eq(TEST_BROADCAST_NAME),
                        eq(settings.getBroadcastCode()),
                        eq(settings.getPublicBroadcastMetadata().getRawMetadata()),
                        eq(expectedQualityArray),
                        eq(expectedDataArray));
        verify(mLeAudioNativeInterface)
                .setUnicastMonitorMode(eq(LeAudioStackEvent.DIRECTION_SINK), eq(true));

        activeGroup = mService.getActiveGroupId();
        Assert.assertEquals(LE_AUDIO_GROUP_ID_INVALID, activeGroup);

        /* Check if broadcast is started automatically when created */
        create_event = new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_CREATED);
        create_event.valueInt1 = broadcastId;
        create_event.valueBool1 = true;
        mService.messageFromNative(create_event);

        /* Switch to active streaming */
        create_event = new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_STATE);
        create_event.device = mBroadcastDevice;
        create_event.valueInt1 = broadcastId;
        create_event.valueInt2 = LeAudioStackEvent.BROADCAST_STATE_STREAMING;
        mService.messageFromNative(create_event);

        verify(mTbsService, times(0)).clearInbandRingtoneSupport(eq(mDevice));
    }

    @Test
    public void testInCallDrivenBroadcastSwitch() {
        mSetFlagsRule.disableFlags(Flags.FLAG_LEAUDIO_USE_AUDIO_MODE_LISTENER);
        int groupId = 1;
        int broadcastId = 243;
        byte[] code = {0x00, 0x01, 0x00, 0x02};

        prepareHandoverStreamingBroadcast(groupId, broadcastId, code);

        /* Imitate setting device in call */
        mService.setInCall(true);

        Assert.assertTrue(mService.mBroadcastIdDeactivatedForUnicastTransition.isPresent());

        /* Check if broadcast is paused by InCall handling */
        verify(mLeAudioBroadcasterNativeInterface).pauseBroadcast(eq(broadcastId));

        LeAudioStackEvent state_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_STATE);
        state_event.valueInt1 = broadcastId;
        state_event.valueInt2 = LeAudioStackEvent.BROADCAST_STATE_PAUSED;
        mService.messageFromNative(state_event);

        verify(mLeAudioNativeInterface).setInCall(eq(true));

        LeAudioStackEvent create_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_GROUP_STATUS_CHANGED);
        create_event.valueInt1 = groupId;
        create_event.valueInt2 = LeAudioStackEvent.GROUP_STATUS_ACTIVE;
        mService.messageFromNative(create_event);

        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(mDevice), eq(null), any(BluetoothProfileConnectionInfo.class));
        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(null), eq(mBroadcastDevice), any(BluetoothProfileConnectionInfo.class));

        /* Active group should become the one that was active before broadcasting */
        int activeGroup = mService.getActiveGroupId();
        Assert.assertEquals(activeGroup, groupId);

        /* Imitate setting device not in call */
        mService.setInCall(false);

        verify(mLeAudioNativeInterface, times(2)).groupSetActive(eq(LE_AUDIO_GROUP_ID_INVALID));

        /* Imitate group inactivity to cause start broadcast */
        create_event = new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_GROUP_STATUS_CHANGED);
        create_event.valueInt1 = groupId;
        create_event.valueInt2 = LeAudioStackEvent.GROUP_STATUS_INACTIVE;
        mService.messageFromNative(create_event);

        /* Only one Unicast device should become inactive due to Sink monitor mode */
        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(null), eq(mDevice), any(BluetoothProfileConnectionInfo.class));
        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(mBroadcastDevice), eq(null), any(BluetoothProfileConnectionInfo.class));

        /* Verify if broadcast triggers transition */
        Assert.assertFalse(mService.mBroadcastIdDeactivatedForUnicastTransition.isPresent());
    }

    @Test
    public void testAudioModeDrivenBroadcastSwitch() {
        mSetFlagsRule.enableFlags(Flags.FLAG_LEAUDIO_USE_AUDIO_MODE_LISTENER);

        int groupId = 1;
        int broadcastId = 243;
        byte[] code = {0x00, 0x01, 0x00, 0x02};

        prepareHandoverStreamingBroadcast(groupId, broadcastId, code);

        /* Imitate setting device in call */
        mService.handleAudioModeChange(AudioManager.MODE_IN_CALL);

        Assert.assertTrue(mService.mBroadcastIdDeactivatedForUnicastTransition.isPresent());

        /* Check if broadcast is paused by AudioMode handling */
        verify(mLeAudioBroadcasterNativeInterface).pauseBroadcast(eq(broadcastId));

        LeAudioStackEvent state_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_STATE);
        state_event.valueInt1 = broadcastId;
        state_event.valueInt2 = LeAudioStackEvent.BROADCAST_STATE_PAUSED;
        mService.messageFromNative(state_event);

        LeAudioStackEvent create_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_GROUP_STATUS_CHANGED);
        create_event.valueInt1 = groupId;
        create_event.valueInt2 = LeAudioStackEvent.GROUP_STATUS_ACTIVE;
        mService.messageFromNative(create_event);

        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(mDevice), eq(null), any(BluetoothProfileConnectionInfo.class));
        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(null), eq(mBroadcastDevice), any(BluetoothProfileConnectionInfo.class));

        /* Active group should become the one that was active before broadcasting */
        int activeGroup = mService.getActiveGroupId();
        Assert.assertEquals(activeGroup, groupId);

        /* Imitate group change request by Bluetooth Sink HAL suspend request */
        create_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_UNICAST_MONITOR_MODE_STATUS);
        create_event.valueInt1 = LeAudioStackEvent.DIRECTION_SINK;
        create_event.valueInt2 = LeAudioStackEvent.STATUS_LOCAL_STREAM_SUSPENDED;
        mService.messageFromNative(create_event);

        /* Device should not be inactivated if in IN_CALL audio mode */
        verify(mLeAudioNativeInterface).groupSetActive(eq(LE_AUDIO_GROUP_ID_INVALID));

        /* Imitate setting device not in call */
        mService.handleAudioModeChange(AudioManager.MODE_NORMAL);

        verify(mLeAudioNativeInterface, times(2)).groupSetActive(eq(LE_AUDIO_GROUP_ID_INVALID));

        /* Imitate group inactivity to cause start broadcast */
        create_event = new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_GROUP_STATUS_CHANGED);
        create_event.valueInt1 = groupId;
        create_event.valueInt2 = LeAudioStackEvent.GROUP_STATUS_INACTIVE;
        mService.messageFromNative(create_event);

        /* Only one Unicast device should become inactive due to Sink monitor mode */
        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(null), eq(mDevice), any(BluetoothProfileConnectionInfo.class));
        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(mBroadcastDevice), eq(null), any(BluetoothProfileConnectionInfo.class));

        if (Flags.leaudioBigDependsOnAudioState()) {
            /* Verify if broadcast triggers transition */
            Assert.assertFalse(mService.mBroadcastIdDeactivatedForUnicastTransition.isPresent());
        }

        /* Verify if broadcast is auto-started on start */
        verify(mLeAudioBroadcasterNativeInterface, times(2)).startBroadcast(eq(broadcastId));
    }

    @Test
    public void testBroadcastResumeUnicastGroupChangeRequestDriven() {
        int groupId = 1;
        int broadcastId = 243;
        byte[] code = {0x00, 0x01, 0x00, 0x02};

        prepareHandoverStreamingBroadcast(groupId, broadcastId, code);

        verify(mLeAudioBroadcasterNativeInterface).startBroadcast(eq(broadcastId));

        /* Imitate group change request by Bluetooth Sink HAL resume request */
        LeAudioStackEvent create_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_UNICAST_MONITOR_MODE_STATUS);
        create_event.valueInt1 = LeAudioStackEvent.DIRECTION_SINK;
        create_event.valueInt2 = LeAudioStackEvent.STATUS_LOCAL_STREAM_REQUESTED;
        mService.messageFromNative(create_event);

        Assert.assertTrue(mService.mBroadcastIdDeactivatedForUnicastTransition.isPresent());

        /* Check if broadcast is paused triggered by group change request */
        verify(mLeAudioBroadcasterNativeInterface).pauseBroadcast(eq(broadcastId));

        LeAudioStackEvent state_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_STATE);
        state_event.valueInt1 = broadcastId;
        state_event.valueInt2 = LeAudioStackEvent.BROADCAST_STATE_PAUSED;
        mService.messageFromNative(state_event);

        create_event = new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_GROUP_STATUS_CHANGED);
        create_event.valueInt1 = groupId;
        create_event.valueInt2 = LeAudioStackEvent.GROUP_STATUS_ACTIVE;
        mService.messageFromNative(create_event);

        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(mDevice), eq(null), any(BluetoothProfileConnectionInfo.class));
        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(null), eq(mBroadcastDevice), any(BluetoothProfileConnectionInfo.class));

        /* Active group should become the one that was active before broadcasting */
        int activeGroup = mService.getActiveGroupId();
        Assert.assertEquals(activeGroup, groupId);

        /* Imitate group change request by Bluetooth Sink HAL suspend request */
        create_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_UNICAST_MONITOR_MODE_STATUS);
        create_event.valueInt1 = LeAudioStackEvent.DIRECTION_SINK;
        create_event.valueInt2 = LeAudioStackEvent.STATUS_LOCAL_STREAM_SUSPENDED;
        mService.messageFromNative(create_event);

        verify(mLeAudioNativeInterface, times(2)).groupSetActive(eq(LE_AUDIO_GROUP_ID_INVALID));

        /* Imitate group inactivity to cause start broadcast */
        create_event = new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_GROUP_STATUS_CHANGED);
        create_event.valueInt1 = groupId;
        create_event.valueInt2 = LeAudioStackEvent.GROUP_STATUS_INACTIVE;
        mService.messageFromNative(create_event);

        /* Only one Unicast device should become inactive due to Sink monitor mode */
        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(null), eq(mDevice), any(BluetoothProfileConnectionInfo.class));
        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(mBroadcastDevice), eq(null), any(BluetoothProfileConnectionInfo.class));

        if (Flags.leaudioBigDependsOnAudioState()) {
            /* Verify if broadcast triggers transition */
            Assert.assertFalse(mService.mBroadcastIdDeactivatedForUnicastTransition.isPresent());
        }

        verify(mLeAudioBroadcasterNativeInterface, times(2)).startBroadcast(eq(broadcastId));
    }

    @Test
    public void testInCallDrivenBroadcastSwitchDuringInternalPause() {
        mSetFlagsRule.disableFlags(Flags.FLAG_LEAUDIO_USE_AUDIO_MODE_LISTENER);
        mSetFlagsRule.enableFlags(Flags.FLAG_LEAUDIO_BIG_DEPENDS_ON_AUDIO_STATE);
        int groupId = 1;
        int broadcastId = 243;
        byte[] code = {0x00, 0x01, 0x00, 0x02};

        prepareHandoverStreamingBroadcast(groupId, broadcastId, code);

        /* Internal broadcast paused due to onAudioSuspend */
        LeAudioStackEvent state_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_STATE);
        state_event.valueInt1 = broadcastId;
        state_event.valueInt2 = LeAudioStackEvent.BROADCAST_STATE_PAUSED;
        mService.messageFromNative(state_event);

        /* Imitate setting device in call */
        mService.setInCall(true);

        Assert.assertTrue(mService.mBroadcastIdDeactivatedForUnicastTransition.isPresent());

        /* Broadcast already paused, not call pause again by InCall handling */
        verify(mLeAudioBroadcasterNativeInterface, never()).pauseBroadcast(eq(broadcastId));

        verify(mLeAudioNativeInterface).setInCall(eq(true));

        LeAudioStackEvent create_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_GROUP_STATUS_CHANGED);
        create_event.valueInt1 = groupId;
        create_event.valueInt2 = LeAudioStackEvent.GROUP_STATUS_ACTIVE;
        mService.messageFromNative(create_event);

        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(mDevice), eq(null), any(BluetoothProfileConnectionInfo.class));
        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(null), eq(mBroadcastDevice), any(BluetoothProfileConnectionInfo.class));

        /* Active group should become the one that was active before broadcasting */
        int activeGroup = mService.getActiveGroupId();
        Assert.assertEquals(activeGroup, groupId);

        /* Imitate setting device not in call */
        mService.setInCall(false);

        verify(mLeAudioNativeInterface, times(2)).groupSetActive(eq(LE_AUDIO_GROUP_ID_INVALID));

        /* Imitate group inactivity to cause start broadcast */
        create_event = new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_GROUP_STATUS_CHANGED);
        create_event.valueInt1 = groupId;
        create_event.valueInt2 = LeAudioStackEvent.GROUP_STATUS_INACTIVE;
        mService.messageFromNative(create_event);

        /* Only one Unicast device should become inactive due to Sink monitor mode */
        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(null), eq(mDevice), any(BluetoothProfileConnectionInfo.class));
        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(mBroadcastDevice), eq(null), any(BluetoothProfileConnectionInfo.class));

        /* Verify if broadcast triggers transition */
        Assert.assertFalse(mService.mBroadcastIdDeactivatedForUnicastTransition.isPresent());
    }

    @Test
    public void testAudioModeDrivenBroadcastSwitchDuringInternalPause() {
        mSetFlagsRule.enableFlags(Flags.FLAG_LEAUDIO_USE_AUDIO_MODE_LISTENER);
        mSetFlagsRule.enableFlags(Flags.FLAG_LEAUDIO_BIG_DEPENDS_ON_AUDIO_STATE);
        int groupId = 1;
        int broadcastId = 243;
        byte[] code = {0x00, 0x01, 0x00, 0x02};

        prepareHandoverStreamingBroadcast(groupId, broadcastId, code);

        /* Internal broadcast paused due to onAudioSuspend */
        LeAudioStackEvent state_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_STATE);
        state_event.valueInt1 = broadcastId;
        state_event.valueInt2 = LeAudioStackEvent.BROADCAST_STATE_PAUSED;
        mService.messageFromNative(state_event);

        /* Imitate setting device in call */
        mService.handleAudioModeChange(AudioManager.MODE_IN_CALL);

        Assert.assertTrue(mService.mBroadcastIdDeactivatedForUnicastTransition.isPresent());

        /* Broadcast already paused, not call pause again by AudioMode handling */
        verify(mLeAudioBroadcasterNativeInterface, never()).pauseBroadcast(eq(broadcastId));

        LeAudioStackEvent create_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_GROUP_STATUS_CHANGED);
        create_event.valueInt1 = groupId;
        create_event.valueInt2 = LeAudioStackEvent.GROUP_STATUS_ACTIVE;
        mService.messageFromNative(create_event);

        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(mDevice), eq(null), any(BluetoothProfileConnectionInfo.class));
        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(null), eq(mBroadcastDevice), any(BluetoothProfileConnectionInfo.class));

        /* Active group should become the one that was active before broadcasting */
        int activeGroup = mService.getActiveGroupId();
        Assert.assertEquals(activeGroup, groupId);

        /* Imitate group change request by Bluetooth Sink HAL suspend request */
        create_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_UNICAST_MONITOR_MODE_STATUS);
        create_event.valueInt1 = LeAudioStackEvent.DIRECTION_SINK;
        create_event.valueInt2 = LeAudioStackEvent.STATUS_LOCAL_STREAM_SUSPENDED;
        mService.messageFromNative(create_event);

        /* Device should not be inactivated if in IN_CALL audio mode */
        verify(mLeAudioNativeInterface).groupSetActive(eq(LE_AUDIO_GROUP_ID_INVALID));

        /* Imitate setting device not in call */
        mService.handleAudioModeChange(AudioManager.MODE_NORMAL);

        verify(mLeAudioNativeInterface, times(2)).groupSetActive(eq(LE_AUDIO_GROUP_ID_INVALID));

        /* Imitate group inactivity to cause start broadcast */
        create_event = new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_GROUP_STATUS_CHANGED);
        create_event.valueInt1 = groupId;
        create_event.valueInt2 = LeAudioStackEvent.GROUP_STATUS_INACTIVE;
        mService.messageFromNative(create_event);

        /* Only one Unicast device should become inactive due to Sink monitor mode */
        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(null), eq(mDevice), any(BluetoothProfileConnectionInfo.class));
        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(mBroadcastDevice), eq(null), any(BluetoothProfileConnectionInfo.class));

        /* Verify if broadcast triggers transition */
        Assert.assertFalse(mService.mBroadcastIdDeactivatedForUnicastTransition.isPresent());
    }

    @Test
    public void testBroadcastResumeUnicastGroupChangeRequestDrivenDuringInternalPause() {
        mSetFlagsRule.enableFlags(Flags.FLAG_LEAUDIO_BIG_DEPENDS_ON_AUDIO_STATE);
        int groupId = 1;
        int broadcastId = 243;
        byte[] code = {0x00, 0x01, 0x00, 0x02};

        prepareHandoverStreamingBroadcast(groupId, broadcastId, code);

        /* Internal broadcast paused due to onAudioSuspend */
        LeAudioStackEvent state_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_STATE);
        state_event.valueInt1 = broadcastId;
        state_event.valueInt2 = LeAudioStackEvent.BROADCAST_STATE_PAUSED;
        mService.messageFromNative(state_event);

        /* Imitate group change request by Bluetooth Sink HAL resume request */
        LeAudioStackEvent create_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_UNICAST_MONITOR_MODE_STATUS);
        create_event.valueInt1 = LeAudioStackEvent.DIRECTION_SINK;
        create_event.valueInt2 = LeAudioStackEvent.STATUS_LOCAL_STREAM_REQUESTED;
        mService.messageFromNative(create_event);

        Assert.assertTrue(mService.mBroadcastIdDeactivatedForUnicastTransition.isPresent());

        /* Broadcast already paused, not call pause again by group change request */
        verify(mLeAudioBroadcasterNativeInterface, never()).pauseBroadcast(eq(broadcastId));

        create_event = new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_GROUP_STATUS_CHANGED);
        create_event.valueInt1 = groupId;
        create_event.valueInt2 = LeAudioStackEvent.GROUP_STATUS_ACTIVE;
        mService.messageFromNative(create_event);

        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(mDevice), eq(null), any(BluetoothProfileConnectionInfo.class));
        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(null), eq(mBroadcastDevice), any(BluetoothProfileConnectionInfo.class));

        /* Active group should become the one that was active before broadcasting */
        int activeGroup = mService.getActiveGroupId();
        Assert.assertEquals(activeGroup, groupId);

        /* Imitate group change request by Bluetooth Sink HAL suspend request */
        create_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_UNICAST_MONITOR_MODE_STATUS);
        create_event.valueInt1 = LeAudioStackEvent.DIRECTION_SINK;
        create_event.valueInt2 = LeAudioStackEvent.STATUS_LOCAL_STREAM_SUSPENDED;
        mService.messageFromNative(create_event);

        verify(mLeAudioNativeInterface, times(2)).groupSetActive(eq(LE_AUDIO_GROUP_ID_INVALID));

        /* Imitate group inactivity to cause start broadcast */
        create_event = new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_GROUP_STATUS_CHANGED);
        create_event.valueInt1 = groupId;
        create_event.valueInt2 = LeAudioStackEvent.GROUP_STATUS_INACTIVE;
        mService.messageFromNative(create_event);

        /* Only one Unicast device should become inactive due to Sink monitor mode */
        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(null), eq(mDevice), any(BluetoothProfileConnectionInfo.class));
        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(mBroadcastDevice), eq(null), any(BluetoothProfileConnectionInfo.class));
        /* Verify if broadcast triggers transition */
        Assert.assertFalse(mService.mBroadcastIdDeactivatedForUnicastTransition.isPresent());
    }

    @Test
    public void testCacheAndResmueSuspendingSources() {
        mSetFlagsRule.enableFlags(Flags.FLAG_LEAUDIO_BIG_DEPENDS_ON_AUDIO_STATE);
        int groupId = 1;
        int broadcastId = 243;
        byte[] code = {0x00, 0x01, 0x00, 0x02};

        prepareHandoverStreamingBroadcast(groupId, broadcastId, code);

        /* Internal broadcast paused due to onAudioSuspend */
        LeAudioStackEvent state_event =
                new LeAudioStackEvent(LeAudioStackEvent.EVENT_TYPE_BROADCAST_STATE);
        state_event.valueInt1 = broadcastId;
        state_event.valueInt2 = LeAudioStackEvent.BROADCAST_STATE_PAUSED;
        mService.messageFromNative(state_event);

        if (!Flags.leaudioBroadcastAssistantPeripheralEntrustment()) {
            verify(mBassClientService).suspendReceiversSourceSynchronization(eq(broadcastId));
        } else {
            verify(mBassClientService).cacheSuspendingSources(eq(broadcastId));
        }

        /* Internal broadcast resumed due to onAudioResumed */
        state_event.valueInt2 = LeAudioStackEvent.BROADCAST_STATE_STREAMING;
        mService.messageFromNative(state_event);

        verify(mBassClientService).resumeReceiversSourceSynchronization();
    }

    @Test
    @DisableFlags(Flags.FLAG_LEAUDIO_BROADCAST_PRIMARY_GROUP_SELECTION)
    public void testUpdateFallbackInputDevice() {
        mSetFlagsRule.disableFlags(Flags.FLAG_LEAUDIO_USE_AUDIO_MODE_LISTENER);
        int groupId = 1;
        int groupId2 = 2;
        int broadcastId = 243;
        byte[] code = {0x00, 0x01, 0x00, 0x02};

        initializeNative();
        prepareConnectedUnicastDevice(groupId2, mDevice2);
        prepareHandoverStreamingBroadcast(groupId, broadcastId, code);

        Assert.assertEquals(mService.mUnicastGroupIdDeactivatedForBroadcastTransition, groupId);

        reset(mAudioManager);

        /* Update fallback active device (only input is active) */
        ArgumentCaptor<BluetoothProfileConnectionInfo> connectionInfoArgumentCaptor =
                ArgumentCaptor.forClass(BluetoothProfileConnectionInfo.class);

        Assert.assertTrue(mService.setActiveDevice(mDevice2));

        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(mDevice2), eq(mDevice), connectionInfoArgumentCaptor.capture());
        List<BluetoothProfileConnectionInfo> connInfos =
                connectionInfoArgumentCaptor.getAllValues();
        Assert.assertEquals(connInfos.size(), 1);
        Assert.assertFalse(connInfos.get(0).isLeOutput());
        Assert.assertEquals(mService.mUnicastGroupIdDeactivatedForBroadcastTransition, groupId2);
    }

    @Test
    @EnableFlags(Flags.FLAG_LEAUDIO_BROADCAST_API_MANAGE_PRIMARY_GROUP)
    public void testOnBroadcastToUnicastFallbackGroupChanged() {
        int groupId1 = 1;
        int groupId2 = 2;
        int broadcastId = 243;
        byte[] code = {0x00, 0x01, 0x00, 0x02};

        onBroadcastToUnicastFallbackGroupChangedCallbackCalled = false;

        IBluetoothLeAudioCallback leAudioCallbacks =
                new IBluetoothLeAudioCallback.Stub() {
                    @Override
                    public void onCodecConfigChanged(int gid, BluetoothLeAudioCodecStatus status) {}

                    @Override
                    public void onGroupStatusChanged(int gid, int gStatus) {}

                    @Override
                    public void onGroupNodeAdded(BluetoothDevice device, int gid) {}

                    @Override
                    public void onGroupNodeRemoved(BluetoothDevice device, int gid) {}

                    @Override
                    public void onGroupStreamStatusChanged(int groupId, int groupStreamStatus) {}

                    @Override
                    public void onBroadcastToUnicastFallbackGroupChanged(int groupId) {
                        onBroadcastToUnicastFallbackGroupChangedCallbackCalled = true;
                        Assert.assertEquals(groupId1, groupId);
                    }
                };

        synchronized (mService.mLeAudioCallbacks) {
            mService.mLeAudioCallbacks.register(leAudioCallbacks);
        }

        initializeNative();
        prepareConnectedUnicastDevice(groupId2, mDevice2);
        prepareHandoverStreamingBroadcast(groupId1, broadcastId, code);

        TestUtils.waitForLooperToFinishScheduledTask(mService.getMainLooper());
        Assert.assertEquals(groupId1, mService.mUnicastGroupIdDeactivatedForBroadcastTransition);
        Assert.assertTrue(onBroadcastToUnicastFallbackGroupChangedCallbackCalled);

        onBroadcastToUnicastFallbackGroupChangedCallbackCalled = false;
        synchronized (mService.mLeAudioCallbacks) {
            mService.mLeAudioCallbacks.unregister(leAudioCallbacks);
        }
    }

    private class LeAudioIntentReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            try {
                mIntentQueue.put(intent);
            } catch (InterruptedException e) {
                Assert.fail("Cannot add Intent to the queue: " + e.getMessage());
            }
        }
    }

    @Test
    @EnableFlags({
        Flags.FLAG_LEAUDIO_BROADCAST_PRIMARY_GROUP_SELECTION,
        Flags.FLAG_LEAUDIO_BROADCAST_API_MANAGE_PRIMARY_GROUP,
        Flags.FLAG_LEAUDIO_USE_AUDIO_MODE_LISTENER
    })
    public void testManageBroadcastToUnicastFallbackGroup() {
        int groupId = 1;
        int groupId2 = 2;
        int broadcastId = 243;
        byte[] code = {0x00, 0x01, 0x00, 0x02};
        List<BluetoothDevice> devices = new ArrayList<>();

        when(mDatabaseManager.getMostRecentlyConnectedDevices()).thenReturn(devices);

        initializeNative();
        devices.add(mDevice);
        prepareHandoverStreamingBroadcast(groupId, broadcastId, code);
        mService.deviceConnected(mDevice);
        devices.add(mDevice2);
        prepareConnectedUnicastDevice(groupId2, mDevice2);
        mService.deviceConnected(mDevice2);

        Assert.assertEquals(mService.mUnicastGroupIdDeactivatedForBroadcastTransition, groupId);

        reset(mAudioManager);

        /* Update fallback active device (only input is active) */
        ArgumentCaptor<BluetoothProfileConnectionInfo> connectionInfoArgumentCaptor =
                ArgumentCaptor.forClass(BluetoothProfileConnectionInfo.class);

        mService.setBroadcastToUnicastFallbackGroup(groupId2);

        verify(mAudioManager)
                .handleBluetoothActiveDeviceChanged(
                        eq(mDevice2), eq(mDevice), connectionInfoArgumentCaptor.capture());
        List<BluetoothProfileConnectionInfo> connInfos =
                connectionInfoArgumentCaptor.getAllValues();
        Assert.assertEquals(connInfos.size(), 1);
        Assert.assertFalse(connInfos.get(0).isLeOutput());
        Assert.assertEquals(mService.getBroadcastToUnicastFallbackGroup(), groupId2);
    }

    private BluetoothLeBroadcastSettings buildBroadcastSettingsFromMetadata(
            BluetoothLeAudioContentMetadata contentMetadata,
            @Nullable byte[] broadcastCode,
            int numOfGroups) {
        BluetoothLeAudioContentMetadata.Builder publicMetaBuilder =
                new BluetoothLeAudioContentMetadata.Builder();
        publicMetaBuilder.setProgramInfo("Public broadcast info");

        BluetoothLeBroadcastSubgroupSettings.Builder subgroupBuilder =
                new BluetoothLeBroadcastSubgroupSettings.Builder()
                        .setContentMetadata(contentMetadata)
                        .setPreferredQuality(BluetoothLeBroadcastSubgroupSettings.QUALITY_HIGH);

        BluetoothLeBroadcastSettings.Builder builder =
                new BluetoothLeBroadcastSettings.Builder()
                        .setPublicBroadcast(true)
                        .setBroadcastName(TEST_BROADCAST_NAME)
                        .setBroadcastCode(broadcastCode)
                        .setPublicBroadcastMetadata(publicMetaBuilder.build());
        // builder expect at least one subgroup setting
        for (int i = 0; i < numOfGroups; i++) {
            // add subgroup settings with the same content
            builder.addSubgroupSettings(subgroupBuilder.build());
        }
        return builder.build();
    }

    private void injectGroupCurrentCodecConfigChanged(
            int groupId,
            BluetoothLeAudioCodecConfig inputCodecConfig,
            BluetoothLeAudioCodecConfig outputCodecConfig) {
        int eventType = LeAudioStackEvent.EVENT_TYPE_AUDIO_GROUP_CURRENT_CODEC_CONFIG_CHANGED;

        LeAudioStackEvent groupCodecConfigChangedEvent = new LeAudioStackEvent(eventType);
        groupCodecConfigChangedEvent.valueInt1 = groupId;
        groupCodecConfigChangedEvent.valueCodec1 = inputCodecConfig;
        groupCodecConfigChangedEvent.valueCodec2 = outputCodecConfig;
        mService.messageFromNative(groupCodecConfigChangedEvent);
    }

    private void injectGroupSelectableCodecConfigChanged(
            int groupId,
            List<BluetoothLeAudioCodecConfig> inputSelectableCodecConfig,
            List<BluetoothLeAudioCodecConfig> outputSelectableCodecConfig) {
        int eventType = LeAudioStackEvent.EVENT_TYPE_AUDIO_GROUP_SELECTABLE_CODEC_CONFIG_CHANGED;

        LeAudioStackEvent groupCodecConfigChangedEvent = new LeAudioStackEvent(eventType);
        groupCodecConfigChangedEvent.valueInt1 = groupId;
        groupCodecConfigChangedEvent.valueCodecList1 = inputSelectableCodecConfig;
        groupCodecConfigChangedEvent.valueCodecList2 = outputSelectableCodecConfig;
        mService.messageFromNative(groupCodecConfigChangedEvent);
    }
}
