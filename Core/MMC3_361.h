#pragma once
#include "stdafx.h"
#include "MMC3.h"

class MMC3_361 : public MMC3
{
private:
	uint8_t _outerBankReg = 0;

protected:
	void InitMapper() override
	{
		_outerBankReg = 0;
		MMC3::InitMapper();
		AddRegisterRange(0x7000, 0x7FFF, MemoryOperation::Write);
	}

	virtual void StreamState(bool saving) override
	{
		MMC3::StreamState(saving);
		Stream(_outerBankReg);
	}

	virtual void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		page &= 0x07F;
		page |= _outerBankReg << 3;

		MMC3::SelectCHRPage(slot, page, memoryType);
	}

	virtual void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		page &= 0x0F;
		page |= _outerBankReg;

		MMC3::SelectPRGPage(slot, page, memoryType);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			_outerBankReg = value & 0xF0;
			UpdateState();
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};