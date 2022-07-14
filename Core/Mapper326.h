#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

// NES 2.0 Mapper 326 is used for a bootleg version of Contra/Gryzor.
// https://forums.nesdev.org/viewtopic.php?f=9&t=17352&p=218722#p218722

class Mapper326 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x2000; }
	uint16_t GetCHRPageSize() override { return 0x0400; }

	void InitMapper() override
	{
		SelectPRGPage(3, -1);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xE010) {
			case 0x8000: SelectPRGPage(0, value); break;
			case 0xA000: SelectPRGPage(1, value); break;
			case 0xC000: SelectPRGPage(2, value); break;
		}

		if((addr & 0x8010) == 0x8010) {
			switch(addr & 0x0F) {
				case 0x00: SelectCHRPage(0, value); break;
				case 0x01: SelectCHRPage(1, value); break;
				case 0x02: SelectCHRPage(2, value); break;
				case 0x03: SelectCHRPage(3, value); break;
				case 0x04: SelectCHRPage(4, value); break;
				case 0x05: SelectCHRPage(5, value); break;
				case 0x06: SelectCHRPage(6, value); break;
				case 0x07: SelectCHRPage(7, value); break;
				case 0x08: SetNametable(0, value & 1); break;
				case 0x09: SetNametable(1, value & 1); break;
				case 0x0A: SetNametable(2, value & 1); break;
				case 0x0B: SetNametable(3, value & 1); break;
				default:
					break;
			}
		}
	}
};
