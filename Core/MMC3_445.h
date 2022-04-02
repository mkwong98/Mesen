#pragma once
#include "stdafx.h"
#include "MMC3.h"

class MMC3_445 : public MMC3
{
private:
	uint8_t _reg[4];

protected:

	void InitMapper() override
	{
		AddRegisterRange(0x5000, 0x5FFF);
		MMC3::InitMapper();
	}

	void Reset(bool softreset) override
	{
		_reg[0] = _reg[1] = _reg[2] = _reg[3] = 0;
		MMC3::UpdateState();
	}

	void StreamState(bool saving) override
	{
		MMC3::StreamState(saving);
		Stream(_reg[0], _reg[1], _reg[2], _reg[3]);
	}

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		uint8_t mask = (_reg[2] & 0x08) ? 0x7F : 0xFF;
		MMC3::SelectCHRPage(slot, ((_reg[1] << 3) & ~mask) | (page & mask), memoryType);
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		uint8_t mask = (_reg[2] & 0x01) ? 0x0F: 0x1F;
		MMC3::SelectPRGPage(slot, (_reg[0] & ~mask) | (page & mask), memoryType);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			if(MMC3::CanWriteToWorkRam()) {
				if(!(_reg[3] & 0x20)) {
					_reg[addr & 0x03] = value;
					MMC3::UpdatePrgMapping();
					MMC3::UpdateChrMapping();
				}
			}
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};