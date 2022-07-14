#pragma once
#include "stdafx.h"
#include "BaseMapper.h"
#include "CPU.h"

class Mapper357 : public BaseMapper
{
private:
	const uint8_t _dipswitchLut[4] = {
		0x00, 0x08, 0x10, 0x18
	};

	const uint8_t bankLut[2][8] = {
		{ 4, 3, 5, 3, 6, 3, 7, 3 },
		{ 1, 1, 5, 1, 4, 1, 5, 1 }
	};
	
	uint16_t _irqCounter;
	bool _irqEnabled;
	uint8_t _smb2jReg;
	uint8_t _unromReg;
	bool _prgSwap;
	uint8_t _dipswitch;

protected:
	uint16_t RegisterStartAddress() override { return 0x4020; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }

	uint16_t GetPRGPageSize() override { return 0x2000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	uint32_t GetDipSwitchCount() override { return 2; }

	void InitMapper() override
	{
		_irqCounter = 0;
		_irqEnabled = false;
		_unromReg = 0;
		_smb2jReg = 0;
		_prgSwap = 0;

		UpdateState();
	}

	void Reset(bool softReset) override
	{
		_dipswitch = _dipswitchLut[GetDipSwitches() & 0x03];

		_irqCounter = 0;
		_irqEnabled = false;

		UpdateState();
	}

	void UpdateState()
	{
		if(_dipswitch == 0) {
			SetCpuMemoryMapping(0x6000, 0x7FFF, _prgSwap ? 0x00 : 0x02, PrgMemoryType::PrgRom);

			SelectPRGPage(0, _prgSwap ? 0x00 : 0x01);
			SelectPRGPage(1, 0x00);
			SelectPRGPage(2, bankLut[_prgSwap][_smb2jReg]);
			SelectPRGPage(3, _prgSwap ? 0x08 : 0x0A);		
		} else {
			SelectPrgPage2x(0, (_dipswitch | _unromReg) << 1);
			SelectPrgPage2x(1, (_dipswitch | 0x07) << 1);
		}

		SelectCHRPage(0, 0);

		SetMirroringType((_dipswitch == 0x18) ? MirroringType::Horizontal : MirroringType::Vertical);
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_irqCounter, _irqEnabled, _unromReg, _smb2jReg, _prgSwap, _dipswitch);

		if(!saving) {
			UpdateState();
		}
	}

	void ProcessCpuClock() override
	{
		if(_irqEnabled) {
			_irqCounter++;
			if(_irqCounter == 0x1000) {
				_irqCounter = 0;
				_console->GetCpu()->SetIrqSource(IRQSource::External);
			}
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr & 0x8000) {
			_unromReg = value & 0x07;
			UpdateState();
		}
		if((addr & 0x71FF) == 0x4022) {
			_smb2jReg = value & 0x07;
			UpdateState();
		}
		if((addr & 0x71FF) == 0x4120) {
			_prgSwap = (value & 0x01) == 0x01;
			UpdateState();
		}
		if((addr & 0x71FF) == 0x4122) {
			_irqEnabled = (value & 0x01) == 0x01;
			_irqCounter = 0;
			_console->GetCpu()->ClearIrqSource(IRQSource::External);
		}
	}
};
