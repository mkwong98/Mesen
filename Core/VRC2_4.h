#pragma once
#include "stdafx.h"
#include "BaseMapper.h"
#include "VrcIrq.h"

enum class VRCVariant
{
	VRC2_308, //308
	VRC2_450, //450
	VRC2_524, //524
	VRC2_525, //525
	VRC2_527, //527
	VRC2a,	//Mapper 22
	VRC2b,	//23
	VRC2c,	//25
	VRC4a,	//21
	VRC4b,	//25
	VRC4c,	//21
	VRC4d,	//25
	VRC4e,	//23
	VRC4f,	//23 sub 1
	VRC4_27, //27
	VRC4_447, // 447
	VRC4_448, // 448
	VRC6a,
	VRC6b
};

class VRC2_4 : public BaseMapper
{
	private:
		unique_ptr<VrcIrq> _irq;
		VRCVariant _variant;
		bool _useHeuristics;

		uint8_t _prgReg0;
		uint8_t _prgReg1;
		uint8_t _prgMode;

		uint8_t _mirroring;

		bool _wramEnable;

		uint8_t _hiCHRRegs[8];
		uint8_t _loCHRRegs[8];

		uint8_t _microWire;
		uint8_t _regs; // extra registers
		uint8_t _latch;

		// Mapper 308 IRQ
		uint16_t _irqCounter;
		uint8_t _irqCounterHigh;
		bool _irqEnabled;

		void DetectVariant()
		{
			switch(_romInfo.MapperID) {
				default:
				case 21:
					//Conflicts: VRC4c
					switch(_romInfo.SubMapperID) {
						default:
						case 0: _variant = VRCVariant::VRC4a; break;
						case 1: _variant = VRCVariant::VRC4a; break;
						case 2: _variant = VRCVariant::VRC4c; break;
					}
					break;

				case 22: _variant = VRCVariant::VRC2a; break;

				case 23:
					//Conflicts: VRC4e
					switch(_romInfo.SubMapperID) {
						default:
						case 0: _variant = VRCVariant::VRC2b; break;
						case 1: _variant = VRCVariant::VRC4f; break;
						case 2: _variant = VRCVariant::VRC4e; break;
						case 3: _variant = VRCVariant::VRC2b; break;
					}
					break;

				case 25:
					//Conflicts: VRC2c, VRC4d
					switch(_romInfo.SubMapperID) {
						default:
						case 0: _variant = VRCVariant::VRC4b; break;
						case 1: _variant = VRCVariant::VRC4b; break;
						case 2: _variant = VRCVariant::VRC4d; break;
						case 3: _variant = VRCVariant::VRC2c; break;
					}
					break;

				case 27: _variant = VRCVariant::VRC4_27; break; //Untested
				case 308: _variant = VRCVariant::VRC2_308; break;
				case 447: _variant = VRCVariant::VRC4_447; break;
				case 448: _variant = VRCVariant::VRC4_448; break;
				case 450: _variant = VRCVariant::VRC2_450; break;
				case 524: _variant = VRCVariant::VRC2_524; break;
				case 525: _variant = VRCVariant::VRC2_525; break;
				case 527: _variant = VRCVariant::VRC2_527; break;
			}

			_useHeuristics = (_romInfo.SubMapperID == 0) && _romInfo.MapperID != 22 &&
				_romInfo.MapperID != 27 && _romInfo.MapperID != 308 &&
				_romInfo.MapperID != 524  && _romInfo.MapperID != 527  &&
				_romInfo.MapperID != 525 && _romInfo.MapperID != 447 &&
				_romInfo.MapperID != 448 && _romInfo.MapperID != 450;
		}

	protected:
		uint16_t GetPRGPageSize() override { return 0x2000; }
		uint16_t GetCHRPageSize() override { return 0x0400; }
		bool AllowRegisterRead() override { return true; }

