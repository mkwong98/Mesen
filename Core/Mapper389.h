#pragma once
#include "BaseMapper.h"

class Mapper389 : public BaseMapper
{
private:
	uint8_t _regs[4];

protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		memset(_regs, 0, sizeof(_regs));

		UpdateState();
	}

	void Reset(bool softreset) override
	{
		memset(_regs, 0, sizeof(_regs));

		UpdateState();
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_regs[0], _regs[1], _regs[2], _regs[3]);
	}

	void UpdateState()
	{
		if (_regs[1] & 0x02) {
			SelectPRGPage(0, (_regs[0] >> 2) | ((_regs[2] >> 2) & 0x03));
			SelectPRGPage(1, (_regs[0] >> 2) | 0x03);
		} else
			SelectPrgPage2x(0, _regs[0] >> 2);

		SelectCHRPage(0, ((_regs[1] >> 1) & 0xFC) | (_regs[2] & 0x03));

		SetMirroringType((_regs[0] & 0x01) ? MirroringType::Horizontal : MirroringType::Vertical);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if (addr <= 0x8FFF) {
			_regs[0] = addr & 0xFF;
		} else if (addr <= 0x9FFF) {
			_regs[1] = addr & 0xFF;
		} else {
			_regs[2] = addr & 0xFF;
		}

		UpdateState();
	}
};