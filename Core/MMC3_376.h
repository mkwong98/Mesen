#pragma once
#include "stdafx.h"
#include "MMC3.h"

class MMC3_376 : public MMC3
{
private:
	uint8_t _exReg[2];

protected:
	uint16_t RegisterStartAddress() override { return 0x7000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }

	void InitMapper() override
	{
		_exReg[0] = _exReg[1] = 0;
		MMC3::InitMapper();
	}

	void StreamState(bool saving) override
	{
		MMC3::StreamState(saving);
		Stream(_exReg[0], _exReg[1]);
	}

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		page &= 0x7F;
		page |= ((_exReg[1] << 8) & 0x100) | ((_exReg[0] << 1) & 0x80);
		MMC3::SelectCHRPage(slot, page, memoryType);
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		uint8_t outerBank = ((_exReg[1] << 4) & 0x10) | ((_exReg[0] >> 3) & 0x08) | (_exReg[0] & 0x07);
		if(_exReg[0] & 0x80) {
			if(_exReg[0] & 0x20) {
				MMC3::SelectPrgPage4x(0, (outerBank & 0xFE) << 1);
			} else {
				MMC3::SelectPrgPage2x(0, outerBank << 1);
				MMC3::SelectPrgPage2x(1, outerBank << 1);
			}
		} else {
			MMC3::SelectPRGPage(slot, ((outerBank << 1) & ~0x0F) | (page & 0x0F), memoryType);
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			_exReg[addr & 0x01] = value;
			MMC3::UpdatePrgMapping();
			MMC3::UpdateChrMapping();
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};