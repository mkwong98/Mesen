#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

class Mapper502 : public BaseMapper
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
		uint16_t outerBank;
		uint16_t mask = (8 << ((_regs[1] >> 4) & 0x03)) - 1;

		SetCpuMemoryMapping(0x7000, 0x7FFF, 0, PrgMemoryType::WorkRam);
		if(_regs[1] & 0x06) {
			outerBank = (_regs[0] & 0x0F) << 2;
			SelectPrgPage2x(0, (outerBank + (_latch & (mask >> 1))) << 1);
		} else {
			outerBank = (_regs[0] & 0x0F) << 3;
			SelectPRGPage(0, outerBank + (_latch & mask));
			SelectPRGPage(1, outerBank + mask);
		}
		SelectCHRPage(0, 0);
		if(_regs[1] & 0x02) {
			SetMirroringType((_latch & 0x10) ? MirroringType::ScreenBOnly : MirroringType::ScreenAOnly);
		} else {
			SetMirroringType((_regs[1] & 0x01) ? MirroringType::Vertical : MirroringType::Horizontal);
		}
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
