/*
 * Copyright 2020 Google LLC
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

#ifndef ECCLESIA_LIB_MCEDECODER_MCE_DECODE_MOCK_H_
#define ECCLESIA_LIB_MCEDECODER_MCE_DECODE_MOCK_H_

#include "testing/base/public/gmock.h"
#include "lib/mcedecoder/mce_decode.h"
#include "lib/mcedecoder/mce_messages.h"

namespace mcedecoder {

class MockMceDecoder : public MceDecoderInterface {
 public:
  MOCK_METHOD(bool, DecodeMceMessage,
              (const MceLogMessage& raw_msg, MceDecodedMessage* decoded_msg),
              (override));
};
}  // namespace mcedecoder
#endif  // ECCLESIA_LIB_MCEDECODER_MCE_DECODE_MOCK_H_
