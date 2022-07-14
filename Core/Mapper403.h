#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

// 89433

class Mapper403 : public BaseMapper
{
private:
	uint8_t _regs[4];

protected:
	virtual uint16_t GetPRGPageSize() override { return 0x4000; }
	virtual uint16_t GetCHRPageSize() override { return 0x2000; }
	virtual bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		memset(_regs, 0, sizeof(_regs));

		AddRegisterRange(0x4100, 0x5FFF, MemoryOperation::Write);

		// TODO: Hack for TetrisA from Tetris Family 19-in-1 NO 1683 to work.
		AddRegisterRange(0x6000, 0x7FFF, MemoryOperation::Read);

		UpdateState();
	}

	virtual void Reset(bool softReset) override
	{
		memset(_regs, 0, sizeof(_regs));

		UpdateState();
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_regs[0], _regs[1], _regs[2], _regs[3]);
	}

	virtual void UpdateState()
	{
		if (_regs[2] & 0x01) {
			SelectPRGPage(0, (_regs[0] >> 1) & 0x3F);
			SelectPRGPage(1, (_regs[0] >> 1) & 0x3F);
		} else {
			SelectPrgPage2x(0, (_regs[0] >> 1) & 0x3E);
		}

		SelectCHRPage(0, _regs[1] & 0x03);

		SetMirroringType((_regs[2] & 0x10) ? MirroringType::Horizontal : MirroringType::Vertical);
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		return InternalReadRam(addr);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if (addr < 0x8000) {
			if (addr & 0x0100) {
				_regs[(addr & 0x03)] = value;
				UpdateState();
			}
		} else {
			if (_regs[2] & 0x04) {
				_regs[1] = value;
				UpdateState();
			}
		}
	}
};
