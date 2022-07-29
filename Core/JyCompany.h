#pragma once
#include "stdafx.h"
#include "BaseMapper.h"
#include "CPU.h"
#include "MemoryManager.h"

class JyCompany : public BaseMapper
{
private:
	enum class JyIrqSource
	{
		CpuClock = 0,
		PpuA12Rise = 1,
		PpuRead = 2,
		CpuWrite = 3
	};

	uint8_t _prgRegs[4];
	uint8_t _chrLowRegs[8];
	uint8_t _chrHighRegs[8];
	uint8_t _chrLatch[2];
	
	uint8_t _prgMode;
	bool _enablePrgAt6000;

	uint8_t _chrMode;
	bool _mirrorChr;
	uint8_t _outerBank; // $D003

	uint8_t _mirroringReg;
	bool _extendedMirroring;

	bool _advancedNtControl;
	bool _disableNtRam;

	uint8_t _ntRamSelectBit;
	bool _chrWriteEnabled;

	uint8_t _ntLowRegs[4];
	uint8_t _ntHighRegs[4];

	bool _irqEnabled;
	JyIrqSource _irqSource;
	uint8_t _irqCountDirection;
	bool _irqFunkyMode;
	uint8_t _irqFunkyModeReg;
	bool _irqSmallPrescaler;
	uint8_t _irqPrescaler;
	uint8_t _irqCounter;
	uint8_t _irqXorReg;

	uint8_t _multiplyValue1;
	uint8_t _multiplyValue2;
	uint8_t _accumulator;
	uint8_t _regRamValue;

	uint16_t _lastPpuAddr;

	bool _inhibitExtendedMirroring;

protected:
	virtual uint32_t GetDipSwitchCount() override { return 2; }
	virtual uint16_t GetPRGPageSize() override { return 0x2000; }
	virtual uint16_t GetCHRPageSize() override { return 0x0400; }
	virtual bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		RemoveRegisterRange(0x8000, 0xFFFF, MemoryOperation::Read);
		AddRegisterRange(0x5000, 0x5FFF, MemoryOperation::Any);

		_chrLatch[0] = 0;
		_chrLatch[1] = 4;

		memset(_prgRegs, 0, sizeof(_prgRegs));
		memset(_chrLowRegs, 0, sizeof(_chrLowRegs));
		memset(_chrHighRegs, 0, sizeof(_chrHighRegs));

		_prgMode = 0;
		_enablePrgAt6000 = false;

		_chrMode = 0;
		_mirrorChr = false;

		_outerBank = 0;

		_mirroringReg = 0;
		_extendedMirroring = false;
		_advancedNtControl = false;
		_disableNtRam = false;

		_ntRamSelectBit = 0;
		_chrWriteEnabled = false;
		memset(_ntLowRegs, 0, sizeof(_ntLowRegs));
		memset(_ntHighRegs, 0, sizeof(_ntHighRegs));

		_irqEnabled = false;
		_irqSource = JyIrqSource::CpuClock;
		_lastPpuAddr = 0;
		_irqCountDirection = 0;
		_irqFunkyMode = false;
		_irqFunkyModeReg = 0;
		_irqSmallPrescaler = false;
		_irqPrescaler = 0;
		_irqCounter = 0;
		_irqXorReg = 0;

		_multiplyValue1 = 0;
		_multiplyValue2 = 0;
		_accumulator = 0;
		_regRamValue = 0;

		if(_romInfo.MapperID == 90 || _romInfo.MapperID == 388) {
			_inhibitExtendedMirroring = true;
		}

