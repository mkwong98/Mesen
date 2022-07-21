#pragma once
#include "stdafx.h"
#include "BaseMapper.h"
#include "CPU.h"
#include "A12Watcher.h"
#include "VrcIrq.h"

class Mapper556 : public BaseMapper
{
private:
	A12Watcher _a12Watcher;
	unique_ptr<VrcIrq> _VrcIrq;

	uint8_t _vrc2Chr[8];
	uint8_t _vrc2Prg[2];
	uint8_t _vrc2Mirroring;

	uint8_t _mmc3Regs[10];
	uint8_t _mmc3Ctrl;
	uint8_t _mmc3Mirroring;

	uint8_t _irqCounter;
	uint8_t _irqReloadValue;
	bool _irqReload;
	bool _irqEnabled;

	uint8_t _regIndex;

	uint16_t _prgMask;
	uint16_t _outerPrg;
	uint16_t _chrMask;
	uint16_t _outerChr;

	bool	_Vrc4Mode;
	bool	_locked;

protected:
	virtual uint16_t GetPRGPageSize() override { return 0x2000; }
	virtual uint16_t GetCHRPageSize() override { return 0x400; }

	void InitMapper() override
	{
		_VrcIrq.reset(new VrcIrq(_console));

		_vrc2Chr[0] = -1;
		_vrc2Chr[1] = -1;
		_vrc2Chr[2] = -1;
		_vrc2Chr[3] = -1;
		_vrc2Chr[4] = 4;
		_vrc2Chr[5] = 5;
		_vrc2Chr[6] = 6;
		_vrc2Chr[7] = 7;
		_vrc2Prg[0] = 0;
		_vrc2Prg[1] = 1;
		_vrc2Mirroring = 0;

		_mmc3Regs[0] = 0;
		_mmc3Regs[1] = 2;
		_mmc3Regs[2] = 4;
		_mmc3Regs[3] = 5;
		_mmc3Regs[4] = 6;
		_mmc3Regs[5] = 7;
		_mmc3Regs[6] = -4;
		_mmc3Regs[7] = -3;
		_mmc3Regs[8] = -2;
		_mmc3Regs[9] = -1;
		_mmc3Ctrl = 0;
		_mmc3Mirroring = 0;
		_irqCounter = 0;
		_irqReloadValue = 0;
		_irqEnabled = false;
		_irqReload = false;

		_Vrc4Mode = false;
		_regIndex = 0;
		_prgMask = 0x3F;
		_outerPrg = 0x00;
		_chrMask = 0xFF;
		_outerChr = 0x00;
		_locked = false;

		UpdateState();

		AddRegisterRange(0x5000, 0x5FFF, MemoryOperation::Write);
	}

	void Reset(bool softReset) override
	{
		_Vrc4Mode = false;
		_regIndex = 0;
		_prgMask = 0x3F;
		_outerPrg = 0x00;
		_chrMask = 0xFF;
		_outerChr = 0x00;
		_locked = false;

		UpdateState();
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);

		SnapshotInfo a12Watcher { &_a12Watcher };
		ArrayInfo<uint8_t> vrc2Chr { _vrc2Chr, 8 };
		ArrayInfo<uint8_t> vrc2Prg { _vrc2Prg, 2 };
		ArrayInfo<uint8_t> mmc3Regs { _mmc3Regs, 10 };

