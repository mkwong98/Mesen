#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

// bootleg cartridge conversion named Super Soccer Champion of the Konami FDS game Exciting Soccer.

class Mapper538 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x2000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		RemoveRegisterRange(0x0000, 0xFFFF, MemoryOperation::Any);
		AddRegisterRange(0xC000, 0xDFFF, MemoryOperation::Write);

		WriteRegister(0xC000, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SetCpuMemoryMapping(0x6000, 0x7FFF, value | 0x01, PrgMemoryType::PrgRom);

		SelectPRGPage(0, ((value & 0x01) && !(value & 0x08)) ? 0x0A : (value & ~1));
		SelectPRGPage(1, -3);
		SelectPRGPage(2, -2);
		SelectPRGPage(3, -1);

		SelectCHRPage(0, 0);
	}
};
