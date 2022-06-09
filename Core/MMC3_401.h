#pragma once
#include "MMC3_45.h"

class MMC3_401 : public MMC3_45
{
private:
	uint32_t GetDipSwitchCount() override { return 3; }
	bool AllowRegisterRead() override { return true; }

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		page &= 0x1F ^ (_reg[3] & 0x1F); // mask
		page |= (_reg[2] & 0x80) | (_reg[1] & 0x1F); // outerbank
		page |= (GetDipSwitches() & 0x02) ? (_reg[2] & 0x20): ((_reg[1] >> 1) & 0x20); // A18
		page |= (GetDipSwitches() & 0x04) ? (_reg[2] & 0x40): ((_reg[1] << 1) & 0x40); // A19

		if((GetDipSwitches() & 0x01) && (_reg[1] & 0x80)) {
			// openbus
		} else {
			MMC3::SelectPRGPage(slot, page, memoryType);
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		if((addr >= 0x8000) && (addr <= 0xFFFF)) {
			if((GetDipSwitches() & 0x01) && (_reg[1] & 0x80)) {
				return _console->GetMemoryManager()->GetOpenBus();
			}
		}
		return InternalReadRam(addr);
	}
};