#pragma once
#include "stdafx.h"
#include "MMC3.h"

class MMC3_441 : public MMC3
{
private:
	uint8_t _reg;

protected:
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }

	void InitMapper() override
	{
		MMC3::InitMapper();
	}

	void Reset(bool softreset) override
	{
		_reg = 0;
		MMC3::UpdateState();
	}

	void StreamState(bool saving) override
	{
		MMC3::StreamState(saving);
		Stream(_reg);
	}

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		uint8_t mask = (_reg & 0x40) ? 0x7F : 0xFF;
		page &= mask;
		page |= ((_reg << 3) & 0x180) & ~mask;
		MMC3::SelectCHRPage(slot, page, memoryType);
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		uint8_t mask = (_reg & 0x08) ? 0x0F: 0x1F;
		uint8_t outerBank = (_reg << 4) & 0x30;
		if(_reg & 0x04) {
			MMC3::SelectPRGPage(0, (outerBank & ~mask) | ((MMC3::_registers[6] & ~0x02) & mask));
			MMC3::SelectPRGPage(1, (outerBank & ~mask) | ((MMC3::_registers[7] & ~0x02) & mask));
			MMC3::SelectPRGPage(2, (outerBank & ~mask) | ((MMC3::_registers[6] |  0x02) & mask));
			MMC3::SelectPRGPage(3, (outerBank & ~mask) | ((MMC3::_registers[7] |  0x02) & mask));
		} else {
			MMC3::SelectPRGPage(slot, (outerBank & ~mask) | (page & mask));
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			if(MMC3::CanWriteToWorkRam()) {
				if(!(_reg & 0x80)) {
					_reg = value;
					MMC3::UpdatePrgMapping();
					MMC3::UpdateChrMapping();
				}
			}
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};