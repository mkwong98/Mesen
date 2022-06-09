#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

class Bmc235 : public BaseMapper
{
protected:
	virtual uint16_t GetPRGPageSize() override { return 0x4000; }
	virtual uint16_t GetCHRPageSize() override { return 0x2000; }

	uint8_t _unrom;

	void InitMapper() override
	{
		SelectPrgPage2x(0, 0);
		SelectCHRPage(0, 0);
	}

	void Reset(bool softReset) override
	{
		SelectPrgPage2x(0, 0);
		if(_prgSize & 0x20000) {
			if(softReset) {
				_unrom = !_unrom;
			} else {
				_unrom = false;
			}
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		uint8_t bank = ((addr >> 3) & 0x60) | (addr & 0x1F);

		if(_unrom) {
			SetMirroringType(MirroringType::Vertical);
			SelectPRGPage(0, (GetPRGPageCount() & 0xC0) | (value & 0x07));
			SelectPRGPage(1, (GetPRGPageCount() & 0xC0) | 0x07);
			return;
		}

		if(bank >= (GetPRGPageCount() >> 1)) {
			RemoveCpuMemoryMapping(0x8000, 0xFFFF);
			return;
		}

		SetMirroringType((addr & 0x0400) ? MirroringType::ScreenAOnly : (addr & 0x2000) ? MirroringType::Horizontal : MirroringType::Vertical);
		
		if(addr & 0x800) {
			bank = (bank << 1) | (addr >> 12 & 0x01);
			SelectPRGPage(0, bank);
			SelectPRGPage(1, bank);
		} else {
			SelectPrgPage2x(0, bank << 1);
		}
	}
};