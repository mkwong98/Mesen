#pragma once
#include "stdafx.h"
#include "Fk23C.h"

class Mapper523 : public Fk23C
{
protected:
	virtual uint32_t GetWorkRamSize() override { return 0x8000; }
	virtual uint32_t GetWorkRamPageSize() override { return 0x2000; }

	void UpdateMirroring() override
	{
		// mirroring is hardwired
	}

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		if(~slot & 0x01) {
			Fk23C::SelectChrPage2x(slot >> 1, page << 1, memoryType);
		}
	}
};
