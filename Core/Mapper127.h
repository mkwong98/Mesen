#pragma once
#include "stdafx.h"
#include "BaseMapper.h"
#include "CPU.h"

// Double Dragon II pirate

class Mapper127 : public BaseMapper
{
private:
	uint8_t _irqCounter;
	bool _irqEnabled;

protected:
	virtual uint16_t GetPRGPageSize() override { return 0x2000; }
	virtual uint16_t GetCHRPageSize() override { return 0x0400; }

	void InitMapper() override
	{
		_irqCounter = 0;
		_irqEnabled = false;

		SelectPRGPage(0, 0x0F);
		SelectPRGPage(1, 0x0F);
		SelectPRGPage(2, 0x0F);
		SelectPRGPage(3, 0x0F);
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_irqEnabled, _irqCounter);
	}

	void ProcessCpuClock() override
	{
		if(_irqEnabled && (!--_irqCounter)) {
			_console->GetCpu()->SetIrqSource(IRQSource::External);
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0x73) {
			case 0x00: SelectPRGPage(0, value & 0x0F); break;
			case 0x01: SelectPRGPage(1, value & 0x0F); break;
			case 0x02: SelectPRGPage(2, value & 0x0F); break;
			case 0x03: SelectPRGPage(3, (value & 0x03) | 0x0C); break;

			case 0x10: SelectCHRPage(0, value & 0x7F); break;
			case 0x11: SelectCHRPage(1, value & 0x7F); break;
			case 0x12: SelectCHRPage(2, value & 0x7F); break;
			case 0x13: SelectCHRPage(3, value & 0x7F); break;
			case 0x20: SelectCHRPage(4, value & 0x7F); break;
			case 0x21: SelectCHRPage(5, value & 0x7F); break;
			case 0x22: SelectCHRPage(6, value & 0x7F); break;
			case 0x23: SelectCHRPage(7, value & 0x7F); break;

			case 0x30:	case 0x31:	case 0x32:	case 0x33:
				_irqEnabled = true;
				break;

			case 0x40:	case 0x41:	case 0x42:	case 0x43:
				_irqEnabled = false;
				_irqCounter = 0;
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
				break;

			case 0x50: SetNametable(0, value & 0x01); break;
			case 0x51: SetNametable(1, value & 0x01); break;
			case 0x52: SetNametable(2, value & 0x01); break;
			case 0x53: SetNametable(3, value & 0x01); break;
		}
	}
};
