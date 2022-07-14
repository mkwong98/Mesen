#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

// retroUSB DPCMcart

class Mapper409 : public BaseMapper
{
protected:
	virtual uint16_t GetPRGPageSize() override { return 0x4000; }
	virtual uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		RemoveRegisterRange(0x0000, 0xFFFF, MemoryOperation::Any);
		AddRegisterRange(0xC000, 0xCFFF, MemoryOperation::Write);

		SelectPRGPage(1, -1);
		SelectCHRPage(0, 0);

		WriteRegister(0xC000, 0);
	}

	virtual void Reset(bool softReset) override
	{
		WriteRegister(0xC000, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectPRGPage(0, addr & 0x0FFF);
	}
};
