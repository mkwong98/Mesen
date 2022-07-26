#pragma once
#include "stdafx.h"
#include "BaseMapper.h"
#include "CPU.h"
#include "A12Watcher.h"

class Mapper413 : public BaseMapper
{
private:
	uint32_t _serialAddress;
	uint8_t _serialControl;

	uint8_t _irqCounter;
	uint8_t _irqReloadValue;
	bool _irqEnabled;

	A12Watcher _a12Watcher;

protected:
	virtual uint16_t GetPRGPageSize() override { return 0x1000; }
	virtual uint16_t GetCHRPageSize() override { return 0x1000; }
	virtual bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		_serialAddress = 0;
		_serialControl = 0;

		_irqCounter = 0;
		_irqReloadValue = 0;
		_irqEnabled = false;

		SetCpuMemoryMapping(0x5000, 0x5FFF, 1, PrgMemoryType::PrgRom);
		SetCpuMemoryMapping(0x6000, 0x6FFF, 0, PrgMemoryType::PrgRom);
		SetCpuMemoryMapping(0x7000, 0x7FFF, 1, PrgMemoryType::PrgRom);

		SelectPrgPage2x(0, 0);
		SelectPrgPage2x(1, 0);
		SelectPRGPage(5, 0x07);
		SelectPRGPage(6, 0x08);
		SelectPRGPage(7, 0x09);

		SelectCHRPage(0, 0);
		SelectCHRPage(1, -3);

		AddRegisterRange(0x4800, 0x4FFF, MemoryOperation::Read);
		RemoveRegisterRange(0x8000, 0xBFFF, MemoryOperation::Read);
		RemoveRegisterRange(0xD000, 0xFFFF, MemoryOperation::Read);
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_serialAddress, _serialControl,
			_irqCounter, _irqReloadValue, _irqEnabled);
	}

	void NotifyVRAMAddressChange(uint16_t addr) override
	{
		switch(_a12Watcher.UpdateVramAddress(addr, _console->GetPpu()->GetFrameCycle())) {
			case A12StateChange::None:
			case A12StateChange::Fall:
				break;

			case A12StateChange::Rise:
				if(_irqCounter == 0) {
					_irqCounter = _irqReloadValue;
				} else {
					_irqCounter--;
				}
				if (_irqCounter == 0 && _irqEnabled) {
					_console->GetCpu()->SetIrqSource(IRQSource::External);
				}
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		if(_serialControl & 0x02) {
			return (_miscRom[_serialAddress++ & (_miscRomSize - 1)]);
		} else {
			return (_miscRom[_serialAddress & (_miscRomSize - 1)]);
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch (addr & 0xF000) {
			case 0x8000:
				_irqReloadValue = value;
				break;

			case 0x9000:
				_irqCounter = 0;
				break;

			case 0xA000:
			case 0xB000:
				_irqEnabled = (addr & 0x1000) != 0;
				if (_irqEnabled == 0) {
					_console->GetCpu()->ClearIrqSource(IRQSource::External);
				}
				break;

			case 0xC000:
				_serialAddress = (_serialAddress << 1) | (value >> 7);
				break;

			case 0xD000:
				_serialControl = value;
				break;

			case 0xE000:
			case 0xF000:
				switch(value >> 6) {
					case 0:
						value <<= 1;
						SetCpuMemoryMapping(0x6000, 0x7FFF, value, PrgMemoryType::PrgRom);
						break;

					case 1:
						value <<= 1;
						SelectPRGPage(0, value);
						SelectPRGPage(1, value + 1);
						break;

					case 2:
						value <<= 1;
						SelectPRGPage(2, value);
						SelectPRGPage(3, value + 1);
						break;

					case 3:
						SelectCHRPage(0, value);
						break;
				}
				break;
		}
	}
};