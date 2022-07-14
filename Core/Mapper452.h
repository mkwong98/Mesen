#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

class Mapper452 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x2000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	uint32_t GetWorkRamSize() override { return 0x2000; }
	uint32_t GetWorkRamPageSize() override { return 0x2000; }
	bool ForceWorkRamSize() override { return true; }

	void InitMapper() override
	{
		// https://www.nesdev.org/wiki/NES_2.0_Mapper_452
		// $E000 - $FFFF does not respond to latch to allow writing to
		// PRG RAM mapped to that range
		RemoveRegisterRange(0xE000, 0xFFFF, MemoryOperation::Any);

		SelectCHRPage(0, 0);

		WriteRegister(0x8000, 0);
	}

	void Reset(bool softReset) override
	{
		WriteRegister(0x8000, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		uint16_t wramBank = 0x8000 | ((value << 9) & 0x6000);
		uint8_t prgPage = addr >> 1;

		if(value & 0x02) {
			SelectPRGPage(0, prgPage);
			SelectPRGPage(1, prgPage);
			SelectPRGPage(2, prgPage);
			SelectPRGPage(3, prgPage);
			SetCpuMemoryMapping((wramBank ^ 0x4000), (wramBank ^ 0x4000) + 0x1FFF, 0, PrgMemoryType::WorkRam);
		} else {
			if (value & 0x08) {
				SelectPRGPage(0, prgPage | 0);
				SelectPRGPage(1, prgPage | 1);
				SelectPRGPage(2, prgPage | 2);
				SelectPRGPage(3, prgPage | 3 | (value & 0x04));
			} else {
				SelectPrgPage2x(0, prgPage & 0xFE);
				SelectPrgPage2x(1, 0);
			}
		}

		SetMirroringType((value & 0x01) ? MirroringType::Horizontal : MirroringType::Vertical);

		SetCpuMemoryMapping(wramBank, wramBank + 0x1FFF, 0, PrgMemoryType::WorkRam);
	}
};
