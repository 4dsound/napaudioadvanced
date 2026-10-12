#include "eqnode.h"

namespace nap
{

	namespace audio
	{

		void EqNode::prepare(int index, Eq::Mode mode, ControllerValue frequency, ControllerValue gain, ControllerValue q)
		{
			mEQs[index].prepare(mode, frequency, getNodeManager().getSampleRate(), gain, q);
		}

		void EqNode::set(int index, Eq::Mode mode, ControllerValue frequency, ControllerValue gain, ControllerValue q)
		{
			mEQs[index].set(mode, frequency, getNodeManager().getSampleRate(), gain, q);
		}


		void EqNode::process()
		{
			auto& outputBuffer = getOutputBuffer(mOutput);
			auto inputBuffer = mInput.pull();

			if (inputBuffer == nullptr)
			{
				std::fill(outputBuffer.begin(), outputBuffer.end(), 0.f);
				return;
			}

			int sampleIndex = 0;
			for (auto& sample : outputBuffer)
			{
				sample = (*inputBuffer)[sampleIndex++];
				int eqIndex = 0;
				for (auto& eq : mEQs)
					if (mEnabled[eqIndex++])
						sample = eq.process(sample);
			}
		}

	}

}