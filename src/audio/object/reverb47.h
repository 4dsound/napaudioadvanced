/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <audio/node/reverbnode47.h>
#include <audio/core/nodeobject.h>

namespace nap
{

    namespace audio
    {

        namespace verb47
        {

            /**
             * Resource for a multichannel reverb47 audio object, containing parallel ReverbNodes
             * Reverb47 is a reverberation algorithm controlled by 4 main parameters: size, decay, damping and diffusion
             * Additionally the pitch of the reverb tail can be modulated with low frequent noise using 3 additional parameters: bandwidth, speed and cutoff frequency.
             */
            class NAPAPI Reverb47 : public ParallelNodeObject<ReverbNode>
            {
            RTTI_ENABLE(ParallelNodeObjectBase)

            public:
                Reverb47() = default;

                std::vector<float> mCorrelationMultiplier = { 1.f, 1.1f }; ///< Property: 'CorrelationMultiplier' Multiplication factor for all "magic" tuning numbers for the reverberation algorithm
                bool mDiffusionCrossover = true;                           ///< Property: 'DiffusionCrossOver' Set to true if the diffusion inputs and outputs of the parallel reverb nodes should be connected to one another. Don't set to true when the object is processed multithreaded in parallel.
				float mSize = 0.8f;
                float mDecay = 0.8f;
				float mDamping = 0.55f;
				float mDiffusion = 0.55f;

                // Inherited from ParallelNodeObject
                bool initNode(int channel, ReverbNode& node, utility::ErrorState& errorState) override;

            private:
                std::vector<ReverbNode*> mInitializedNodes;
            };


            /**
             * Instance of Reverb47
             */
            using ReverbInstance47 = ParallelNodeObjectInstance<ReverbNode>;


        }

    }

}