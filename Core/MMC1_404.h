#pragma once
#include "MMC1.h"

// JY012005 - 1998 Super HiK 8-in-1 (JY-021B) multicart.

class MMC1_404 : public MMC1
{
private:
	uint8_t _outerPrgBank;
	uint8_t _outerChrBank;
	uint8_t _prgMask;
	uint8_t _reg;

protected:
	void InitMapper() override
	{
		AddRegisterRange(0x6000, 0x7FFF, MemoryOperation::Write);
		MMC1::InitMapper();
	}

	void Reset(bool softReset) override
	{
		MMC1::Reset(softReset);

		_outerPrgBank = 0;
		_outerChrBank = 0;
		_prgMask = 0x0F;
		_reg = 0;

		UpdateState();
	}

	void StreamState(bool saving) override
	{
		MMC1::StreamState(saving);
		Stream(_outerPrgBank, _outerChrBank, _prgMask, _reg);
	}

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType) override
	{
		MMC1::SelectCHRPage(slot, _outerChrBank | (page & 0x1F), memoryType);
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType) override
	{
		MMC1::SelectPRGPage(slot, _outerPrgBank | (page & _prgMask), memoryType);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			if (!(_reg & 0x80)) {
				_reg = value;
				_prgMask = (_reg & 0x40) ? 0x07 : 0x0F;
				_outerPrgBank = ((_reg & 0x0F) << 3) & ~_prgMask;
				_outerChrBank = ((_reg & 0x0F) << 5);
				UpdateState();
			}
		} else {
			MMC1::WriteRegister(addr, value);
		}
	}
};