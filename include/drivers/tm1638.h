#pragma once
#include <cstdint>
#include <string>

typedef void(*voidFuncPtrTM1638)(uint8_t);

//0,4,5
template<typename STROBE, typename CLK, typename DATA>
	class TM1638 {
	public:
  
		static void init() {
			STROBE::SetOutput();
			CLK::SetOutput();
			DATA::SetOutput();
			STROBE::Set();
			IssueCommand(Commands::ACTIVATE);
			Reset();
		}
  
		static void SetInputHandler(voidFuncPtrTM1638 handler) {
			static os_timer_t timer;
			inputHandler = handler;
			os_timer_setfn(&timer, Refresh, NULL);
			os_timer_arm(&timer, 1, 1);
		}
  
		static void SetLeds(uint8_t value) {
			for (int i = 0; i < 8; i++)
			{
				SetLed(i, (value & (1 << i)) != 0);
			}
    
		}
  
		static void SetLed(uint8_t ledNumber, uint8_t value) {
			IssueCommand(Commands::WRITE_SINGLE);
			STROBE::Clear();
			Write(0xC1 + (ledNumber << 1));
			Write(value);
			STROBE::Set();
		}
	
		static void Display(uint32_t number)
		{	
			ResetDisplay();
			int i = 0;
			while (number != 0)
			{
				DisplayDigit(7 - i++, (number % 10));
				number /= 10;
			}
		}
  
		static void DisplayDigit(uint8_t position, uint8_t intValue)
		{
			IssueCommand(Commands::WRITE_SINGLE);
			STROBE::Clear();
			Write(0xC0 + (position << 1));
			Write(digits[intValue]);
			STROBE::Set();
		}
  
		static void ResetDisplay()
		{
			for (uint8_t i = 0; i < 8; i++)
			{
				IssueCommand(Commands::WRITE_SINGLE);
				STROBE::Clear();
				Write(0xC0 + (i << 1));
				Write(0x00);
				STROBE::Set(); 
			}
		}
		
		static void Reset()
		{
			IssueCommand(Commands::WRITE_CONSECUTIVE);
			STROBE::Clear();
			Write(0xC0); 
			for (uint8_t i = 0; i < 16; i++)
			{
				Write(0x00);
			}
			STROBE::Set(); 
		}
  
  
	private:
  
		enum class Commands:uint8_t
		{
			READ              = 0x42,
			WRITE_SINGLE      = 0x44,
			WRITE_CONSECUTIVE = 0X40,
			ACTIVATE          = 0x8f
		};

		static const uint8_t digits[];

  
		static voidFuncPtrTM1638 inputHandler;
  
		static void Refresh(void *arg) {
			uint8_t key;
			inputHandler(ReadKey());		
		}
  
		static void IssueCommand(Commands cmd)
		{
			STROBE::Clear();
			Write(static_cast <uint8_t>(cmd));
			STROBE::Set();
		}
  
		static void WriteData(uint8_t  value)
		{   
		
			if (value == 1)
			{
				DATA::Set();
			}
			else
			{
				DATA::Clear();
			}
		}
  
		static void Write(uint8_t  data)
		{
			for (uint8_t i = 0; i < 8; i++)
			{
				WriteData((data & (1 << i)) != 0);
				CLK::PulseHigh();
			}
		}
		
		static uint8_t Read()
		{
			uint8_t value = 0;
			for (uint8_t i = 0; i < 8; ++i)
			{
				CLK::Set();
				value |= DATA::Read() << i;
				CLK::Clear();
			}
			return value;
		}
		
		static uint8_t ReadKey()
		{
			STROBE::Clear();
			Write(static_cast <uint8_t>(Commands::READ));
			DATA::SetInput();
			uint8_t buttons = 0;
			for (uint8_t i = 0; i < 4; i++)
			{
				uint8_t byteRead = Read();
				buttons |= (byteRead << i);
			}
			DATA::SetOutput();
			STROBE::Set();
			return buttons;
		}
	};

template<typename STROBE, typename CLK, typename DATA> const uint8_t TM1638<STROBE, CLK, DATA>::digits[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f };
template<typename STROBE, typename CLK, typename DATA> voidFuncPtrTM1638 TM1638<STROBE, CLK, DATA>::inputHandler;