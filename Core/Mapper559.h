#pragma once
#include "stdafx.h"
#include "BaseMapper.h"
#include "VrcIrq.h"
#include "Console.h"

// VRC4 Clone with nametable mirroring

class Mapper559 : public BaseMapper
{
	private:
		unique_ptr<VrcIrq> _irq;

		uint8_t _prgReg0;
		uint8_t _prgReg1;
		uint8_t _prgReg2;
		uint8_t _prgReg3;

		uint8_t _prgMode;

		uint8_t _mirroring;

		bool _wramEnable;

		uint8_t _hiCHRRegs[8];
		uint8_t _loCHRRegs[8];

		uint8_t _ntRegs[4];

	protected:
		uint16_t GetPRGPageSize() override { return 0x2000; }
		uint16_t GetCHRPageSize() override { return 0x0400; }

		void InitMapper() override
		{
			_irq.reset(new VrcIrq(_console));

			_prgMode = 0;

			_prgReg0 = 0;
			_prgReg1 = 1;
			_prgReg2 = -2;
			_prgReg3 = -1;

			_mirroring = 0;

			_wramEnable = _romInfo.IsNes20Header ? false : true;

			for(int i = 0; i < 8; i++) {
				_loCHRRegs[i] = i;
				_hiCHRRegs[i] = 0;
			}

			for(int i = 0; i < 4; i++) {
				_ntRegs[i] = i & 0x01;
			}

			UpdateState();
		}

		void ProcessCpuClock() override
		{
			_irq->ProcessCpuClock();
		}

		void UpdateState()
		{
			MemoryAccessType access;
			if(_wramEnable) {
				access = MemoryAccessType::ReadWrite;
			} else {
				access = MemoryAccessType::NoAccess;
			}
			SetCpuMemoryMapping(0x6000, 0x7FFF, 0, HasBattery() ? PrgMemoryType::SaveRam : PrgMemoryType::WorkRam, access);

			for(int i = 0; i < 8; i++) {
				uint32_t page = _loCHRRegs[i] | (_hiCHRRegs[i] << 4);
				SelectCHRPage(i, page);
			}

			SetNametable(0, _ntRegs[0] & 0x01);
			SetNametable(1, _ntRegs[1] & 0x01);
			SetNametable(2, _ntRegs[2] & 0x01);
			SetNametable(3, _ntRegs[3] & 0x01);

			if(_prgMode == 0) {
				SelectPRGPage(0, (_prgReg0 & 0x1F));
				SelectPRGPage(1, (_prgReg1 & 0x1F));
				SelectPRGPage(2, (_prgReg2 & 0x1F));
				SelectPRGPage(3, (_prgReg3 & 0x1F));
			} else {
				SelectPRGPage(0, (_prgReg2 & 0x1F));
				SelectPRGPage(1, (_prgReg1 & 0x1F));
				SelectPRGPage(2, (_prgReg0 & 0x1F));
				SelectPRGPage(3, (_prgReg3 & 0x1F));
			}
		}

		void WriteRegister(uint16_t addr, uint8_t value) override
		{
			uint16_t fullAddr = addr;

			addr = (addr & 0xF000) | ((addr & 0xC00) >> 10);

			if(addr >= 0x8000 && addr <= 0x8003) {
				_prgReg0 = value & 0x1F;
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
						_wramEnable = (value & 0x01) == 0x01;
						_prgMode = (value >> 1) & 0x01;
						break;
					case 3:
						if(fullAddr & 0x04) {
							_ntRegs[fullAddr & 3] = value;
						} else {
							_prgReg2 = value;
						}
						break;
				}
			} else if(addr >= 0xA000 && addr <= 0xA003) {
				_prgReg1 = value & 0x1F;
			} else {
				if(fullAddr & 0x400) {
					value >>= 4;
				}
			
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
			ArrayInfo<uint8_t> ntRegs = { _ntRegs, 4 };
			SnapshotInfo irq{ _irq.get() };
			Stream(_prgReg0, _prgReg1, _prgReg2, _prgReg3, _prgMode, loChrRegs, hiChrRegs, ntRegs, irq, _wramEnable);
		}
};