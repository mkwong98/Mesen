#pragma once

#include "stdafx.h"
#include "MMC3.h"

class MMC3_456: public MMC3
{
private:
	uint8_t _outerBank;

protected:
	void InitMapper() override
	{
		_outerBank = 0;

		AddRegisterRange(0x4100, 0x5FFF, MemoryOperation::Write);

		MMC3::InitMapper();
	}

	void Reset(bool softReset) override
	{
		_outerBank = 0;

		MMC3::UpdateState();
	}


	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		MMC3::SelectCHRPage(slot, (_outerBank << 7) | (page & 0x7F));
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		MMC3::SelectPRGPage(slot, (_outerBank << 4) | (page & 0x0F));
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
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