		UpdateState();
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);

		ArrayInfo<uint8_t> prgRegs{ _prgRegs, 4 };
		ArrayInfo<uint8_t> chrLowRegs{ _chrLowRegs, 8 };
		ArrayInfo<uint8_t> chrHighRegs{ _chrHighRegs, 8 };
		ArrayInfo<uint8_t> ntLowRegs{ _ntLowRegs, 4 };
		ArrayInfo<uint8_t> ntHighRegs{ _ntHighRegs, 4 };

		Stream(_chrLatch[0], _chrLatch[1], _prgMode, _enablePrgAt6000, _chrMode, _mirrorChr, _outerBank, _mirroringReg, _advancedNtControl,
			_disableNtRam, _ntRamSelectBit, _irqEnabled, _irqSource, _lastPpuAddr, _irqCountDirection, _irqFunkyMode, _irqFunkyModeReg, _irqSmallPrescaler,
			_irqPrescaler, _irqCounter, _irqXorReg, _multiplyValue1, _multiplyValue2, _regRamValue, _accumulator, _chrWriteEnabled, _extendedMirroring,
			prgRegs, chrLowRegs, chrHighRegs, ntLowRegs, ntHighRegs
			);

		if(!saving) {
			UpdateState();
		}
	}

	void UpdateState()
	{
		UpdatePrgState();
		UpdateChrState();
		UpdateMirroringState();
	}

	uint8_t InvertPrgBits(uint8_t value)
	{
		return (value & 0x01) << 6 | (value & 0x02) << 4 | (value & 0x04) << 2 | (value & 0x10) >> 2 | (value & 0x20) >> 4 | (value & 0x40) >> 6;
	}

	void UpdatePrgState()
	{
		uint8_t lastBank = (_prgMode & 0x04) ? _prgRegs[3] : 0x3F;
		uint8_t wramBank = 0;
		uint8_t prgMask = 0;
		uint8_t prgBase = 0;

		switch(_romInfo.MapperID) {
			case 281:
				prgMask = 0x1F;
				prgBase = (_outerBank & 0x03) << 5;
				break;
			
			case 282:
			case 358:
				prgMask = 0x1F;
				prgBase = (_outerBank << 4) & ~prgMask;
				break;

			case 295:
				prgMask = 0x0F;
				prgBase = (_outerBank & 0x07) << 4;
				break;

			case 386:
				prgMask = 0x1F;
				prgBase = (((_outerBank & 0x02) >> 1) | ((_outerBank & 0x08) >> 2)) << 5;
				break;

			case 387:
				prgMask = 0x0F;
				prgBase = (((_outerBank & 0x02) >> 1) | ((_outerBank & 0x08) >> 2)) << 4;
				break;
			
			case 388:
				prgMask = 0x1F;
				prgBase = (_outerBank & 0x0C) << 3;
				break;
			
			case 397:
				prgMask = 0x1F;
				prgBase = (_outerBank & 0x06) << 4;
				break;
			
			case 421:
				if(_outerBank & 0x04) {
					prgMask = 0x3F;
					prgBase = (_outerBank & 0x0C) << 4;
				} else {
					prgMask = 0x1F;
					prgBase = (_outerBank & 0x0E) << 4;
				}
				break;

			default: // Mapper 35/90/209/211
				prgMask = 0x3F;
				prgBase = (_outerBank << 5) & ~prgMask;
				break;
		}

		switch(_prgMode & 0x03) {
			case 0:
				SelectPrgPage4x(0, ((lastBank & (prgMask >> 2)) | (prgBase >> 2)) << 2);
				wramBank = (_prgRegs[3] * 4 + 3);
				break;

			case 1:
				SelectPrgPage2x(0, ((_prgRegs[1] & (prgMask >> 1)) | (prgBase >> 1)) << 1);
				SelectPrgPage2x(1, ((lastBank    & (prgMask >> 1)) | (prgBase >> 1)) << 1);
				wramBank = (_prgRegs[3] * 2 + 1);
				break;

			case 2:
				SelectPRGPage(0, (_prgRegs[0] & prgMask) | prgBase);
				SelectPRGPage(1, (_prgRegs[1] & prgMask) | prgBase);
				SelectPRGPage(2, (_prgRegs[2] & prgMask) | prgBase);
				SelectPRGPage(3, (lastBank    & prgMask) | prgBase);
				wramBank = _prgRegs[3];
				break;

			case 3:
				SelectPRGPage(0, (InvertPrgBits(_prgRegs[0]) & prgMask) | prgBase);
				SelectPRGPage(1, (InvertPrgBits(_prgRegs[1]) & prgMask) | prgBase);
				SelectPRGPage(2, (InvertPrgBits(_prgRegs[2]) & prgMask) | prgBase);
				SelectPRGPage(3, (InvertPrgBits(lastBank)    & prgMask) | prgBase);
				wramBank = _prgRegs[3];
				break;
		}

		if(_enablePrgAt6000) {
			SetCpuMemoryMapping(0x6000, 0x7FFF, (wramBank & prgMask) | prgBase, PrgMemoryType::PrgRom);
		} else {
			RemoveCpuMemoryMapping(0x6000, 0x7FFF);
		}
	}

	uint16_t GetChrReg(int index)
	{
		return _chrLowRegs[index] | (_chrHighRegs[index] << 8);
	}

	void GetChrSetup(uint16_t *mask, uint16_t *base)
	{
		switch(_romInfo.MapperID) {
			case 281:
				(*mask) = 0xFF;
				(*base) = (_outerBank & 0x03) << 8;
				break;
			
			case 295:
			case 397:
				(*mask) = 0x7F;
				(*base) = (_outerBank & 0x07) << 7;
				break;

			case 358:
			case 386:
			case 387:
				if(_outerBank & 0x20) {
					(*mask) = 0x1FF;
					(*base) = (_outerBank & 0x04) << 7;
				} else {
					(*mask) = 0x0FF;
					(*base) = (((_outerBank & 0x04) >> 1) | (_outerBank & 0x01)) << 8;
				}
				break;
			
			case 388:
				if(_outerBank & 0x20) {
					(*mask) = 0x1FF;
					(*base) = (_outerBank & 0x02) << 8;
				} else {
					(*mask) = 0x0FF;
					(*base) = (_outerBank & 0x03) << 8;
				}
				break;

			case 421:
				(*mask) = 0x1FF;
				(*base) = (_outerBank & 0x03) << 8;
				break;

			default: // Mapper 35/90/209/211/282
				if(_outerBank & 0x20) {
					(*mask) = 0x1FF;
					(*base) = (_outerBank & 0x18) << 6;
				} else  {
					(*mask) = 0xFF;
					(*base) = (_outerBank & 0x18) << 6 | (_outerBank & 0x01) << 8;
				}
				break;
		}
	}

	void UpdateChrState()
	{
		uint16_t chrMask = 0;
		uint16_t chrBase = 0;

		GetChrSetup(&chrMask, &chrBase);

		switch(_chrMode) {
			case 0: 
				SelectChrPage8x(0, ((GetChrReg(0) & (chrMask >> 3)) | (chrBase >> 3)) << 3);
				break;

			case 1: 
				SelectChrPage4x(0, ((GetChrReg(_chrLatch[0]) & (chrMask >> 2)) | (chrBase >> 2)) << 2);
				SelectChrPage4x(1, ((GetChrReg(_chrLatch[1]) & (chrMask >> 2)) | (chrBase >> 2)) << 2);
				break;

			case 2: 
				SelectChrPage2x(0, ((GetChrReg(0) & (chrMask >> 1)) | (chrBase >> 1)) << 1);
				SelectChrPage2x(1, ((GetChrReg(2) & (chrMask >> 1)) | (chrBase >> 1)) << 1);
				SelectChrPage2x(2, ((GetChrReg(4) & (chrMask >> 1)) | (chrBase >> 1)) << 1);
				SelectChrPage2x(3, ((GetChrReg(6) & (chrMask >> 1)) | (chrBase >> 1)) << 1);
				break;

			case 3:
				for(int i = 0; i < 8; i++) {
					SelectCHRPage(i, (GetChrReg(i) & chrMask) | chrBase);
				}
				break;
		}

		SetPpuMemoryMapping(0, 0x1FFF, 0, ChrMemoryType::ChrRam, _chrWriteEnabled ? MemoryAccessType::ReadWrite : MemoryAccessType::Read);
	}

	void UpdateMirroringState()
	{
		if(_advancedNtControl || _extendedMirroring) {
			for(int i = 0; i < 4; i++) {
				SetNametable(i, _ntLowRegs[i] & 0x01);
			}
		} else {
			switch(_mirroringReg) {
				case 0: SetMirroringType(MirroringType::Vertical); break;
				case 1: SetMirroringType(MirroringType::Horizontal); break;
				case 2: SetMirroringType(MirroringType::ScreenAOnly); break;
				case 3: SetMirroringType(MirroringType::ScreenBOnly); break;
			}
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		uint8_t openBus = _console->GetMemoryManager()->GetOpenBus();

		if((addr != 0x5800) && ((addr & 0x3FF) == 0)) {
			return ((GetDipSwitches() << 6) | (openBus & 0x3F));
		}

		switch(addr & 0xF803) {
			case 0x5800: return (_multiplyValue1 * _multiplyValue2) & 0xFF;
			case 0x5801: return ((_multiplyValue1 * _multiplyValue2) >> 8) & 0xFF;
			case 0x5802: return _accumulator;
			case 0x5803: return _regRamValue;
		}

		return openBus;
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			switch(addr & 0xF803) {
				case 0x5800: _multiplyValue1 = value; break;
				case 0x5801: _multiplyValue2 = value; break;
				case 0x5802: _accumulator += value; break;
				case 0x5803:
					_regRamValue = value;
					_accumulator = 0;
					break;
			}
		} else if((addr >= 0xC000) && (addr < 0xD000)) {
			switch(addr & 0xF007) {
				case 0xC000:
					if(value & 0x01) {
						_irqEnabled = true;
					} else {
						_irqEnabled = false;
						_irqPrescaler = 0;
						_console->GetCpu()->ClearIrqSource(IRQSource::External);
					}
					break;

				case 0xC001:
					_irqCountDirection = (value >> 6) & 0x03;
					_irqFunkyMode = (value & 0x08) == 0x08;
					_irqSmallPrescaler = ((value >> 2) & 0x01) == 0x01;
					_irqSource = (JyIrqSource)(value & 0x03);
					break;

				case 0xC002:
					_irqEnabled = false;
					_irqPrescaler = 0;
					_console->GetCpu()->ClearIrqSource(IRQSource::External);
					break;

				case 0xC003: _irqEnabled = true; break;
				case 0xC004: _irqPrescaler = value ^ _irqXorReg; break;
				case 0xC005: _irqCounter = value ^ _irqXorReg; break;
				case 0xC006: _irqXorReg = value; break;
				case 0xC007: _irqFunkyModeReg = value; break;
			}
		// these registers extend only through addressing ranges $x000 - $x7FF
		} else if ((addr >= 0x9000) && (addr < 0xC000)) {
			switch(addr & 0xF807) {
				case 0x9000: case 0x9001: case 0x9002: case 0x9003:
				case 0x9004: case 0x9005: case 0x9006: case 0x9007:
					_chrLowRegs[addr & 0x07] = value;
					break;

				case 0xA000: case 0xA001: case 0xA002: case 0xA003:
				case 0xA004: case 0xA005: case 0xA006: case 0xA007:
					_chrHighRegs[addr & 0x07] = value;
					break;

				case 0xB000: case 0xB001: case 0xB002: case 0xB003:
					_ntLowRegs[addr & 0x03] = value;
					break;
				
				case 0xB004: case 0xB005: case 0xB006: case 0xB007:
					_ntHighRegs[addr & 0x03] = value;
					break;
			}
		} else {
			switch(addr & 0xF803) {
				case 0x8000: case 0x8001: case 0x8002: case 0x8003:
					_prgRegs[addr & 0x03] = value & 0x7F;
					break;

				case 0xD000:
					if(_inhibitExtendedMirroring) {
						value &= ~0x20;
					}

					_prgMode = value & 0x07;
					_chrMode = (value >> 3) & 0x03;
					_advancedNtControl = (value & 0x20) == 0x20;
					_disableNtRam = (value & 0x40) == 0x40;
					_enablePrgAt6000 = (value & 0x80) == 0x80;
					break;

				case 0xD001:
					if(_inhibitExtendedMirroring) {
						value &= ~0x08;
					}

					_mirroringReg = value & 0x03;
					_extendedMirroring = (value & 0x08) == 0x08;
					break;

				case 0xD002:
					_chrWriteEnabled = (value & 0x40) == 0x40;
					_ntRamSelectBit = value & 0x80;
					break;

				case 0xD003:
					_mirrorChr = (value & 0x80) == 0x80;
					_outerBank = value & 0x3F;
					break;
			}
		}

		UpdateState();
	}

	void ProcessCpuClock() override
	{
		if(_irqSource == JyIrqSource::CpuClock || (_irqSource == JyIrqSource::CpuWrite && _console->GetCpu()->IsCpuWrite())) {
			TickIrqCounter();
		}
	}

	uint8_t MapperReadVRAM(uint16_t addr, MemoryOperationType type) override
	{
		if(_irqSource == JyIrqSource::PpuRead && type == MemoryOperationType::PpuRenderingRead) {
			TickIrqCounter();
		}

		if(addr >= 0x2000) {
			//This behavior only affects reads, not writes.
			//Additional info: https://forums.nesdev.com/viewtopic.php?f=3&t=17198
			if(_advancedNtControl) {
				uint8_t ntIndex = ((addr & 0x2FFF) - 0x2000) / 0x400;
				if(_disableNtRam || (_ntLowRegs[ntIndex] & 0x80) != (_ntRamSelectBit & 0x80)) {
					uint16_t mask, base;
					GetChrSetup(&mask, &base);

					uint16_t chrPage = base | ((_ntLowRegs[ntIndex] | (_ntHighRegs[ntIndex] << 8)) & mask);
					uint32_t chrOffset = chrPage * 0x400 + (addr & 0x3FF);
					if(_chrRomSize > chrOffset) {
						return _chrRom[chrOffset];
					} else {
						return 0;
					}
				}
			}
		}

		return BaseMapper::MapperReadVRAM(addr, type);
	}

	void NotifyVRAMAddressChange(uint16_t addr) override
	{
		if(_irqSource == JyIrqSource::PpuA12Rise && (addr & 0x1000) && !(_lastPpuAddr & 0x1000)) {
			TickIrqCounter();
		}
		_lastPpuAddr = addr;

		if(_mirrorChr) {
			switch(addr & 0x2FF8) {
				case 0x0FD8:
				case 0x0FE8:
					_chrLatch[addr >> 12] = addr >> 4 & ((addr >> 10 & 0x04) | 0x02);
					if(_chrMode == 1) {
						UpdateChrState();
					}
					break;
			}
		}
	}

	void TickIrqCounter()
	{
		bool clockIrqCounter = false;
		uint8_t mask = _irqSmallPrescaler ? 0x07 : 0xFF;
		uint8_t prescaler = _irqPrescaler & mask;

		if(_irqEnabled) {
			if(_irqCountDirection == 0x01) {
				prescaler++;
				if((prescaler & mask) == 0) {
					clockIrqCounter = true;
				}
			} else if(_irqCountDirection == 0x02) {
				if(--prescaler == 0) {
					clockIrqCounter = true;
				}
			}
			_irqPrescaler = (_irqPrescaler & ~mask) | (prescaler & mask);

			if(clockIrqCounter) {
				if(_irqCountDirection == 0x01) {
					_irqCounter++;
					if(_irqCounter == 0) {
						_console->GetCpu()->SetIrqSource(IRQSource::External);
					}
				} else if(_irqCountDirection == 0x02) {
					_irqCounter--;
					if(_irqCounter == 0xFF) {
						_console->GetCpu()->SetIrqSource(IRQSource::External);
					}
				}
			}
		}
	}
};
