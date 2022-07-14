#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

// J-2282 - 22-in-1 Olympic Games multicart

class Mapper402 : public BaseMapper
{
protected:
	virtual uint16_t GetPRGPageSize() override { return 0x2000; }
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
		if (addr & 0x0800)
			SetCpuMemoryMapping(0x6000, 0x7FFF, (((addr & 0x1F) << 1) | 0x03), PrgMemoryType::PrgRom);
		else {
			RemoveCpuMemoryMapping(0x6000, 0x7FFF);
		}

		if (addr & 0x0040) {
			SelectPrgPage2x(0, (addr & 0x1F) << 1);
			SelectPrgPage2x(1, (addr & 0x1F) << 1);
		} else {
			SelectPrgPage4x(0, (addr & 0x1E) << 1);
		}

		SelectCHRPage(0, 0);
		SetPpuMemoryMapping(0, 0x1FFF, 0, ChrMemoryType::Default, !(addr & 0x0400) ? MemoryAccessType::Read : MemoryAccessType::ReadWrite);

		SetMirroringType((addr & 0x80) ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};
