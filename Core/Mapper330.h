#pragma once
#include "stdafx.h"
#include "BaseMapper.h"
#include "Namco163Audio.h"
#include "BatteryManager.h"

// NES 2.0 Mapper 330 is used for a bootleg version of Namco's 三国志 II: 覇王の大陸 (Sangokushi II: Haō no Tairiku).
// http://forums.nesdev.org/viewtopic.php?f=9&t=17469

class Mapper330 : public BaseMapper
{
private:
	unique_ptr<Namco163Audio> _audio;
	uint16_t _irqCounter;

protected:
	virtual uint16_t GetPRGPageSize() override { return 0x2000; }
	virtual uint16_t GetCHRPageSize() override { return 0x400; }
	virtual bool AllowRegisterRead() override { return true; }
	
	void InitMapper() override
	{
		_audio.reset(new Namco163Audio(_console));
		_irqCounter = 0;

		AddRegisterRange(0x4800, 0x4FFF, MemoryOperation::Any);
		RemoveRegisterRange(0x5000, 0xFFFF, MemoryOperation::Read);

		SelectPRGPage(3, -1);
	}
	
	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);

		SnapshotInfo audio{ _audio.get() };
		Stream(_irqCounter, audio);
	}

	void LoadBattery() override
	{
		if(HasBattery()) {
			vector<uint8_t> batteryContent(_saveRamSize + Namco163Audio::AudioRamSize, 0);
			_console->GetBatteryManager()->LoadBattery(".sav", batteryContent.data(), (uint32_t)batteryContent.size());

			memcpy(_saveRam, batteryContent.data(), _saveRamSize);
			memcpy(_audio->GetInternalRam(), batteryContent.data()+_saveRamSize, Namco163Audio::AudioRamSize);
		}
	}

	void SaveBattery() override
	{
		if(HasBattery()) {
			vector<uint8_t> batteryContent(_saveRamSize + Namco163Audio::AudioRamSize, 0);
			memcpy(batteryContent.data(), _saveRam, _saveRamSize);
			memcpy(batteryContent.data() + _saveRamSize, _audio->GetInternalRam(), Namco163Audio::AudioRamSize);

			_console->GetBatteryManager()->SaveBattery(".sav", batteryContent.data(), (uint32_t)batteryContent.size());
		}
	}

	void ProcessCpuClock() override
	{
		if(_irqCounter & 0x8000 && (_irqCounter & 0x7FFF) != 0x7FFF) {
			_irqCounter++;
			if((_irqCounter & 0x7FFF) == 0x7FFF) {
				_console->GetCpu()->SetIrqSource(IRQSource::External);
			}
		}

		_audio->Clock();
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		if(addr & 0x800) {
			return _audio->ReadRegister(addr);
		}
		return BaseMapper::ReadRegister(addr);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xF000) {
			case 0x4000:
				if(addr & 0x800) {
					_audio->WriteRegister(addr, value);
				}
				break;

			case 0x8000:
			case 0x9000:
			case 0xA000:
			case 0xB000:
				if((addr & 0x400) && !(addr & 0x4000)) {
					if(addr & 0x2000) {
						_irqCounter = (_irqCounter & 0x00FF) | (value << 8);
						_console->GetCpu()->ClearIrqSource(IRQSource::External);
					} else {
						_irqCounter = (_irqCounter & 0xFF00) | value;
					}
				} else {
					uint8_t bankNumber = (addr - 0x8000) >> 11;
					SelectCHRPage(bankNumber, value);
				}
				break;

			case 0xC000:
			case 0xD000:
				if(!(addr & 0x400)) {
					uint8_t bankNumber = (addr - 0xC000) >> 11;
					SetNametable(bankNumber, value & 0x01);
				}
				break;

			case 0xE000:
			case 0xF000:
				if(((addr & 0xF000) == 0xF000) && (addr & 0x800)) {
					_audio->WriteRegister(addr, value);
				} else {
					if(!(addr & 0x400)) {
						uint8_t bankNumber = (addr - 0xE000) >> 11;
						SelectPRGPage(bankNumber, value);
					}
				}
				break;

			default:
				break;
		}
	}
};
