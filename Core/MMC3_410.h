#pragma once
#include "MMC3_45.h"

class MMC3_410 : public MMC3_45
{
private:
	uint16_t GetChrRamPageSize() override { return 0x400; }
	uint32_t GetChrRamSize() override { return 0x2000; }

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		uint8_t mask = 0xFF >> (0x0F - (_reg[2] & 0x0F));
		if(_reg[2] & 0x40) {
            MMC3::SelectChrPage8x(0, 0, ChrMemoryType::ChrRam);
        } else {
			page &= mask;
			page |= _reg[0] | ((_reg[2] & 0xF0) << 4);
			MMC3::SelectCHRPage(slot, page);
		}
	}
};