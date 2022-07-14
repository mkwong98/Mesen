#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

// K-3071

class Mapper438 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		WriteRegister(0x8000, 0);
	}

	void Reset(bool softReset) override
	{
		WriteRegister(0x8000, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SetMirroringType((value & 0x01) ? MirroringType::Horizontal : MirroringType::Vertical);

		if (addr & 0x01) {
			SelectPrgPage2x(0, (addr >> 1) & 0xFE);
		} else {
			SelectPRGPage(0, addr >> 1);
			SelectPRGPage(1, addr >> 1);
		}

		SelectCHRPage(0, value >> 1);
	}
};
