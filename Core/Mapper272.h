#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

class Mapper272 : public BaseMapper
{
private:
	uint8_t	_chrBanks[8];
	uint8_t _irqCounter;
	bool _irqEnabled;
	uint16_t _lastPpuAddr;

protected:
	virtual uint16_t GetPRGPageSize() override { return 0x2000; }
	virtual uint16_t GetCHRPageSize() override { return 0x0400; }

	void InitMapper() override
	{
		memset(_chrBanks, 0, sizeof(_chrBanks));

		_irqEnabled = false;
		_irqCounter = 0;

		SelectPRGPage(2, -2);
		SelectPRGPage(3, -1);
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		ArrayInfo<uint8_t> chrBanks = { _chrBanks, 8 };
		Stream(_irqEnabled, _irqCounter, chrBanks);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if((addr >= 0xB000) && (addr <= 0xE003)) {
			uint8_t index = ((((addr >> 12) & 0x07) - 3) << 1) + ((addr >> 1) & 0x01);
			bool lowBits = (addr & 0x01) == 0x00;
			if (lowBits) {
				_chrBanks[index] = (_chrBanks[index] & 0xF0) | (value & 0x0F);
			} else {
				_chrBanks[index] = (_chrBanks[index] & 0x0F) | (value << 4);
			}
			SelectCHRPage(index, _chrBanks[index]);
		} else {
			switch(addr & 0xF000) {
				case 0x8000:
					SelectPRGPage(0, value);
					break;

				case 0xA000:
					SelectPRGPage(1, value);
					break;

				case 0x9000:
					switch(value & 0x01) {
						case 0: SetMirroringType(MirroringType::Vertical); break;
						case 1: SetMirroringType(MirroringType::Horizontal); break;
					}
					break;
			}
		}

		switch(addr & 0xC00C) {
			case 0x8004:
				switch(value & 0x03) {
					case 2: SetMirroringType(MirroringType::ScreenAOnly); break;
					case 3: SetMirroringType(MirroringType::ScreenBOnly); break;
				}
				break;

			case 0x800C:
				_console->GetCpu()->SetIrqSource(IRQSource::External);
				break;

			case 0xC004:
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
				break;

			case 0xC008:
				_irqEnabled = true;
				break;

			case 0xC00C:
				_irqEnabled = false;
				_irqCounter = 0;
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
				break;
		}
	}

	void NotifyVRAMAddressChange(uint16_t addr) override
	{
		if ((_lastPpuAddr & 0x2000) && !(addr & 0x2000)) {
			if(_irqEnabled) {
				_irqCounter++;
				if(_irqCounter == 84) {
					_irqCounter = 0;
					_console->GetCpu()->SetIrqSource(IRQSource::External);
				}
			}
		}

		_lastPpuAddr = addr;
	}
};

