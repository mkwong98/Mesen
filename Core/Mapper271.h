#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

// BMC-22026 TXC 4-in-1 multicart (MGC-026)

class Mapper271 : public BaseMapper
{
protected:
	virtual uint16_t GetPRGPageSize() override { return 0x8000; }
	virtual uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		WriteRegister(0x8000, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectPRGPage(0, (value >> 4) & 0x03);
		SelectCHRPage(0, value & 0x0F);
		SetMirroringType((value & 0x20) ? MirroringType::Vertical : MirroringType::Horizontal);
	}
};
