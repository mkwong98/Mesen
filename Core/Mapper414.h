#pragma once
#include "stdafx.h"
#include "BaseMapper.h"
#include "CPU.h"

class Mapper414 : public BaseMapper
{
private:
	const uint16_t _dpswlut[5] = { 0x0010, 0x0000, 0x0030, 0x0070, 0x00F0 };
	uint16_t _dipSwitch;
	uint16_t _latch;

protected:
	virtual uint16_t GetPRGPageSize() override { return 0x4000; }
	virtual uint16_t GetCHRPageSize() override { return 0x2000; }
	virtual bool AllowRegisterRead() override { return true; }
	virtual bool HasBusConflicts() override { return true; }
	virtual uint32_t GetDipSwitchCount() override { return 3; }

	void InitMapper() override
	{
		uint32_t index = GetDipSwitches();
		if (index > 4)
			index = 0;
		_dipSwitch = _dpswlut[index];
		WriteRegister(0x8000, 0);
	}

	void Reset(bool softReset) override
	{
		WriteRegister(0x8000, 0);
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_latch);
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		if (addr >= 0xC000) {
			if (!(_latch & 0x100) && (_latch & _dipSwitch)) {
				return _console->GetMemoryManager()->GetOpenBus();
			}
		}
		return InternalReadRam(addr);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		_latch = addr;

		if (_latch & 0x2000)
			SelectPrgPage2x(0, (_latch >> 1) & 0xFE);
		else {
			SelectPRGPage(0, _latch >> 1);
			SelectPRGPage(1, _latch >> 1);
		}
		SelectCHRPage(0, value & 0x03);
		SetMirroringType((_latch & 0x01) ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};