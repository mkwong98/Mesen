#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

class Mapper429 : public BaseMapper
{
protected:
	virtual uint16_t GetPRGPageSize() override { return 0x8000; }
	virtual uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		WriteRegister(0x8000, 0x04);
	}

	virtual void Reset(bool softReset) override
	{
		WriteRegister(0x8000, 0x04);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectPRGPage(0, value >> 2);
		SelectCHRPage(0, value & 0x03);
	}
};
