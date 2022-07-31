#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

// YUNG-08

class Mapper368 : public BaseMapper
{
private:
	uint8_t _latch;

	bool _irqEnabled;
	uint16_t _irqCounter;

protected:
	uint16_t GetPRGPageSize() override { return 0x2000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }
	uint16_t RegisterStartAddress() override { return 0x4022; }
	uint16_t RegisterEndAddress() override { return 0x4FFF; }
	bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		_irqEnabled = false;
		_irqCounter = 0;
		_latch = 0;

		SetCpuMemoryMapping(0x6000, 0x7FFF, 0x02, PrgMemoryType::PrgRom);

		SelectPRGPage(0, 1);
		SelectPRGPage(1, 0);
		SelectPRGPage(3, 8);

		SelectCHRPage(0, 0);
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_latch, _irqEnabled, _irqCounter);
	}

	void ProcessCpuClock() override
	{
		if(_irqEnabled) {
			_irqCounter++;
			if((_irqCounter & 0xFFF) == 0) {
				_console->GetCpu()->SetIrqSource(IRQSource::External);
			}
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		if((addr & 0x1FF) == 0x122) {
			return (0x8A | (_latch & 0x35));
		}
		return InternalReadRam(addr);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0x1FF) {
			case 0x022:
				// bank order = { 4, 3, 5, 3, 6, 3, 7, 3 };
				SelectPRGPage(2, (value & 0x01) ? 0x03 : (0x04 | value >> 1));
				break;
			
			case 0x122:
				_latch = value;
				_irqEnabled = (value & 0x01) != 0;
				if(!_irqEnabled) {
					_irqCounter = 0;
					_console->GetCpu()->ClearIrqSource(IRQSource::External);
				}
				break;
		}
	}
};
