#pragma once
#include "stdafx.h"
#include "Snapshotable.h"
#include "APU.h"
#include "BaseExpansionAudio.h"
#include "SSGAudio.h"
#include "Console.h"
#include "CPU.h"

#include <array>
#include "ym3438.h"

using EPSMSSGAudio = SSGAudio<AudioChannel::EPSM_L, AudioChannel::EPSM_R>;

class EPSMAudio : public EPSMSSGAudio
{
private:
	ym3438_t _chip;

	int16_t _lastOutputs[2];
	int16_t _currentOutputs[2];
	uint8_t writeValue;
	uint16_t writeAddr;
	uint16_t irqATimer;
	uint16_t irqBTimer;
	uint16_t irqACurrentTimer;
	uint16_t irqBCurrentTimer;
	uint8_t irqATimerEnable;
	uint8_t irqBTimerEnable;
	uint8_t irqAHighValue;
	uint8_t irqALowValue;
	uint8_t irqBValue;
	uint8_t irqEnable;
	uint16_t currentRegister;


	double _clock;
	double _clockIRQ;
	double _cycleCountIRQ = 0;

	static constexpr uint8_t cycleCount = 24;

	struct InputEntry
	{
		uint8_t addr;
		uint8_t data;
		uint8_t cycle;
		uint8_t wrote;
	};

	static constexpr uint8_t INPUT_BUFFER_SIZE = cycleCount;
	using InputBuffer = std::array<InputEntry, INPUT_BUFFER_SIZE>;
	InputBuffer _inputBuffer;

	void UpdateOutputLevel()
	{
		for (size_t x = 0; x < 2; x++)
		{
			_console->GetApu()->AddExpansionAudioDelta(x == 0 ? AudioChannel::EPSM_L : AudioChannel::EPSM_R, _currentOutputs[x] - _lastOutputs[x]);
			_lastOutputs[x] = _currentOutputs[x];
		}
	}

	uint8_t GetCurrentCycle() const
	{
		return static_cast<uint8_t>(std::floor(_clock)) % cycleCount;
	}

	void WriteToChip(uint8_t a, uint8_t d)
	{
		const auto cycle = GetCurrentCycle();

		if (_inputBuffer[cycle].wrote)
		{
			std::cout << "EPSM CHIP DOUBLE WRITE" << std::endl;
		}

		_inputBuffer[cycle] = {
			a,
			d,
			cycle,
			true
		};
	}


	void WriteToChipIRQ(uint16_t addr, uint8_t value)
	{
		switch (addr) {
		case 0x0:
		case 0x2:
			currentRegister = value;
			break;

		case 0x1:
			if (irqEnable) {
				if (currentRegister == 0x24) {
					//Timer A High 8 bits
					irqAHighValue = value;
				}
				if (currentRegister == 0x25) {
					//Timer A Low 2 bits
					irqALowValue = (value & 0x3);
				}
				if (currentRegister == 0x26) {
					//Timer B 8 bits
					irqBValue = value;
				}
				if (currentRegister == 0x27) {
					//Load+Enable IRQ (0xA = TimerB, 0x5 = TimerA)
					if ((value & 0x5)) {
						irqATimer = (uint16_t(irqAHighValue) << 2) | irqALowValue;
						irqACurrentTimer = 72 * (1024 - irqATimer) * 2;
						irqATimerEnable = 1;
					}
					if ((value & 0xA)) {
						irqBTimer = 1152 * (256 - irqBValue) * 2;
						irqBCurrentTimer = irqBTimer;
						irqBTimerEnable = 1;
					}
					if ((value & 0x10)) {
						//Enable/Reset IRQ
						_console->GetCpu()->ClearIrqSource(IRQSource::EPSM);
						irqATimerEnable = 0;
					}
					if ((value & 0x20)) {
						//Enable/Reset IRQ
						_console->GetCpu()->ClearIrqSource(IRQSource::EPSM);
						irqBTimerEnable = 0;
					}
				}
			}
			if (currentRegister == 0x29) {
				//std::cout << std::hex << "value: " << value << std::endl;
				if ((value & 0x3 && (value & 0x80))) {
				//if ((value & 0x3 )) {
					//enable IRQ's
					//std::cout << "enable IRQ's" << std::endl;
					irqEnable = 1;
				}
				if (!(value & 0x83)) {
					//enable IRQ's
					//std::cout << "enable IRQ's" << std::endl;
					irqEnable = 0;
					_console->GetCpu()->ClearIrqSource(IRQSource::EPSM);
					irqATimerEnable = 0;
					irqBTimerEnable = 0;
				}
			}
			break;
		case 0x3:
			/*if (currentRegister == 0x10) {
				std::cout << "0x10" << std::endl;
			}*/
			break;
		}
	}

	uint32_t getClockFrequency()  
	{
		return _console->GetSettings()->GetEPSMClockFrequency() / 6;
	}

protected:
	void StreamState(bool saving) override
	{
		EPSMSSGAudio::StreamState(saving);

		ArrayInfo<int16_t> lastOutputs{ _lastOutputs, 2 };
		ArrayInfo<int16_t> currentOutputs{ _currentOutputs, 2 };
		ArrayInfo<InputEntry> inputBuffer{ _inputBuffer.data(), INPUT_BUFFER_SIZE };
		ValueInfo<ym3438_t> chip{ &_chip };
		ValueInfo<double> clock { &_clock };
		Stream(lastOutputs, currentOutputs, inputBuffer, chip, clock);
	}

