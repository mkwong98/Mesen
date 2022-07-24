#pragma once
#include "stdafx.h"
#include "BaseMapper.h"
#include "VrcIrq.h"
#include "Console.h"

// VRC4 Clone with highest CHR-ROM bank bits are repurposed as outer bank bits

class Mapper362 : public BaseMapper
{
	private:
		unique_ptr<VrcIrq> _irq;

		uint8_t _prgReg0;
		uint8_t _prgReg1;

		uint8_t _prgMode;

		uint8_t _mirroring;

		uint8_t _hiCHRRegs[8];
		uint8_t _loCHRRegs[8];

		uint8_t _game;
		uint8_t _currentChrBank;

	protected:
		uint16_t GetPRGPageSize() override { return 0x2000; }
		uint16_t GetCHRPageSize() override { return 0x0400; }

		void InitMapper() override
		{
			_irq.reset(new VrcIrq(_console));

			_prgMode = 0;

			_prgReg0 = 0;
			_prgReg1 = 1;

			_mirroring = 0;

			for(int i = 0; i < 8; i++) {
				_loCHRRegs[i] = i;
				_hiCHRRegs[i] = 0;
			}

			_currentChrBank = 0;
			_game = 0;

			UpdateState();
		}

		void Reset(bool softReset) override
		{
			if(!softReset || (_prgSize <= (512 * 1024))) {
				_game = 0;
			} else {
				_game = (_game + 1) & 0x01;
			}
			
			UpdateState();
		}

		void ProcessCpuClock() override
		{
			_irq->ProcessCpuClock();
		}

		void UpdateState()
		{
			uint32_t base = (_game == 0) ? ((_hiCHRRegs[_currentChrBank] << 4) & 0x180) : 0x200;
			uint32_t mask = (_game == 0) ? 0x7F : 0x1FF;

			for(int i = 0; i < 8; i++) {
				uint32_t page = _loCHRRegs[i] | (_hiCHRRegs[i] << 4);
				SelectCHRPage(i, base | (page & mask));
			}

			base = (_game == 0) ? ((_hiCHRRegs[_currentChrBank] << 1) & 0x30) : 0x40;
			mask = 0x0F;

			if(_prgMode == 0) {
				SelectPRGPage(0, base | (_prgReg0 & mask));
				SelectPRGPage(1, base | (_prgReg1 & mask));
				SelectPRGPage(2, base | ((-2) & mask));
				SelectPRGPage(3, base | ((-1) & mask));
			} else {
				SelectPRGPage(0, base | ((-2) & mask));
				SelectPRGPage(1, base | (_prgReg1 & mask));
				SelectPRGPage(2, base | (_prgReg0 & mask));
				SelectPRGPage(3, base | ((-1) & mask));
			}
		}

		void WriteRegister(uint16_t addr, uint8_t value) override
		{
			addr &= 0xF003;
			if(addr >= 0x8000 && addr <= 0x8003) {
				_prgReg0 = value;
			} else if(addr >= 0x9000 && addr <= 0x9003) {
				switch(addr & 0x03) {
					case 0:
					case 1:
						switch(value & 0x03) {
						case 0: SetMirroringType(MirroringType::Vertical); break;
						case 1: SetMirroringType(MirroringType::Horizontal); break;
						case 2: SetMirroringType(MirroringType::ScreenAOnly); break;
						case 3: SetMirroringType(MirroringType::ScreenBOnly); break;
						}
						break;
					case 2:
					case 3:
						_prgMode = (value >> 1) & 0x01;
						break;
				}
			} else if(addr >= 0xA000 && addr <= 0xA003) {
				_prgReg1 = value;
			} else {
				if(addr >= 0xB000 && addr <= 0xE003) {
					uint8_t regNumber = ((((addr >> 12) & 0x07) - 3) << 1) + ((addr >> 1) & 0x01);
					bool lowBits = (addr & 0x01) == 0x00;
					if(lowBits) {
						//The other reg contains the low 4 bits
						_loCHRRegs[regNumber] = value & 0x0F;
					} else {
						//One reg contains the high 5 bits
						_hiCHRRegs[regNumber] = value & 0x1F;
					}
				} else if(addr == 0xF000) {
					_irq->SetReloadValueNibble(value, false);
				} else if(addr == 0xF001) {
					_irq->SetReloadValueNibble(value, true);
				} else if(addr == 0xF002) {
					_irq->SetControlValue(value);
				} else if(addr == 0xF003) {
					_irq->AcknowledgeIrq();
				}
			}

			UpdateState();
		}

		void StreamState(bool saving) override
		{
			BaseMapper::StreamState(saving);
			ArrayInfo<uint8_t> loChrRegs = { _loCHRRegs, 8 };
			ArrayInfo<uint8_t> hiChrRegs = { _hiCHRRegs, 8 };
			SnapshotInfo irq{ _irq.get() };
			Stream(_prgReg0, _prgReg1, _prgMode, loChrRegs, hiChrRegs, irq, _game, _currentChrBank);
		}

		uint8_t MapperReadVRAM(uint16_t addr, MemoryOperationType type) override
		{
			uint8_t bank = addr >> 10;
			if(!(addr & 0x2000) && (_game == 0) && (_currentChrBank != bank)) {
				_currentChrBank =  bank;
				UpdateState();
			}

			return BaseMapper::MapperReadVRAM(addr, type);
		}
};