#pragma once
#include "MMC1.h"

class MMC1_111 : public MMC1
{
protected:
	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		// Traditional MMC1 only has 128K Chr, but this mapper has 256K
		if((_state.Reg8000 & 0x10) == 0x10) { // 4K Mode
			page = slot ? _state.RegC000 : _state.RegA000;
		} else { // 8K Mode
			page = (_state.RegA000 & 0x3E) + slot;
		}
		MMC1::SelectCHRPage(slot, page, memoryType);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch((addr & 0x6000) >> 13) {
			case 0: _state.Reg8000 = value; break;
			case 1: _state.RegA000 = value; break;
			case 2: _state.RegC000 = value; break;
			case 3: _state.RegE000 = value; break;
		}

		MMC1::UpdateState();
	}
};