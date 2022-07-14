#pragma once

#include "stdafx.h"
#include "MMC3.h"

class MMC3_199 : public MMC3
{
protected:
	uint32_t GetChrRamSize() override { return 0x2000; }
	uint16_t GetChrRamPageSize() override { return 0x2000; }

	uint32_t GetWorkRamPageSize() override { return 0x1000; }

	void InitMapper() override
	{
		MMC3::InitMapper();

		SetCpuMemoryMapping(0x5000, 0x5FFF, 2, PrgMemoryType::WorkRam);
		SetCpuMemoryMapping(0x6000, 0x6FFF, 0, HasBattery() ? PrgMemoryType::SaveRam : PrgMemoryType::WorkRam);
		SetCpuMemoryMapping(0x7000, 0x7FFF, 1, HasBattery() ? PrgMemoryType::SaveRam : PrgMemoryType::WorkRam);
	}

	void UpdateChrMapping() override
	{
		SelectCHRPage(0, 0);
	}
};