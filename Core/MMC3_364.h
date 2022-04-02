#pragma once
#include "MMC3.h"

class MMC3_364 : public MMC3
{
protected:
	uint8_t _exReg;

	void InitMapper() override
	{
		_exReg = 0;
		MMC3::InitMapper();
        AddRegisterRange(0x7000, 0x7FFF, MemoryOperation::Write);
	}

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType) override
	{
		uint8_t mask = (_exReg & 0x20) ? 0x7F : 0xFF;
		MMC3::SelectCHRPage(slot, ((_exReg << 4) & 0x100) | (page & mask));
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
        uint8_t mask = (_exReg & 0x20) ? 0x0F : 0x1F;
		MMC3::SelectPRGPage(slot, ((_exReg >> 1) & 0x20) | (page & mask));
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr <= 0x7FFF) {
			_exReg = value;
			UpdatePrgMapping();
			UpdateChrMapping();
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};