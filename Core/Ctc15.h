#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

class Ctc15 : public BaseMapper
{
private:
	uint8_t _latch;
	uint16_t _irqCounter;
	bool _irqEnabled;

protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		AddRegisterRange(0x4800, 0x4FFF, MemoryOperation::Write);
		AddRegisterRange(0x5000, 0x57FF, MemoryOperation::Write);
		RemoveRegisterRange(0x8000, 0xFFFF, MemoryOperation::Write);

		_latch = 0x07;
		_irqCounter = 0;
		_irqEnabled = false;

		SelectCHRPage(0, 0);
		SelectPRGPage(0, _latch ^ 0x05);
		SelectPRGPage(1, 0x03);
		SetCpuMemoryMapping(0x6000, 0x7FFF, 0, PrgMemoryType::WorkRam);
	}

	virtual void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_latch, _irqCounter, _irqEnabled);
	}

	void ProcessCpuClock() override
	{
		if (_irqEnabled) {
			if (_irqCounter == 23680) {
				_console->GetCpu()->SetIrqSource(IRQSource::External);
			} else if (_irqCounter == 24320) {
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
			}
			_irqCounter++;
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if (addr < 0x5000) {
			_latch = ((addr >> 3) & 0x04) | ((addr >> 2) & 0x03);
			_irqEnabled = (addr & 0x04) != 0x04;

			if (!_irqEnabled) {
				_irqCounter = 0;
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
			}
		} else {
			SelectPRGPage(0, _latch ^ 0x05);
		}
	}
};
