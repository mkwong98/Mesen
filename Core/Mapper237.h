#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

// Used by Teletubbies 420-in-1 multicart

class Mapper237 : public BaseMapper
{
private:
	uint8_t _prgBank;
	bool _type;
	bool _lock;

protected:
	virtual uint32_t GetDipSwitchCount() override { return 2; }
	virtual uint16_t GetPRGPageSize() override { return 0x4000; }
	virtual uint16_t GetCHRPageSize() override { return 0x2000; }
	virtual bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		_lock = false;
		WriteRegister(0x8000, 0);
	}

	void Reset(bool softReset) override
	{
		_lock = false;
		WriteRegister(0x8000, 0);
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_prgBank, _lock, _type);
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		if (_type)
			return GetDipSwitches();
		return InternalReadRam(addr);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(_lock) {
			_prgBank &= ~0x07;
			_prgBank |= value & 0x07;
		} else {
			_type = (addr & 0x01) == 0x01;
			_lock = (addr & 0x02) == 0x02;
			_prgBank = ((addr << 3) & 0x20) | (value & 0x1F);
		}

		SetMirroringType((value & 0x20) == 0x20 ? MirroringType::Horizontal : MirroringType::Vertical);

		if(value & 0x80) {
			if(value & 0x40) {
				SelectPrgPage2x(0, _prgBank & 0xFE);
			} else {
				SelectPRGPage(0, _prgBank);
				SelectPRGPage(1, _prgBank);		
			}
		} else {
			SelectPRGPage(0, _prgBank);
			SelectPRGPage(1, _prgBank | 0x07);
		}

		SelectCHRPage(0, 0);
	}
};
