#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

class BmcS2009 : public BaseMapper
{
private:
	uint8_t _prgBank;
	uint8_t _outerBank;

protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		AddRegisterRange(0x6000, 0x7FFF, MemoryOperation::Write);
		UpdateState();
	}

	void Reset(bool softReset) override
	{
		_prgBank = 0;
		_outerBank = 0;
		UpdateState();
	}

	void UpdateState()
	{
		SelectPRGPage(0, (_outerBank << 3) | (_prgBank & 0x07));
		SelectPRGPage(1, (_outerBank << 3) | 0x07);
		SelectCHRPage(0, 0);
		SetMirroringType(((_outerBank & 0x20) == 0x20) ? MirroringType::Vertical : MirroringType::Horizontal);
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_prgBank, _outerBank);
	}

	// TODO: suppose to have bus conflicts but why it crash if done so?

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			_outerBank = value;
		} else {
			_prgBank = value;
		}
		UpdateState();
	}
};