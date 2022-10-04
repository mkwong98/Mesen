#pragma once

#include "stdafx.h"
#include "MMC3.h"

class MMC3_457: public MMC3
{
private:
	uint8_t _outerBank;

protected:
	void InitMapper() override
	{
		_outerBank = 0;

		AddRegisterRange(0x6000, 0x7FFF, MemoryOperation::Write);

		MMC3::InitMapper();
	}

	void Reset(bool softReset) override
	{
		_outerBank = 0;

		MMC3::UpdateState();
	}


	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		uint8_t mask = (_outerBank & 0x08) ? 0xFF : 0x7F;
		MMC3::SelectCHRPage(slot, ((_outerBank << 7) & ~mask) | (page & mask));
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		uint8_t mask = (_outerBank & 0x08) ? 0x1F : 0x0F;
		MMC3::SelectPRGPage(slot, ((_outerBank << 4) & ~mask) | (page & mask));
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			WritePrgRam(addr, value);
			if(CanWriteToWorkRam()) {
				_outerBank = value;
				MMC3::UpdatePrgMapping();
				MMC3::UpdateChrMapping();
			}
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}

	void StreamState(bool saving) override
	{
		MMC3::StreamState(saving);
		Stream(_outerBank);
	}
};