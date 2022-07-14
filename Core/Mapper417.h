#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

// Bootleg copy of Batman: The Video Game

class Mapper417 : public BaseMapper
{
private:
	uint16_t _irqCounter;
	bool _irqEnabled;

protected:
	virtual uint16_t GetPRGPageSize() override { return 0x2000; }
	virtual uint16_t GetCHRPageSize() override { return 0x0400; }

	void InitMapper() override
	{
		_irqEnabled = false;
		_irqCounter = 0;
		
		SelectPRGPage(3, -1);
	}

	virtual void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_irqEnabled, _irqCounter);
	}

	void ProcessCpuClock() override
	{
		_irqCounter++;
		if (_irqCounter & 0x400) {
			if (_irqEnabled) {
				_console->GetCpu()->SetIrqSource(IRQSource::External);
			}
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch ((addr >> 4) & 0x07) {
			case 0:
				SelectPRGPage(addr & 0x03, value);
				break;

			case 1:
				SelectCHRPage(addr & 0x03, value);
				break;

			case 2:
				SelectCHRPage((addr & 0x03) | 0x04, value);
				break;

			case 3:
				_irqEnabled = true;
				_irqCounter = 0;
				break;

			case 4:
				_irqEnabled = false;
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
				break;

			case 5:
				SetNametable(addr & 0x03, value & 0x01);
				break;
		}
	}
};
