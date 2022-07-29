#pragma once
#include "MMC1.h"

class MMC1_550 : public MMC1
{
private:
	uint8_t _selectedBlock;
	uint8_t _latch;

protected:
	void InitMapper() override
	{
		_latch = 0;
		_selectedBlock = 0;
		MMC1::InitMapper();
		AddRegisterRange(0x7000, 0x7FFF, MemoryOperation::Write);
	}

	void StreamState(bool saving) override
	{
		MMC1::StreamState(saving);
		Stream(_selectedBlock, _latch);
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		BaseMapper::SelectPRGPage(slot, (_selectedBlock << 2) | (page & 0x07));
	}

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		BaseMapper::SelectCHRPage(slot, ((_selectedBlock << 2) & 0x18) | (page & 0x07));
	}

	void UpdateState() override
	{
		if ((_selectedBlock & 0x06) == 0x06) {
			MMC1::UpdateState();
		} else {
			BaseMapper::SelectPrgPage2x(0, ((_selectedBlock << 1) | (_latch >> 4)) << 1);
			BaseMapper::SelectChrPage2x(0, (((_selectedBlock << 1) & 0x0C) | (_latch & 0x03)) << 1);
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if (addr < 0x8000) {
			WritePrgRam(addr, value);
			if (!(_selectedBlock & 0x08)) {
				_selectedBlock = addr & 0x0F;
				UpdateState();
			}
		} else {
			_latch = value;
			if ((_selectedBlock & 0x06) == 0x06) {
				MMC1::WriteRegister(addr, value);
			}
			UpdateState();
		}
	}
};
