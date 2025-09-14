# Copyright 2024 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import logging

from mobly.asserts import assert_equal

from pairing.br_edr.ssp.test_base import BREDRSSPPairTestBase

from pandora.security_pb2 import PairingEventAnswer


class BREDRDisplayOnlyTestClass(BREDRSSPPairTestBase):

    def _setup_devices(self) -> None:

        self.ref.config.setdefault('classic_enabled', True)
        self.ref.config.setdefault('le_enabled', False)
        self.ref.config.setdefault('classic_ssp_enabled', True)
        self.ref.config.setdefault('classic_sc_enabled', False)

        self.ref.config.setdefault(
            'server',
             {
                'pairing_sc_enable': False,
                'pairing_mitm_enable': False,
                'pairing_bonding_enable': True,
                # Android IO CAP: Display_YESNO
                # Ref IO CAP:
                'io_capability': 'display_output_only',
             },
        )

    async def accept_pairing(self):
        init_ev = await anext(self.initiator_pairing_event_stream)
        logging.debug(f'[{self.initiator_pairing_event_stream.device.name}] init_ev.method_variant():{init_ev.method_variant()}')

        # responder receives just works
        responder_ev = await anext(self.responder_pairing_event_stream)
        logging.debug(f'[{self.responder_pairing_event_stream.device.name}] responder_ev.method_variant():{responder_ev.method_variant()}')

        if self.initiator_pairing_event_stream == self.android_pairing_stream:
            assert_equal(init_ev.method_variant(), 'numeric_comparison')
            assert_equal(responder_ev.method_variant(), 'just_works')
        else:
            assert_equal(init_ev.method_variant(), 'just_works')

        init_ev_ans = PairingEventAnswer(event=init_ev, confirm=True)
        self.initiator_pairing_event_stream.send_nowait(init_ev_ans)
        responder_ev_ans = PairingEventAnswer(event=responder_ev, confirm=True)
        self.responder_pairing_event_stream.send_nowait(responder_ev_ans)
