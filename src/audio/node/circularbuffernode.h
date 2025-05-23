/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <audio/core/audionode.h>
#include <audio/utility/audiofunctions.h>
#include <audio/utility/safeptr.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Manages a circular buffer that when processed will be filled by input coming in through the @audioInput pin.
         * A circular buffer internally saves a write position at which new input will be written.
         * The write position wraps around when the end of the buffer has been reach. (hence the "circular")
         * Samples can be read from the circular buffer relatively to the write position.
         * Default will be processed as root process by the @NodeManager.
         */
        class NAPAPI CircularBufferNode : public Node
        {
            RTTI_ENABLE(Node)
        public:
            /**
             * Differs to the default signature of @Node constructors and therefore cannot be wrapped in a NodeObject.
             * @param nodeManager @NodeManager that de Node will be processed by.
             * @param bufferSize Size of the circular buffer. Has to be a power of two.
             */
            CircularBufferNode(NodeManager& nodeManager, unsigned int bufferSize);

            ~CircularBufferNode() override;

            /**
             * Input signal that will be fed into the circular buffer.
             */
            InputPin audioInput = { this };

            /**
             * Reads a sample from the circular buffer at an absolute position, regardless of the current write position.
             * @param absolutePosition Absolute discrete sample position in the buffer, regardless of the current write position.
             * @return Value of the sample at the specified position.
             */
            inline const SampleValue& getSample(const DiscreteTimeValue& absolutePosition) const { return mBuffer[wrap(absolutePosition, mBuffer.size())]; }

            /**
             * Translates a position relative to the current write position to an absolute position.
             * @param relativePosition Position relative to the current write position.
             * @return Absolute position in the circular buffer.
             */
            DiscreteTimeValue getAbsolutePosition(unsigned int relativePosition) const { return wrap(mWritePosition - relativePosition, mBuffer.size()); }

			/**
			 * Clears the contents of the buffer. Either perform this on the audio thread or while the node is not processing.
			 */
			void clear();
            
        private:
            void process() override;

            SampleBuffer mBuffer;
            DiscreteTimeValue mWritePosition = 0;
            
			std::mutex mMutex; // Mutex lock to guard against probably false positive thread sanitizer warnings about process() and clear().
        };
        
    }
}
