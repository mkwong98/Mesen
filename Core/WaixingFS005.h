#pragma once
#include "stdafx.h"
#include "Fk23C.h"

class WaixingFS005 : public Fk23C
{
protected:
	virtual uint32_t GetChrRamSize() override { return 0x2000; }
	virtual uint16_t GetChrRamPageSize() override { return 0x400; }

	virtual uint32_t GetWorkRamSize() override { return 0x8000; }
	virtual uint32_t GetWorkRamPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		_romInfo.SubMapperID = 2;

		Fk23C::InitMapper();
	}
};
