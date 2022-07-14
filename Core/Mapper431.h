#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

// Realtec GN-91B multicart.

class Mapper431 : public BaseMapper
{
private:
	uint8_t _regs[2];

protected:
	virtual uint16_t GetPRGPageSize() override { return 0x4000; }
	virtual uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		_regs[0] = _regs[1] = 0;
		SelectCHRPage(0, 0);
		UpdateState();
	}

	virtual void Reset(bool softReset) override
	{
		_regs[0] = _regs[1] = 0;
		UpdateState();
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_regs[0], _regs[1]);
	}

	void UpdateState()
	{
		SelectPRGPage(0, ((_regs[0] >> 2) & 0x08) | (_regs[1] & 0x07));
		SelectPRGPage(1, ((_regs[0] >> 2) & 0x08) | 0x07);
		SetMirroringType((_regs[0] & 0x01) ? MirroringType::Horizontal : MirroringType::Vertical);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if (addr < 0xC000) {
			_regs[0] = value;
		} else {
			_regs[1] = value;
		}
		UpdateState();
	}
};
