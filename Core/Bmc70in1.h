#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

class Bmc70in1 : public BaseMapper
{
private:
	uint8_t _bankMode;
	uint8_t _outerBank;
	uint8_t _prgReg;
	uint8_t _chrReg;
	uint8_t _dipSwitch;
	bool _useOuterBank;

protected:
	uint16_t GetPRGPageSize() { return 0x4000; }
	uint16_t GetCHRPageSize() { return 0x2000; }
	bool AllowRegisterRead() { return true; }

	void InitMapper() override
	{
		_prgReg = 0;
		_chrReg = 0;

		if(HasChrRom()) {
			_useOuterBank = false;
			_dipSwitch = 0x0C;
		} else {
			_useOuterBank = true;
			_dipSwitch = 0x05;
		}

		SelectCHRPage(0, 0);
		UpdateState();
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_bankMode, _outerBank, _prgReg, _chrReg, _dipSwitch);
	}

	void Reset(bool softReset) override
	{
		BaseMapper::Reset(softReset);

		_bankMode = 0;
		_outerBank = 0;
		_dipSwitch = (_dipSwitch + 1) & 0x0F;
	}

	void UpdateState()
	{
		switch(_bankMode) {
			case 0x00: case 0x10:
				SelectPRGPage(0, _outerBank | _prgReg);
				SelectPRGPage(1, _outerBank | 7);
				break;
			
			case 0x20:
				SelectPrgPage2x(0, (_outerBank | _prgReg) & 0xFE);
				break;
			
			case 0x30:
				SelectPRGPage(0, _outerBank | _prgReg);
				SelectPRGPage(1, _outerBank | _prgReg);
				break;
		}

		if(!_useOuterBank) {
			SelectCHRPage(0, _chrReg);
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		if(_bankMode == 0x10) {
			return InternalReadRam((addr & 0xFFF0) | _dipSwitch);
		} else {
			return InternalReadRam(addr);
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr & 0x4000) { 
			_bankMode = addr & 0x30;
			_prgReg = addr & 0x07;
		} else {
			SetMirroringType(addr & 0x20 ? MirroringType::Horizontal : MirroringType::Vertical);
			if(_useOuterBank) {
				_outerBank = (addr & 0x03) << 3;
			} else {
				_chrReg = addr & 0x07;
			}
		}

		UpdateState();
	}
};