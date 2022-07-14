#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

class Kaiser7010 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x2000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }
	bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		SelectPRGPage(0, 10);
		SelectPRGPage(1, 11);
		SelectPRGPage(2, 6);
		SelectPRGPage(3, 7);
		SelectCHRPage(0, 0);
		SetCpuMemoryMapping(0x6000, 0x7FFF, 0, PrgMemoryType::PrgRom);
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		if ((addr >= 0xCAB6) && (addr <= 0xCAD7)) {
			SelectCHRPage(0, (addr >> 2) & 0x0F);
			SetCpuMemoryMapping(0x6000, 0x7FFF, (addr >> 2) & 0x0F, PrgMemoryType::PrgRom);
		}
		else if (((addr & 0xFFFE) == 0xEBE2) || ((addr & 0xFFFE) == 0xEE32)) {
			SelectCHRPage(0, (addr >> 2) & 0x0F);
			SetCpuMemoryMapping(0x6000, 0x7FFF, (addr >> 2) & 0x0F, PrgMemoryType::PrgRom);
		}
		else if ((addr & 0xFFFE) == 0xFFFC) {
			SelectCHRPage(0, (addr >> 2) & 0x0F);
			SetCpuMemoryMapping(0x6000, 0x7FFF, (addr >> 2) & 0x0F, PrgMemoryType::PrgRom);
		}
		return InternalReadRam(addr);
	}
};