		Stream(a12Watcher,
				_Vrc4Mode, _regIndex, _prgMask, _outerPrg, _chrMask, _outerChr,
				vrc2Chr, vrc2Prg, _vrc2Mirroring,
				mmc3Regs, _mmc3Ctrl, _mmc3Mirroring, _irqCounter, _irqEnabled, _irqReload, _irqReloadValue
		);
	}

	virtual void ProcessCpuClock() override
	{
		if(_Vrc4Mode) {
			_VrcIrq->ProcessCpuClock();
		}
	}

	virtual void NotifyVRAMAddressChange(uint16_t addr) override
	{
		if(!_Vrc4Mode) {
			switch(_a12Watcher.UpdateVramAddress(addr, _console->GetPpu()->GetFrameCycle())) {
				case A12StateChange::None:
				case A12StateChange::Fall:
					break;

				case A12StateChange::Rise:
					if(_irqCounter == 0 || _irqReload) {
						_irqCounter = _irqReloadValue;
					} else {
						_irqCounter--;
					}

					if(_irqCounter == 0 && _irqEnabled) {
						_console->GetCpu()->SetIrqSource(IRQSource::External);
					}
					_irqReload = false;
					break;
			}
		}
	}

	void UpdatePrg()
	{
		if(_Vrc4Mode) {
			SelectPRGPage(0, _outerPrg | (_vrc2Prg[0] & _prgMask));
			SelectPRGPage(1, _outerPrg | (_vrc2Prg[1] & _prgMask));
			SelectPRGPage(2, _outerPrg | ((-2) & _prgMask));
			SelectPRGPage(3, _outerPrg | ((-1) & _prgMask));
		} else {
			uint32_t prgMode = (_mmc3Ctrl >> 5) & 0x02;
			SelectPRGPage(0, _outerPrg | (_mmc3Regs[6 + prgMode] & _prgMask));
			SelectPRGPage(1, _outerPrg | (_mmc3Regs[7] & _prgMask));
			SelectPRGPage(2, _outerPrg | (_mmc3Regs[6 + (prgMode ^ 0x02)] & _prgMask));
			SelectPRGPage(3, _outerPrg | (_mmc3Regs[9] & _prgMask));
		}
	}

	void UpdateChr()
	{
		if(_Vrc4Mode) {
			for(int i = 0; i < 8; i++) {
				SelectCHRPage(i, _outerChr | (_vrc2Chr[i] & _chrMask));
			}
		} else {
			uint32_t slotSwap = (_mmc3Ctrl & 0x80) ? 4 : 0;
			SelectCHRPage(0 ^ slotSwap, _outerChr | (((_mmc3Regs[0]) & 0xFE) & _chrMask));
			SelectCHRPage(1 ^ slotSwap, _outerChr | ((_mmc3Regs[0] | 1) & _chrMask));
			SelectCHRPage(2 ^ slotSwap, _outerChr | (((_mmc3Regs[1]) & 0xFE) & _chrMask));
			SelectCHRPage(3 ^ slotSwap, _outerChr | ((_mmc3Regs[1] | 1) & _chrMask));
			SelectCHRPage(4 ^ slotSwap, _outerChr | (_mmc3Regs[2] & _chrMask));
			SelectCHRPage(5 ^ slotSwap, _outerChr | (_mmc3Regs[3] & _chrMask));
			SelectCHRPage(6 ^ slotSwap, _outerChr | (_mmc3Regs[4] & _chrMask));
			SelectCHRPage(7 ^ slotSwap, _outerChr | (_mmc3Regs[5] & _chrMask));
		}
	}

	void UpdateMirroring()
	{
		if(_Vrc4Mode) {
			SetMirroringType((_vrc2Mirroring & 0x01) ? MirroringType::Horizontal : MirroringType::Vertical);
		} else {
			SetMirroringType((_mmc3Mirroring & 0x01) ? MirroringType::Horizontal : MirroringType::Vertical);
		}
	}

	void UpdateState()
	{
		UpdatePrg();
		UpdateChr();
		UpdateMirroring();
	}

	void WriteVrc2Register(uint16_t addr, uint8_t value)
	{
		addr = (addr & 0xFF00) | ((addr >> 2) & 3);

		if(addr >= 0xB000 && addr <= 0xE003) {
			int32_t regIndex = ((((addr & 0x02) | (addr >> 10)) >> 1) + 2) & 0x07;
			int32_t lowHighNibble = ((addr & 1) << 2);
			_vrc2Chr[regIndex] = (_vrc2Chr[regIndex] & (0xF0 >> lowHighNibble)) | ((value & 0x0F) << lowHighNibble);
			UpdateChr();
		} else {
			switch(addr & 0xF000) {
				case 0x8000: _vrc2Prg[0] = value; UpdatePrg(); break;
				case 0xA000: _vrc2Prg[1] = value; UpdatePrg(); break;
				case 0x9000: _vrc2Mirroring = value; UpdateMirroring(); break;
				case 0xF000:
					if(addr == 0xF000) {
						_VrcIrq->SetReloadValueNibble(value, false);
					} else if(addr == 0xF001) {
						_VrcIrq->SetReloadValueNibble(value, true);
					} else if(addr == 0xF002) {
						_VrcIrq->SetControlValue(value);
					} else if(addr == 0xF003) {
						_VrcIrq->AcknowledgeIrq();
					}
					break;
			}
		}
	}

	void WriteMmc3Register(uint16_t addr, uint8_t value)
	{
		switch(addr & 0xE001) {
			case 0x8000:
				_mmc3Ctrl = value;
				UpdateState();
				break;

			case 0x8001:
				_mmc3Regs[_mmc3Ctrl & 0x07] = value;
				UpdateState();
				break;

			case 0xA000:
				_mmc3Mirroring = value;
				UpdateState();
				break;

			case 0xC000: _irqReloadValue = value; break;
			case 0xC001: _irqReload = true; break;

			case 0xE000:
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
				_irqEnabled = false;
				break;

			case 0xE001: _irqEnabled = true; break;
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			if(!_locked) {
				switch(_regIndex) {
					case 0:
						_outerChr &= 0xFF00;
						_outerChr |= value;
						break;

					case 1:
						_outerPrg &= 0xFF00;
						_outerPrg |= value;
						break;

					case 2:
						_chrMask = 0xFF >> (~value & 0xF);
						_outerChr = (_outerChr & ~0x0F00) | ((value & 0xF0) << 4);
						_Vrc4Mode = (value & 0x80) == 0x80;
						break;

					case 3:
						_prgMask = ~value & 0x3F;
						_outerPrg = (_outerPrg & ~0x0100) | ((value & 0x40) << 2);
						_outerChr = (_outerChr & ~0x1000) | ((value & 0x40) << 6);
						_locked = (value & 0x80) == 0x80;
						break;
				}
				_regIndex = (_regIndex + 1) & 0x03;
				UpdateState();
			}
		} else {
			if(_Vrc4Mode) {
				WriteVrc2Register(addr, value);
			} else {
				WriteMmc3Register(addr, value);
			}
		}
	}
};
