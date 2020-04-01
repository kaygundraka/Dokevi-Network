#pragma once
#include "pch.h"
#include "ISingleton.h"

namespace DokeviNet
{
	class RandGenerator : public ISingleton<RandGenerator>
	{
	public:
		virtual void Initialize() override
		{
			srand(static_cast<unsigned int>(std::time(0)));
		}

		float GetFloat(int min, int max)
		{
			float result = (float)(std::rand() % ((max - min + 1) * 1000)) / 1000.0f;
			result += min;

			return result;
		}

		int GetInt(int min, int max)
		{
			int result = std::rand() % (max - min + 1);
			result += min;

			return result;
		}
	};
}