#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

// (KS-7016B) Kaiser port of Meikyuu Jiin Dababa from the Famicom Disk System.

class Mapper549 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x2000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPrgPage4x(0, (0x02) << 2);
		SelectCHRPage(0, 0);

		WriteRegister(0x8000, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SetCpuMemoryMapping(0x6000, 0x7FFF, (addr >> 2) | ((addr >> 3) & 0x04), PrgMemoryType::PrgRom);
	}
};
