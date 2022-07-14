#pragma once
#include "stdafx.h"
#include "MMC3_126.h"

class MMC3_516 : public MMC3
{
private:
	uint8_t _outerReg;

protected:
	void InitMapper() override
	{
		_outerReg = 0;
		MMC3::InitMapper();
	}

	void StreamState(bool saving) override
	{
		MMC3::StreamState(saving);
		Stream(_outerReg);
	}

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		MMC3::SelectCHRPage(slot, ((_outerReg << 5) & 0x180) | (page & 0x7F), memoryType);
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		MMC3::SelectPRGPage(slot, ((_outerReg << 4) & 0x30) | (page & 0x0F), memoryType);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if (addr & 0x10) {
			_outerReg = addr & 0x0F;
		}
		MMC3::WriteRegister(addr, value);
	}
};
