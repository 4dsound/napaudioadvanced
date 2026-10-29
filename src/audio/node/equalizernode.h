#pragma once

#include <audio/utility/Equalizer.h>
#include <audio/core/audionode.h>
#include <audio/core/audionodemanager.h>

namespace nap
{
	namespace audio
	{

		class NAPAPI EqualizerNode : public Node
		{
			RTTI_ENABLE(Node)

		public:
			/**
			 * Number of bands
			 */
			static constexpr int bandCount = 8;

			EqualizerNode(NodeManager& nodeManager) : Node(nodeManager)
			{
				for (auto& eq : mEQs)
					eq.prepare(Equalizer::Mode::Peak, 440, nodeManager.getSampleRate(), 0.f, 0.f);

				for (auto& enable : mEnabled)
					enable.store(false);
			}

			void prepare(int index, Equalizer::Mode mode, ControllerValue frequency, ControllerValue gain, ControllerValue q);
			void set(int index, Equalizer::Mode mode, ControllerValue frequency, ControllerValue gain, ControllerValue q);
			void setEnabled(int index, bool enabled) { mEnabled[index] = enabled; }

			InputPin mInput = { this };
			OutputPin mOutput = { this };

		private:
			void process() override;

			void sampleRateChanged(float sampleRate) override
			{
				for (auto& eq : mEQs)
					eq.setSampleRate(sampleRate);
			}

			std::array<Equalizer, bandCount> mEQs;
			std::array<std::atomic<bool>, bandCount> mEnabled;
		};

	}
}
