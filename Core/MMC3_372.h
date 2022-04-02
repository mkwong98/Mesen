#pragma once
#include "MMC3_45.h"

class MMC3_372 : public MMC3_45
{
private:
	uint16_t GetChrRamPageSize() override { return 0x400; }
	uint32_t GetChrRamSize() override { return 0x2000; }

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		if(!(_reg[2] & 0x20)) {
			page &= 0xFF >> (0x0F - (_reg[2] & 0x0F));
			page |= _reg[0] | ((_reg[2] & 0xF0) << 4);
		}
		MMC3::SelectCHRPage(slot, page, (_reg[2] & 0x20) ? ChrMemoryType::ChrRam : ChrMemoryType::ChrRom);
	}
};