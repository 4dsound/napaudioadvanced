#pragma once

#include <audio/utility/eq.h>
#include <audio/core/audionode.h>
#include <audio/core/audionodemanager.h>

namespace nap
{
	namespace audio
	{

		class NAPAPI EqNode : public Node
		{
			RTTI_ENABLE(Node)

		public:
			EqNode(NodeManager& nodeManager) : Node(nodeManager)
			{
				for (auto& eq : mEQs)
					eq.prepare(Eq::Mode::Peak, 440, nodeManager.getSampleRate(), 0.f, 0.f);

				for (auto& enable : mEnabled)
					enable.store(false);
			}

			void prepare(int index, Eq::Mode mode, ControllerValue frequency, ControllerValue gain, ControllerValue q);
			void set(int index, Eq::Mode mode, ControllerValue frequency, ControllerValue gain, ControllerValue q);
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

			std::array<Eq, 8> mEQs;
			std::array<std::atomic<bool>, 8> mEnabled;
		};

	}
}
