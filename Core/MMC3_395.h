#pragma once
#include "stdafx.h"
#include "MMC3.h"

class MMC3_395 : public MMC3
{
private:
	uint8_t _exReg[2];

protected:
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }

	void Reset(bool softreset) override
	{
		_exReg[0] = _exReg[1] = 0;
		MMC3::UpdateState();
	}

	void StreamState(bool saving) override
	{
		MMC3::StreamState(saving);
		Stream(_exReg[0], _exReg[1]);
	}

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		uint8_t mask = (_exReg[1] & 0x40) ? 0x7F: 0xFF;
        uint16_t outerBank = ((_exReg[1] << 5) & 0x400) | ((_exReg[0] << 4) & 0x300) | ((_exReg[1] << 3) & 0x80);
        MMC3::SelectCHRPage(slot, outerBank | (page & mask));
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		uint8_t mask = (_exReg[1] & 0x08) ? 0x0F: 0x1F;
        uint8_t outerBank = ((_exReg[0] << 4) & 0x80) | ((_exReg[0] << 1) & 0x60) | ((_exReg[1] << 4) &0x10);
        MMC3::SelectPRGPage(slot, outerBank | (page & mask), memoryType);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
            WritePrgRam(addr, value);
            if(!(_exReg[1] & 0x80)) {
			    _exReg[(addr >> 4) & 0x01] = value;
			    MMC3::UpdatePrgMapping();
			    MMC3::UpdateChrMapping();
            }
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};