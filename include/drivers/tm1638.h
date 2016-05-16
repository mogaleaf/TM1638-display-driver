#pragma once
#include <cstdint>
#include <string>

typedef void (*voidFuncPtrTM1638)(uint8_t);

//0,4,5
template <typename STROBE, typename CLK, typename DATA>
class TM1638 {
public:
    static const uint8_t digits[];

    static void init() {
        STROBE::setOutput();
        CLK::setOutput();
        DATA::setOutput();
        STROBE::set();
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
        for (auto i = 0; i < 8; i++) {
            SetLed(i, (value & (1 << i)) != 0);
        }
    }

    static void SetLed(uint8_t ledNumber, uint8_t value) {
        IssueCommand(Commands::WRITE_SINGLE);
        STROBE::clear();
        Write(0xC1 + (ledNumber << 1));
        Write(value);
        STROBE::set();
    }

    static void DisplayDigit(uint8_t position, uint8_t intValue) {
        IssueCommand(Commands::WRITE_SINGLE);
        STROBE::clear();
        Write(0xC0 + (position << 1));
        Write(digits[intValue]);
        STROBE::set();
    }

    static void ResetDisplay() {
        for (uint8_t i = 0; i < 8; i++) {
            IssueCommand(Commands::WRITE_SINGLE);
            STROBE::clear();
            Write(0xC0 + (i << 1));
            Write(0x00);
            STROBE::set();
        }
    }

    static void Reset() {
        IssueCommand(Commands::WRITE_CONSECUTIVE);
        STROBE::clear();
        Write(0xC0);
        for (uint8_t i = 0; i < 16; i++) {
            Write(0x00);
        }
        STROBE::set();
    }


private:

    enum class Commands:uint8_t {
        READ = 0x42,
        WRITE_SINGLE = 0x44,
        WRITE_CONSECUTIVE = 0X40,
        ACTIVATE = 0x8f
    };


    static voidFuncPtrTM1638 inputHandler;

    static void Refresh(void* arg) {
        uint8_t key;
        inputHandler(ReadKey());
    }

    static void IssueCommand(Commands cmd) {
        STROBE::clear();
        Write(static_cast<uint8_t>(cmd));
        STROBE::set();
    }

    static void Write(uint8_t data) {
        for (auto bitIndex = 0; bitIndex < 8; ++bitIndex) {
            DATA::set((data & (1 << bitIndex)) != 0);
            CLK::pulseHigh();
        }
    }

    static uint8_t Read() {
        uint8_t value = 0;
        for (auto bitIndex = 0; bitIndex < 8; ++bitIndex) {
            CLK::set();
            value |= DATA::read() << bitIndex;
            CLK::clear();
        }
        return value;
    }

    static uint8_t ReadKey() {
        STROBE::clear();
        Write(static_cast<uint8_t>(Commands::READ));
        DATA::setInput();
        uint8_t buttons = 0;
        for (auto bitIndex = 0; bitIndex < 4; ++bitIndex) {
            auto byteRead = Read();
            buttons |= (byteRead << bitIndex);
        }
        DATA::setOutput();
        STROBE::set();
        return buttons;
    }
};


template <typename TM1638>
class TMPrinter {
public:
    static void Print(uint8_t intValue) {
        //ResetDisplay();
        for (auto i = 0; i < 8 && intValue > 0; i++)
        {
            TM1638::DisplayDigit(i, (intValue % 10));
            intValue /= 10;
        }
    }
};



template <typename STROBE, typename CLK, typename DATA> const uint8_t TM1638<STROBE, CLK, DATA>::digits[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f};
template <typename STROBE, typename CLK, typename DATA> voidFuncPtrTM1638 TM1638<STROBE, CLK, DATA>::inputHandler;

