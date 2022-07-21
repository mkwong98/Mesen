#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

class Mapper533 : public BaseMapper
{
private:
	uint8_t _latch;

protected:
	uint16_t GetPRGPageSize() override { return 0x8000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		RemoveRegisterRange(0x0000, 0xFFFF, MemoryOperation::Any);

		AddRegisterRange(0x8000, 0xFFFF, MemoryOperation::Write);
		AddRegisterRange(0xE000, 0xEFFF, MemoryOperation::Read);

		SelectPRGPage(0, 0);
		SelectCHRPage(0, 0);

		_latch = 0;

		WriteRegister(0x8000, 0);
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		return (_prgRom[0x6000 | (addr & 0xFFF)] & 0xF0) | _latch;
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		_latch = value >> 4;
		SelectCHRPage(0, _latch & 0x01);
	}
};
