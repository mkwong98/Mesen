#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

// NC-20MB, 20-in-1 (CA-006) multicart.

class Mapper433 : public BaseMapper
{
protected:
	virtual uint16_t GetPRGPageSize() override { return 0x4000; }
	virtual uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		WriteRegister(0x8000, 0);
	}

	virtual void Reset(bool softReset) override
	{
		WriteRegister(0x8000, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectCHRPage(0, 0);
		if(value & 0x20) {
			SelectPRGPage(0, value & 0x1F);
			SelectPRGPage(1, value & 0x1F);
		} else {
			SelectPRGPage(0, value & 0x1E);
			SelectPRGPage(1, (value & 0x1E) + 1);
		}
		SetMirroringType(value & 0x40 ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};
