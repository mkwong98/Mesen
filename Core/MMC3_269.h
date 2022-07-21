#pragma once
#include "stdafx.h"
#include "MMC3.h"

class MMC3_269 : public MMC3
{
protected:
	uint8_t _regIndex;
	uint16_t _prgMask;
	uint16_t _outerPrg;
	uint16_t _chrMask;
	uint16_t _outerChr;
	bool _locked;

	void InitMapper() override
	{
		MMC3::InitMapper();

		AddRegisterRange(0x5000, 0x5FFF, MemoryOperation::Write);
	}

	void Reset(bool softReset) override
	{
		_regIndex = 0;
		_prgMask = 0x3F;
		_chrMask = 0xFF;
		_outerPrg = 0x00;
		_outerChr = 0x00;
		_locked = false;

		UpdateState();
	}

	void StreamState(bool saving) override
	{
		MMC3::StreamState(saving);
		Stream(_regIndex, _outerPrg, _outerChr, _prgMask, _chrMask, _locked);
	}

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType) override
	{
		page &= _chrMask;
		page |= _outerChr;

		MMC3::SelectCHRPage(slot, page, memoryType);
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		page &= _prgMask;
		page |= _outerPrg;

		MMC3::SelectPRGPage(slot, page, memoryType);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			if (!_locked) {
				switch (_regIndex) {
					case 0:
						_outerChr &= 0xFF00;
						_outerChr |= value;
						break;

					case 1:
						_outerPrg = 0xFF00;
						_outerPrg |= value;
						break;

					case 2:
						_chrMask = 0xFF >> (~value & 0xF);
						_outerChr = (_outerChr & ~0x0F00) | ((value & 0xF0) << 4);
						break;

					case 3:
						_prgMask = ~value & 0x3F;
						_outerPrg = (_outerPrg & ~0x0100) | ((value & 0x40) << 2);
						_outerChr = (_outerChr & ~0x1000) | ((value & 0x40) << 6);
						_locked = (value & 0x80) == 0x80;
						break;
				}
				_regIndex = (_regIndex + 1) & 0x03;
				MMC3::UpdateState();
			}
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};