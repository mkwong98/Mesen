#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

class Mapper541 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPrgPage2x(0, 0);
		SelectCHRPage(0, 0);

		RemoveRegisterRange(0x8000, 0xBFFF, MemoryOperation::Write);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if (addr & 0x02) {
			SelectPRGPage(0, addr >> 2);
			SelectPRGPage(1, addr >> 2);
		} else {
			SelectPrgPage2x(0, (addr >> 2) & 0xFE);
		}
		SetMirroringType((addr & 0x01) ? MirroringType::Vertical : MirroringType::Horizontal);
	}
};
