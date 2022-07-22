#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

class Mapper500 : public BaseMapper
{
private:
	uint8_t _regs[2];
	uint8_t _latch;

protected:
	virtual uint16_t GetPRGPageSize() override { return 0x4000; }
	virtual uint16_t GetCHRPageSize() override { return 0x2000; }

	uint32_t GetWorkRamSize() override { return 0x2000; }
	uint32_t GetWorkRamPageSize() override { return 0x1000; }

	void InitMapper() override
	{
		_regs[0] = 0;
		_regs[1] = 0;
		_latch = 0;
		AddRegisterRange(0x6000, 0x6FFF, MemoryOperation::Write);

		UpdateState();
	}

	void Reset(bool softReset) override
	{
		if(!softReset) {
			_latch = 0;
		}

		_regs[0] = 0;
		_regs[1] = 0;

		UpdateState();
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_regs[0], _regs[1], _latch);
	}

	void UpdateState()
	{
		uint16_t outerBank = (_regs[0] & 0x0F) << 3;

		SetCpuMemoryMapping(0x7000, 0x7FFF, 0, PrgMemoryType::WorkRam);
		SelectPRGPage(0, outerBank | (_latch & 0x07));
		SelectPRGPage(1, outerBank | 0x07);
		SelectCHRPage(0, 0);
		SetMirroringType((_regs[1] & 0x01) ? MirroringType::Vertical : MirroringType::Horizontal);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			if((_regs[1] & 0x80) == 0) {
				_regs[addr & 0x01] = value;
			}
		} else {
			_latch = value;
		}

		UpdateState();
	}
};
