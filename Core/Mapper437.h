#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

// NTDEC TH2348 8-in-1 multicart

class Mapper437 : public BaseMapper
{
private:
	uint8_t _regs[2];

protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		AddRegisterRange(0x5000, 0x5FFF, MemoryOperation::Write);

		WriteRegister(0x5000, 0);
		WriteRegister(0x8000, 0);
	}

	void Reset(bool softReset) override
	{
		WriteRegister(0x5000, 0);
		WriteRegister(0x8000, 0);
	}

	void UpdateState()
	{
		SelectCHRPage(0, 0);
		SelectPRGPage(0, _regs[0] << 3 | (_regs[1] & 0x07));
		SelectPRGPage(1, _regs[0] << 3 | 0x07);
		SetMirroringType((_regs[0] & 0x08) ? MirroringType::Horizontal : MirroringType::Vertical);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			_regs[0] = addr & 0x0F;
		} else {
			_regs[1] = value;
		}
		UpdateState();
	}
};