		void InitMapper() override
		{
			_irq.reset(new VrcIrq(_console));
			DetectVariant();

			//PRG mode only exists for VRC4+ (so keep it as 0 at all times for VRC2)
			_prgMode = _variant >= VRCVariant::VRC4a ? (GetPowerOnByte() & 0x01) : 0;

			_prgReg0 = GetPowerOnByte() & 0x1F;
			_prgReg1 = GetPowerOnByte() & 0x1F;
			
			_microWire = 0 ;

			_regs = 0;
			_latch = 0;

			_mirroring = 0;

			_wramEnable = _romInfo.IsNes20Header ? false : true;

			for(int i = 0; i < 8; i++) {
				_loCHRRegs[i] = GetPowerOnByte() & 0x0F;
				_hiCHRRegs[i] = GetPowerOnByte() & 0x1F;
			}

			UpdateState();

			RemoveRegisterRange(0, 0xFFFF, MemoryOperation::Read);
			if(!_useHeuristics && _variant <= VRCVariant::VRC2c && _workRamSize == 0 && _saveRamSize == 0) {
				AddRegisterRange(0x6000, 0x7FFF, MemoryOperation::Any);
			}
			if((_variant == VRCVariant::VRC4_447) ||
			   (_variant == VRCVariant::VRC4_448)) {
				AddRegisterRange(0x6000, 0x7FFF, MemoryOperation::Write);
			}
		}

		void ProcessCpuClock() override
		{
			if(_variant == VRCVariant::VRC2_308) {
				if(_irqEnabled) {
					_irqCounter++;
					if((_irqCounter & 0x0FFF) == 2048)
						_irqCounterHigh--;
					if(!_irqCounterHigh && (_irqCounter & 0x0FFF) < 2048)
						_console->GetCpu()->SetIrqSource(IRQSource::External);
				}
			} else if(_variant == VRCVariant::VRC2_524) {
				if(_irqEnabled) {
					_irqCounter++;
					if(_irqCounter & 1024)
						_console->GetCpu()->SetIrqSource(IRQSource::External);
				}
			} else if((_useHeuristics && _romInfo.MapperID != 22) || _variant >= VRCVariant::VRC4a) {
				//Only VRC4 supports IRQs
				_irq->ProcessCpuClock();
			}
		}

