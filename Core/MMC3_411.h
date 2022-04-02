#pragma once
#include "stdafx.h"
#include "MMC3.h"

class MMC3_411 : public MMC3
{
private:
	uint8_t _exRegs[2];

protected:
	virtual void InitMapper() override
	{
		AddRegisterRange(0x5000, 0x5FFF);
		MMC3::InitMapper();
	}

	virtual void Reset(bool softreset) override
	{
		_exRegs[0] = 0x80;
		_exRegs[1] = 0x82;
		MMC3::UpdateState();
	}

	virtual void StreamState(bool saving) override
	{
		MMC3::StreamState(saving);
		Stream(_exRegs[0], _exRegs[1]);
	}

	virtual void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		page &= (_exRegs[1] & 2) ? 0xFF : 0x7F;
		MMC3::SelectCHRPage(slot, ((_exRegs[1] << 5) & 0x80) | ((_exRegs[0] << 4) & 0x100) | page);
	}

	virtual void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		if (_exRegs[0] & 0x40) {		// NROM Mode
			uint8_t bank = ((_exRegs[1] >> 2) & 0x10) | (_exRegs[1] & 0x08) | (_exRegs[0] & 0x04) | ((_exRegs[0] >> 2) & 0x02) | (_exRegs[0] & 0x01);
			if (_exRegs[0] & 0x02) {	// NROM-256
				SelectPrgPage4x(0, (bank & 0xFE) << 1);
			} else {					// NROM-128
				MMC3::SelectPrgPage2x(0, bank << 1);
				MMC3::SelectPrgPage2x(1, bank << 1);
			}
		} else {						// MMC3 Mode
			page &= (_exRegs[1] & 2) ? 0x1F : 0x0F;
			MMC3::SelectPRGPage(slot, ((_exRegs[1] >> 1) & 0x20) | ((_exRegs[1] << 1) & 0x10) | page);
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr <= 0x5FFF) {
			_exRegs[addr & 0x01] = value;
			MMC3::UpdatePrgMapping();
			MMC3::UpdateChrMapping();
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};