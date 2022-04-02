#pragma once
#include "stdafx.h"
#include "MMC3.h"

class MMC3_377 : public MMC3
{
private:
	uint8_t _outerBank = 0;

protected:
	virtual uint16_t RegisterStartAddress() override { return 0x6000; }
	virtual uint16_t RegisterEndAddress() override { return 0xFFFF; }

	virtual void Reset(bool softreset) override
    {
        _outerBank = 0;
        MMC3::UpdateState();
    }

    virtual void StreamState(bool saving) override
	{
		MMC3::StreamState(saving);
		Stream(_outerBank);
	}

	virtual void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		page &= 0x7F;
        page |= (((_outerBank >> 2) & 0x08) | (_outerBank & 0x06)) << 6;
		MMC3::SelectCHRPage(slot, page, memoryType);
	}

	virtual void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
        page &= 0x0F;
        page |= (((_outerBank >> 2) & 0x08) | (_outerBank & 0x06)) << 3;
		MMC3::SelectPRGPage(slot, page, memoryType);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
            if(!(_outerBank & 0x80)) {
			    _outerBank = value;
			    MMC3::UpdatePrgMapping();
                MMC3::UpdateChrMapping();
            }
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};