#pragma once
#include "stdafx.h"
#include "BaseMapper.h"

class Mapper382 : public BaseMapper
{
private:
	uint8_t _outerPrg;
	uint8_t _innerPrg;
	uint8_t _prgMode;
	uint8_t _mirroring;
	bool _locked;


protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectCHRPage(0, 0);

		UpdateState();
	}

	void Reset(bool softReset) override
	{
		_outerPrg = 0;
		_innerPrg = 0;
		_prgMode = 0;
		_mirroring = 0;
		_locked = false;

		UpdateState();
	}

	void StreamState(bool saving) override
	{
		BaseMapper::StreamState(saving);
		Stream(_outerPrg, _innerPrg, _prgMode, _mirroring, _locked);
	}

	void UpdateState()
	{
		if (_prgMode) {		// BNROM
			SelectPrgPage2x(0, ((_outerPrg << 2) | (_innerPrg & 0x03)) << 1);
		} else {			// UNROM
			SelectPRGPage(0, (_outerPrg << 3) | (_innerPrg & 0x07));
			SelectPRGPage(1, (_outerPrg << 3) | 0x07);
		}

		SetMirroringType(!_mirroring ? MirroringType::Vertical : MirroringType::Horizontal);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if (!_locked) {
			_outerPrg = addr & 0x07;
			_prgMode = (addr & 0x08) == 0x08;
			_mirroring = (addr & 0x10) == 0x10;
			_locked = (addr & 0x20) == 0x20;
		}

		_innerPrg = value & 0x07;

		UpdateState();
	}
};
