#pragma once
#include "stdafx.h"
#include "BaseMapper.h"
#include "CPU.h"

//  N-32 cartridge conversion 4-in-1

class Mapper416 : public BaseMapper
{
private:
	uint8_t _regs[2];
	uint16_t _irqCounter;
	bool _irqEnabled;

protected:
	virtual uint16_t GetPRGPageSize() override { return 0x2000; }
	virtual uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		AddRegisterRange(0x4000, 0x5FFF, MemoryOperation::Write);
		RemoveRegisterRange(0xA000, 0xFFFF, MemoryOperation::Write);

		SetCpuMemoryMapping(0x6000, 0x7FFF, 0x07, PrgMemoryType::PrgRom);

		_regs[0] = _regs[1] = 0;
		_irqEnabled = false;
		_irqCounter = 0;

		UpdateState();
	}

	virtual void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_regs[0], _regs[1], _irqEnabled, _irqCounter);
	}

	void UpdateState()
	{
		if (_regs[1] & 0x08) {
			uint8_t page = ((_regs[1] >> 1) & 0x04) | ((_regs[1] >> 6) & 0x02) | ((_regs[1] >> 5) & 0x01);
			page <<= 1;
			if (_regs[1] & 0x80) {
				SelectPrgPage4x(0, page & 0xFE);
			} else {
				if (_regs[1] & 0x40) {
					SelectPrgPage2x(0, page);
					SelectPrgPage2x(1, page);
				} else {
					SelectPRGPage(0, page);
					SelectPRGPage(1, page);
					SelectPRGPage(2, page);
					SelectPRGPage(3, page);
				}
			}
		} else {
			SelectPRGPage(0, 0x00);
			SelectPRGPage(1, 0x01);
			SelectPRGPage(2, (_regs[0] & 0x08) | ((_regs[0] << 2) & 0x04) | ((_regs[0] >> 1) & 0x03));
			SelectPRGPage(3, 0x03);
		}

		SelectCHRPage(0x0, _regs[1] >> 1 & 0x03);

		SetMirroringType((_regs[1] & 0x04) ? MirroringType::Horizontal : MirroringType::Vertical);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if (addr < 0x8000) { // 0x4000-0x5FFF
			if (addr & 0x20) {
				if (addr & 0x100) {
					_irqEnabled = (value & 0x01) == 0x01;
				} else {
					_regs[0] = value;
				}
			}
		} else { // 0x8000 - 0x9FFF
			_regs[1] = value;
		}

		UpdateState();
	}

	void ProcessCpuClock() override
	{
		if (_irqEnabled) {
			_irqCounter++;
			if (_irqCounter & 0x1000) {
				_console->GetCpu()->SetIrqSource(IRQSource::External);
			}
		} else {
			_irqCounter = 0;
			_console->GetCpu()->ClearIrqSource(IRQSource::External);
		}
	}
};
