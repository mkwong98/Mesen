#pragma once
#include "MMC1.h"

class MMC1_297 : public MMC1
{
private:
	uint8_t _mode;
	uint8_t _latch;

protected:
	void InitMapper() override
	{
		_latch = 0;
		_mode = 0;
		MMC1::InitMapper();
		AddRegisterRange(0x4120, 0x4120, MemoryOperation::Write);
	}

	void StreamState(bool saving) override
	{
		MMC1::StreamState(saving);
		Stream(_mode, _latch);
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		BaseMapper::SelectPRGPage(slot, ((_mode << 3) & 0x08) | (page & 0x07));
	}

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		BaseMapper::SelectCHRPage(slot, ((_mode << 5) & 0x20) | (page & 0x1F));
	}

	void UpdateState() override
	{
		if(_mode & 0x01) {	// MMC1
			MMC1::UpdateState();
		} else {			// Mapper 70
			BaseMapper::SelectPRGPage(0, ((_mode & 0x02) << 1) | ((_latch >> 4) & 0x03));
			BaseMapper::SelectPRGPage(1, ((_mode & 0x02) << 1) | 0x03);
			BaseMapper::SelectChrPage2x(0, (_latch & 0xF) << 1);
			BaseMapper::SetMirroringType(MirroringType::Vertical);
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			if (addr & 0x100) {
				_mode = value;
				UpdateState();
			}
		} else {
			if(_mode & 1) {
				MMC1::WriteRegister(addr, value);
			} else {
				_latch = value;
				UpdateState();
			}
		}
	}
};