		void UpdateVRC24Prg(uint16_t mask, uint16_t base)
		{
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

		void UpdateState()
		{
			if(_variant == VRCVariant::VRC4_448) {
				SelectChrPage8x(0, 0);
			} else {
			for(int i = 0; i < 8; i++) {
				uint32_t page = _loCHRRegs[i] | (_hiCHRRegs[i] << 4);
				if(_variant == VRCVariant::VRC2a) {
					//"On VRC2a (mapper 022) only the high 7 bits of the CHR regs are used -- the low bit is ignored.  Therefore, you effectively have to right-shift the CHR page by 1 to get the actual page number."
					page >>= 1;
				}
				if(_variant == VRCVariant::VRC4_447) {
					page &= 0x7F;
					page |= _regs << 7;
				} else if(_variant == VRCVariant::VRC2_450) {
					page &= 0x7F;
					page |= _microWire << 7;
				}
				SelectCHRPage(i, page);
			}
			}

			if(_variant == VRCVariant::VRC2_527) {
				SetNametable(0, (_hiCHRRegs[0] >> 3) & 0x01);
				SetNametable(1, (_hiCHRRegs[0] >> 3) & 0x01);
				SetNametable(2, (_hiCHRRegs[1] >> 3) & 0x01);
				SetNametable(3, (_hiCHRRegs[1] >> 3) & 0x01);
			}

			if(_variant == VRCVariant::VRC4_447) {
				if(_regs & 0x04) {
					uint8_t prgA14 = ~_regs & 0x02;
					SelectPRGPage(0, (_regs << 4) | ((_prgReg0 & ~prgA14) & 0x0F));
					SelectPRGPage(1, (_regs << 4) | ((_prgReg1 & ~prgA14) & 0x0F));
					SelectPRGPage(2, (_regs << 4) | ((_prgReg0 |  prgA14) & 0x0F));
					SelectPRGPage(3, (_regs << 4) | ((_prgReg1 |  prgA14) & 0x0F));
				} else {
					UpdateVRC24Prg(0x0F, (_regs << 4));
				}
			} else if(_variant == VRCVariant::VRC4_448) {
				if(_regs & 0x08) {
					SelectPrgPage4x(0, (((_regs << 2) & 0xF8) | (_latch & 0x07)) << 2);
					SetMirroringType((_latch & 0x10) ? MirroringType::ScreenBOnly : MirroringType::ScreenAOnly);
				} else {
					if(_regs & 0x04) {
						SelectPrgPage2x(0, (((_regs << 3) & 0xF0) | (_prgReg0 & 0x0F)) << 1);
						SelectPrgPage2x(1, (((_regs << 3) & 0xF0) | 0x0F) << 1);
					} else {
						SelectPrgPage2x(0, ((_regs << 3) | (_prgReg0 & 0x07)) << 1);
						SelectPrgPage2x(1, ((_regs << 3) | 0x07) << 1);
					}
				}
			} else if(_variant == VRCVariant::VRC2_450) {
				UpdateVRC24Prg(0x0F, (_microWire << 4));
			} else if(_variant == VRCVariant::VRC2_525) {
				SelectPRGPage(0, _prgReg0 + 0);
				SelectPRGPage(1, _prgReg0 + 1);
				SelectPRGPage(2, -2);
				SelectPRGPage(3, -1);
			} else {
			 	UpdateVRC24Prg(0x1F, 0);
			}
		}

		uint8_t ReadRegister(uint16_t addr) override
		{
			//Microwire interface ($6000-$6FFF) (VRC2 only)
			return (_microWire & 0x01) | (_console->GetMemoryManager()->GetOpenBus() & 0xFE);
		}

		void WriteRegister(uint16_t addr, uint8_t value) override
		{
			if(addr < 0x8000) {
				WritePrgRam(addr, value);
				//Microwire interface ($6000-$6FFF) (VRC2 only)
				// Wires repurposed as outerbank register for Mapper 450
				_microWire = value & 0x07;
				if(_wramEnable) {
					if(_variant == VRCVariant::VRC4_447) {
						if(!(_regs & 0x01)) {
							_regs = addr & 0xFF;
						}
					} else if(_variant == VRCVariant::VRC4_448) {
						_regs = addr & 0xFF;
					}
				}
				UpdateState();
				return;
			}

			_latch = value;

			addr = TranslateAddress(addr) & 0xF00F;

			if(addr >= 0x8000 && addr <= 0x8006) {
				_prgReg0 = value & 0x1F;
			} else if((_variant <= VRCVariant::VRC2c && addr >= 0x9000 && addr <= 0x9003) || (_variant >= VRCVariant::VRC4a && addr >= 0x9000 && addr <= 0x9001)) {
				uint8_t mask = 0x03;
				if(!_useHeuristics && (_variant >= VRCVariant::VRC2a && _variant <= VRCVariant::VRC2c)) {
					//When we are certain this is a VRC2 game, only use the first bit for mirroring selection
					mask = 0x01;
				}

				switch(value & mask) {
					case 0: SetMirroringType(MirroringType::Vertical); break;
					case 1: SetMirroringType(MirroringType::Horizontal); break;
					case 2: SetMirroringType(MirroringType::ScreenAOnly); break;
					case 3: SetMirroringType(MirroringType::ScreenBOnly); break;
				}
			} else if(_variant >= VRCVariant::VRC4a && addr >= 0x9002 && addr <= 0x9003) {
				_wramEnable = (value & 0x01) == 0x01;
				_prgMode = (value >> 1) & 0x01;
			} else if(addr >= 0xA000 && addr <= 0xA006) {
				_prgReg1 = value & 0x1F;
			} else if(addr >= 0xB000 && addr <= 0xE006) {
				if(_variant == VRCVariant::VRC2_525) {
					if(addr <= 0xB007) {
						_loCHRRegs[addr & 0x07] = value;
						_hiCHRRegs[addr & 0x07] = 0;
					}
				} else {
				uint8_t regNumber = ((((addr >> 12) & 0x07) - 3) << 1) + ((addr >> 1) & 0x01);
				bool lowBits = (addr & 0x01) == 0x00;
				if(lowBits) {
					//The other reg contains the low 4 bits
					_loCHRRegs[regNumber] = value & 0x0F;
				} else {
					//One reg contains the high 5 bits
					_hiCHRRegs[regNumber] = value & 0x1F;
				}
				}
			} else if(_variant == VRCVariant::VRC2_308) {
				if(addr == 0xF000) {
					_irqEnabled = false;
					_irqCounter = 0;
					_console->GetCpu()->ClearIrqSource(IRQSource::External);
				} else if(addr == 0xF001) {
					_irqEnabled = true;
				} else if(addr == 0xF003) {
					_irqCounterHigh = value >> 4;
				}
			} else if(_variant == VRCVariant::VRC2_524) {
				if(addr == 0xF002) {
					_irqEnabled = true;
				} else if(addr == 0xF003) {
					_irqEnabled = false;
					_irqCounter = 0;
					_console->GetCpu()->ClearIrqSource(IRQSource::External);
				}
			} else {
				if(addr == 0xF000) {
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

	public:
		uint16_t TranslateAddress(uint16_t addr)
		{
			uint32_t A0, A1;

			if(_useHeuristics) {
				switch(_variant) {
					case VRCVariant::VRC2c:
					case VRCVariant::VRC4b:
					case VRCVariant::VRC4d:
						//Mapper 25
						//ORing both values should make most games work.
						//VRC2c & VRC4b (Both uses the same bits)
						A0 = (addr >> 1) & 0x01;
						A1 = (addr & 0x01);

						//VRC4d
						A0 |= (addr >> 3) & 0x01;
						A1 |= (addr >> 2) & 0x01;
						break;
					case VRCVariant::VRC4a:
					case VRCVariant::VRC4c:
						//Mapper 21
						//VRC4a
						A0 = (addr >> 1) & 0x01;
						A1 = (addr >> 2) & 0x01;

						//VRC4c
						A0 |= (addr >> 6) & 0x01;
						A1 |= (addr >> 7) & 0x01;
						break;

					case VRCVariant::VRC2b:
					case VRCVariant::VRC4e:
						//Mapper 23
						//VRC2b
						A0 = addr & 0x01;
						A1 = (addr >> 1) & 0x01;

						//VRC4e
						A0 |= (addr >> 2) & 0x01;
						A1 |= (addr >> 3) & 0x01;
						break;
					default:
						throw std::runtime_error("not supported");
						break;
				}
			} else {
				switch(_variant) {
					case VRCVariant::VRC2a:
						//Mapper 22
						A0 = (addr >> 1) & 0x01;
						A1 = (addr & 0x01);
						break;

					case VRCVariant::VRC4_27:
					case VRCVariant::VRC2_308:
					case VRCVariant::VRC2_527:
						//Mapper 27
						A0 = addr & 0x01;
						A1 = (addr >> 1) & 0x01;
						break;

					case VRCVariant::VRC2_525:
						A0 = addr & 0x01;
						A1 = (addr >> 1) & 0x03;
						break;

					case VRCVariant::VRC4f:
						// Mapper 32, submapper 1
						// VRC4 clone with VRC2 registers
						A0 = addr & 0x01;
						A1 = (addr >> 1) & 0x01;
						break;

					case VRCVariant::VRC2_524:
						//Mapper 524
						A0 = addr & 0x01;
						A1 = (addr >> 1) & 0x01;
						A0 |= (addr >> 2) & 0x01;
						A1 |= (addr >> 3) & 0x01;
						break;

					case VRCVariant::VRC4_447:
					case VRCVariant::VRC4_448:
						// Mapper 447
						A0 = (addr >> 2) & 0x01;
						A1 = (addr >> 3) & 0x01;
						break;

					case VRCVariant::VRC2_450:
						A0 = addr & 0x01;
						A1 = (addr >> 1) & 0x01;
						break;

					case VRCVariant::VRC2c:
					case VRCVariant::VRC4b:
						//Mapper 25
						A0 = (addr >> 1) & 0x01;
						A1 = (addr & 0x01);
						break;

					case VRCVariant::VRC4d:
						//Mapper 25
						A0 = (addr >> 3) & 0x01;
						A1 = (addr >> 2) & 0x01;
						break;

					case VRCVariant::VRC4a:
						//Mapper 21
						A0 = (addr >> 1) & 0x01;
						A1 = (addr >> 2) & 0x01;
						break;

					case VRCVariant::VRC4c:
						//Mapper 21
						A0 = (addr >> 6) & 0x01;
						A1 = (addr >> 7) & 0x01;
						break;

					case VRCVariant::VRC2b:
						//Mapper 23
						A0 = addr & 0x01;
						A1 = (addr >> 1) & 0x01;
						break;

					case VRCVariant::VRC4e:
						//Mapper 23
						A0 = (addr >> 2) & 0x01;
						A1 = (addr >> 3) & 0x01;
						break;

					default:
						throw std::runtime_error("not supported");
						break;
				}
			}

			return (addr & 0xFF00) | (A1 << 1) | A0;
		}

		void StreamState(bool saving) override
		{
			BaseMapper::StreamState(saving);
			ArrayInfo<uint8_t> loChrRegs = { _loCHRRegs, 8 };
			ArrayInfo<uint8_t> hiChrRegs = { _hiCHRRegs, 8 };
			SnapshotInfo irq{ _irq.get() };
			Stream(_prgReg0, _prgReg1, _prgMode, loChrRegs, hiChrRegs, _microWire, irq, _irqCounter, _irqCounterHigh, _irqEnabled, _wramEnable, _regs, _latch);
		}
};