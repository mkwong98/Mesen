#pragma once
#include "MMC1.h"

class MMC1_374 : public MMC1
{
private:
	uint8_t _gameIndex;

protected:
	void InitMapper() override
	{
		_gameIndex = 0;
		MMC1::InitMapper();
	}

	void Reset(bool softReset) override
	{
		if(softReset) {
			_gameIndex++;
			_gameIndex &= 0x03;
		} else {
			_gameIndex = 0;
		}

		MMC1::UpdateState();
	}

	void StreamState(bool saving) override
	{
		MMC1::StreamState(saving);
		Stream(_gameIndex);
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		BaseMapper::SelectPRGPage(slot, (_gameIndex << 3) | (page & 0x07));
	}

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		BaseMapper::SelectCHRPage(slot, (_gameIndex << 5) | (page & 0x1F));
	}
};
