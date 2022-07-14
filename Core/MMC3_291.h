#pragma once
#include "stdafx.h"
#include "MMC3.h"

class MMC3_291 : public MMC3
{
private:
	uint8_t _outerBank;

protected:
	virtual void InitMapper() override
	{
		_outerBank = 0;
		AddRegisterRange(0x6000, 0x7FFF, MemoryOperation::Write);
		MMC3::InitMapper();
	}

	virtual void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		if(_outerBank & 0x20) {
			page = ((_outerBank >> 3) & 0x08) | (_outerBank & 0x06);
			BaseMapper::SelectPrgPage4x(0, page << 1);
		} else {
			page &= 0x0F;
			page |= ((_outerBank >> 2) & 0x10);
			BaseMapper::SelectPRGPage(slot, page, memoryType);
		}
	}

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		page |= ((_outerBank << 2) & 0x100);
		BaseMapper::SelectCHRPage(slot, page);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			_outerBank = value;
			UpdateState();
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};