	void ClockAudio() override
	{
		EPSMSSGAudio::ClockAudio();

		_clock += getClockFrequency() / (double)_console->GetCpu()->GetClockRate(_console->GetModel());
		_clockIRQ += _console->GetSettings()->GetEPSMClockFrequency() / (double)_console->GetCpu()->GetClockRate(_console->GetModel());
		while (_clockIRQ >= _cycleCountIRQ) {
			_cycleCountIRQ++;
			if (irqATimerEnable) {
				irqACurrentTimer--;
				if (!irqACurrentTimer) {
					irqACurrentTimer++;
					_console->GetCpu()->SetIrqSource(IRQSource::EPSM);
				}

			}
			if (irqBTimerEnable) {
				irqBCurrentTimer--;
				if (!irqBCurrentTimer) {
					irqBCurrentTimer++;
					_console->GetCpu()->SetIrqSource(IRQSource::EPSM);
				}

			}
		}

		while (_clock >= cycleCount)
		{
			for (uint8_t x = 0; x < 2; x++)
			{
				_currentOutputs[x] = 0;
			}

			for (uint8_t cycle = 0; cycle < cycleCount; cycle++)
			{
				_clock--;

				int16_t samples[4];
				OPN2_Clock(&_chip, samples);

				for (uint8_t x = 0; x < 2; x++)
				{
					_currentOutputs[x] += samples[x];
					_currentOutputs[x] += samples[x+2]/11;
				}

				auto& input = _inputBuffer[cycle];
				if(input.wrote)
				{
					input.wrote = false;
					OPN2_Write(&_chip, input.addr, input.data);
				}
			}

			for (uint8_t x = 0; x < 2; x++)
			{
				_currentOutputs[x] = (int16_t)(_currentOutputs[x] / 2.5);
			}

			UpdateOutputLevel();
		}
	}

	virtual uint32_t GetSSGClockFrequency() override
	{
		return _console->GetSettings()->GetEPSMClockFrequency() / 4;
	}

public:
	EPSMAudio(shared_ptr<Console> console) : EPSMSSGAudio(console)
	{
		memset(_lastOutputs, 0, sizeof(_lastOutputs));
		memset(_currentOutputs, 0, sizeof(_currentOutputs));
		_inputBuffer = {};

		_clock = 0;
		_clockIRQ = 0;
		irqEnable = 0;

		irqATimerEnable = 0;
		irqBTimerEnable = 0;
		OPN2_Reset(&_chip);
		OPN2_SetChipType(0);
	}

	void WriteRegister(uint16_t addr, uint8_t value, uint8_t custom = 0, uint8_t epsmA0 = 0, uint8_t epsmA1 = 0)
	{
		if (!custom) {
			switch (addr) {
			case 0x4016:
				if ((value & 0x0E) == 0x02) {writeAddr = 0x0;} //A0 = 0, A1 = 0
				if ((value & 0x0E) == 0x0A) {writeAddr = 0x1;} //A0 = 1, A1 = 0
				if ((value & 0x0E) == 0x06) {writeAddr = 0x2;} //A0 = 0, A1 = 1
				if ((value & 0x0E) == 0x0E) {writeAddr = 0x3;} //A0 = 1, A1 = 1
				if (value & 0x0E) {writeValue = value;}
				if ((value & 0x02) == 0x00) {
					writeValue = (writeValue & 0xF0) | (value >> 4);

					const uint8_t a0 = !!(writeAddr & 0x1);
					const uint8_t a1 = !!(writeAddr & 0x2);
					if (a0 == 0x0) { writeAddr = 0xC000; }
					if (a0 == 0x1) { writeAddr = 0xE000; }
					if (a1 == 0x0) { EPSMSSGAudio::WriteRegister(writeAddr, writeValue); }
					WriteToChip(a0 | (a1 << 1), writeValue);
					WriteToChipIRQ(a0 | (a1 << 1), writeValue);
				}
				break;
			case 0x401c: //0xC000 A0 = 0, A1 = 0
			case 0x401d: //0xE000 A0 = 1, A1 = 0
			case 0x401e: //0xC002 A0 = 0, A1 = 1
			case 0x401f: //0xE002 A0 = 1, A1 = 1
				
				const uint8_t a0 = !!(addr & 0x1); //const uint8_t a0 = (addr & 0xF000) == 0xE000;
				const uint8_t a1 = !!(addr & 0x2); //const uint8_t a1 = !!(addr & 0xF);
				if (a0 == 0x0) { addr = 0xC000; }
				if (a0 == 0x1) { addr = 0xE000; }
				if (a1 == 0x0) { EPSMSSGAudio::WriteRegister(addr, value); }
				WriteToChip(a0 | (a1 << 1), value);
				WriteToChipIRQ(a0 | (a1 << 1), value);

				break;
			}
		}
		else{
			if (epsmA0 == 0x0) { addr = 0xC000; }
			if (epsmA0 == 0x1) { addr = 0xE000; }
			if (epsmA1 == 0x0) { EPSMSSGAudio::WriteRegister(addr, value); }
			WriteToChip(epsmA0 | (epsmA1 << 1), value);
			WriteToChipIRQ(epsmA0 | (epsmA1 << 1), value);
		}

	}
};
