#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

class Sachen3013 : public BaseMapper
{
protected:
	virtual uint16_t GetPRGPageSize() override { return 0x8000; }
	virtual uint16_t GetCHRPageSize() override { return 0x2000; }
	bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		SelectPRGPage(0, 0);
		SelectCHRPage(0, 0);
		RemoveRegisterRange(0xC000, 0xFFFF, MemoryOperation::Read);
	}

	uint8_t ReadRegister(uint16_t ) override
	{
		return 0x3A;
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
	}
};