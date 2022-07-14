#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

class Mapper359 : public BaseMapper
{
protected:
	virtual uint16_t GetPRGPageSize() override { return 0x2000; }
	virtual uint16_t GetCHRPageSize() override { return 0x0400; }

	// $8000 - $8003
	uint8_t _prgBanks[4];

	// $9000
	uint8_t _outerPrgBank;

	// $9001
	uint8_t _prgMask;
	uint8_t _chrMask;

	// $9002
	uint8_t _mirroring;

	// $9003
	uint16_t _outerChrBank;

	// $A000 - $B003
	uint8_t _chrBanks[8];

	// IRQ
	bool _irqPpuA12;
	bool _irqAutoEnable;
	bool _irqEnabled;
	bool _irqReload;
	uint16_t _irqCounter;
	
	A12Watcher _a12Watcher;

	void InitMapper() override
	{
		_prgBanks[0] = -4;
		_prgBanks[1] = -3;
		_prgBanks[2] = -2;
		_prgBanks[3] = -1;

		_chrBanks[0] = 0x00;
		_chrBanks[1] = 0x01;
		_chrBanks[2] = 0x02;
		_chrBanks[3] = 0x03;
		_chrBanks[4] = 0x04;
		_chrBanks[5] = 0x05;
		_chrBanks[6] = 0x06;
		_chrBanks[7] = 0x07;

		_outerPrgBank = 0x00;
		_outerChrBank = 0x00;

		_prgMask = 0x3F;
		_chrMask = 0xFF;

		_mirroring = 0;

		_irqCounter = 0;
		_irqPpuA12 = false;
		_irqAutoEnable = false;
		_irqReload = false;
		_irqEnabled = false;

		UpdateState();
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);

		SnapshotInfo a12Watcher { &_a12Watcher };

		ArrayInfo<uint8_t> prgBanks { _prgBanks, 4 };
		ArrayInfo<uint8_t> chrBanks { _chrBanks, 8 };

		Stream(
			_outerPrgBank, _outerChrBank,
			_prgMask, _chrMask, _mirroring,
			_irqCounter, _irqPpuA12, _irqReload,
			_irqEnabled, _irqAutoEnable, 
			prgBanks, chrBanks, a12Watcher
		);

		if(!saving) {
			UpdateState();
		}
	}

	void UpdatePrgMapping()
	{
		SelectPRGPage(0, _outerPrgBank | (_prgBanks[0] & _prgMask));
		SelectPRGPage(1, _outerPrgBank | (_prgBanks[1] & _prgMask));
		SelectPRGPage(2, _outerPrgBank | (_prgBanks[2] & _prgMask));
		SelectPRGPage(3, _outerPrgBank | ((-1) & _prgMask));

		SetCpuMemoryMapping(0x6000, 0x7FFF, _outerPrgBank | (_prgBanks[3] & _prgMask), PrgMemoryType::PrgRom);
	}

	virtual void UpdateChrMapping()
	{
		if(HasChrRom()) {
			SelectCHRPage(0, _outerChrBank | (_chrBanks[0] & _chrMask));
			SelectCHRPage(1, _outerChrBank | (_chrBanks[1] & _chrMask));
			SelectCHRPage(2, _outerChrBank | (_chrBanks[2] & _chrMask));
			SelectCHRPage(3, _outerChrBank | (_chrBanks[3] & _chrMask));
			SelectCHRPage(4, _outerChrBank | (_chrBanks[4] & _chrMask));
			SelectCHRPage(5, _outerChrBank | (_chrBanks[5] & _chrMask));
			SelectCHRPage(6, _outerChrBank | (_chrBanks[6] & _chrMask));
			SelectCHRPage(7, _outerChrBank | (_chrBanks[7] & _chrMask));
		}
	}

	void UpdateMirroring()
	{
		switch(_mirroring) {
			case 0: SetMirroringType(MirroringType::Vertical); break;
			case 1: SetMirroringType(MirroringType::Horizontal); break;
			case 2: SetMirroringType(MirroringType::ScreenAOnly); break;
			case 3: SetMirroringType(MirroringType::ScreenBOnly); break;
		}
	}

	void UpdateState()
	{
		UpdatePrgMapping();
		UpdateChrMapping();
		UpdateMirroring();
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xF000) {
			case 0x8000:
				_prgBanks[addr & 0x03] = value;
				UpdatePrgMapping();
				break;

			case 0x9000:
				switch(addr & 0x03) {
					case 0:
						_outerPrgBank = (value & 0x38) << 1;
						UpdatePrgMapping();
						break;

					case 1:
						switch(value & 0x03) {
							case 0: _prgMask = 0x3F; break;
							case 1: _prgMask = 0x1F; break;
							case 2: _prgMask = 0x2F; break;
							case 3: _prgMask = 0x0F; break;
						}
						_chrMask = (value & 0x40) ? 0xFF : 0x7F;
						UpdatePrgMapping();
						UpdateChrMapping();
						break;

					case 2:
						_mirroring = value & 0x03;
						UpdateMirroring();
						break;
					
					case 3:
						_outerChrBank = value << 7;
						UpdateChrMapping();
						break;
				}
				break;

			case 0xA000:
				_chrBanks[0 | (addr & 0x03)] = value;
				UpdateChrMapping();
				break;

			case 0xB000:
				_chrBanks[4 | (addr & 0x03)] = value;
				UpdateChrMapping();
				break;
			
			case 0xC000:
				switch(addr & 0x03) {
					case 0:
						if(_irqAutoEnable) {
							_irqEnabled = false;
						}
						_irqCounter &= 0xFF00;
						_irqCounter |= value;
						break;

					case 1:
						if(_irqAutoEnable) {
							_irqEnabled = true;
						}
						_irqCounter &= 0x00FF;
						_irqCounter |= value << 8;
						_irqReload = true;
						break;

					case 2:
						_irqEnabled = (value & 0x01) == 0x01;
						_irqPpuA12 = (value & 0x02) == 0x02;
						_irqAutoEnable = (value & 0x04) == 0x04;
						break;

					case 3:
						_irqEnabled = (value & 0x01) == 0x01;
						break;
				}

				_console->GetCpu()->ClearIrqSource(IRQSource::External);
				break;
		}
	}

public:
	void ProcessCpuClock() override
	{
		if(_irqPpuA12)
			return;

		if(_irqEnabled && _irqCounter) {
			_irqCounter--;
			if(_irqCounter == 0) {
				_console->GetCpu()->SetIrqSource(IRQSource::External);
			}
		}
	}

	void NotifyVRAMAddressChange(uint16_t addr) override
	{
		uint8_t counter;

		if(!_irqPpuA12)
			return;

		switch(_a12Watcher.UpdateVramAddress(addr, _console->GetPpu()->GetFrameCycle())) {
			case A12StateChange::None:
			case A12StateChange::Fall:
				break;

			case A12StateChange::Rise:
				counter = _irqCounter & 0xFF;

				if(counter == 0 || _irqReload) {
					counter = (_irqCounter & 0xFF00) >> 8;
				} else {
					counter--;
				}
				
				if(counter == 0 && _irqEnabled) {
					_console->GetCpu()->SetIrqSource(IRQSource::External);
				}

				_irqReload =false;
				_irqCounter &= 0xFF00;
				_irqCounter |= counter;

				break;
		}
	}
};
