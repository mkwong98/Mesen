#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

class Mapper428 : public BaseMapper
{
private:
	uint8_t _regs[4];
	uint8_t _chrLatch;

protected:
	uint32_t GetDipSwitchCount() override { return 2; }
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }
	bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		RemoveRegisterRange(0x8000, 0xFFFF, MemoryOperation::Read);

		_chrLatch = 0;
		memset(_regs, 0, sizeof(_regs));
		UpdateState();
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_regs[0], _regs[1], _regs[2], _regs[3], _chrLatch);

		if(!saving) {
			UpdateState();
		}
	}

	void Reset(bool softReset) override
	{
		BaseMapper::Reset(softReset);

		_chrLatch = 0;
		memset(_regs, 0, sizeof(_regs));
		UpdateState();
	}

	void UpdateState()
	{
		if (_regs[1] & 0x10) {
			SelectPrgPage2x(0, (_regs[1] >> 5) & 0xFE);
		} else {
			SelectPRGPage(0, _regs[1] >> 5);
			SelectPRGPage(1, _regs[1] >> 5);
		}
		SelectCHRPage(0, ((_regs[1] & 0x07) & ~(_regs[2] >> 6)) | (_chrLatch & (_regs[2] >> 6)));
		SetMirroringType((_regs[1] & 0x08) ? MirroringType::Horizontal : MirroringType::Vertical);
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		return (_console->GetMemoryManager()->GetOpenBus() & 0xFC) | (GetDipSwitches() & 0x03);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			_regs[addr & 0x03] = value;
		} else {
			_chrLatch = value;
		}
		UpdateState();
	}
};