#pragma once
#include "stdafx.h"
#include "BaseMapper.h"
#include "CPU.h"

//  FDS Conversion of Roger Rabbit to Lucky Rabbit

class Mapper415 : public BaseMapper
{
protected:
	virtual uint16_t GetPRGPageSize() override { return 0x2000; }
	virtual uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPRGPage(0, -4);
		SelectPRGPage(1, -3);
		SelectPRGPage(2, -2);
		SelectPRGPage(3, -1);

		SelectCHRPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SetCpuMemoryMapping(0x6000, 0x7FFF, value & 0x0F, PrgMemoryType::PrgRom);
		SetMirroringType((value & 0x10) ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};
