#pragma once
#include "stdafx.h"
#include "MMC3.h"

class MMC3_412 : public MMC3
{
private:
	uint8_t _exRegs[4];

protected:
	virtual uint16_t RegisterStartAddress() override { return 0x6000; }
	virtual uint16_t RegisterEndAddress() override { return 0xFFFF; }

	void InitMapper() override
	{
		MMC3::InitMapper();
	}

	void Reset(bool softreset) override
	{
		_exRegs[0] = 0;
		_exRegs[1] = 0;
        _exRegs[2] = 0;
		_exRegs[3] = 0;
		MMC3::UpdateState();
	}

	void StreamState(bool saving) override
	{
		MMC3::StreamState(saving);
		Stream(_exRegs[0], _exRegs[1], _exRegs[2], _exRegs[3]);
	}

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
        if (_exRegs[2] & 0x02) {
            MMC3::SelectChrPage8x(0, _exRegs[0] << 1);
        } else {
		    page &= 0x7F;
		    MMC3::SelectCHRPage(slot, (_exRegs[1] & 0x80) | page);
        }
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		if (_exRegs[2] & 0x02) {		// NROM Mode
			uint8_t bank = (_exRegs[2] >> 3);
			if (_exRegs[2] & 0x04) {	// NROM-256
				SelectPrgPage4x(0, (bank & 0xFE) << 1);
			} else {					// NROM-128
				MMC3::SelectPrgPage2x(0, bank << 1);
				MMC3::SelectPrgPage2x(1, bank << 1);
			}
		} else {						// MMC3 Mode
            uint8_t mask = 0x3F & ~((_exRegs[1] << 3) & 0x38);
			page &= mask;
            page |= ((_exRegs[1] >> 2) & 0x3E) & ~mask;
			MMC3::SelectPRGPage(slot, page);
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			_exRegs[addr & 0x03] = value;
			MMC3::UpdatePrgMapping();
			MMC3::UpdateChrMapping();
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};