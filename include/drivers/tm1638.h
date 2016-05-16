#pragma once
#include <cstdint>
#include <string>

typedef void (*voidFuncPtrTM1638)(uint8_t);

//0,4,5
template <typename STROBE, typename CLK, typename DATA>
class TM1638 {
public:
    static const uint8_t digits[];
    static const uint8_t font[];

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
    /*
    static void DisplayDigit(uint8_t position, uint8_t intValue) {
        IssueCommand(Commands::WRITE_SINGLE);
        STROBE::clear();
        Write(0xC0 + (position << 1));
        Write(font[33+intValue]);
        STROBE::set();
    }
    */
   
    static void DisplayDigit(uint8_t position, char character) {
        IssueCommand(Commands::WRITE_SINGLE);
        STROBE::clear();
        Write(0xC0 + (position << 1));
        Write(font[character + 32]);
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
template <typename Strobe, typename Clk, typename Data> const uint8_t TM1638<Strobe, Clk, Data>::font[] = {
    0b00000000 /* */, 0b10000110 /*!*/, 0b00100010 /*"*/, 0b01111110 /*#*/, 0b01101101 /*$*/, 0b00000000 /*%*/, 0b00000000 /*&*/, 0b00000010 /*'*/,
    0b00110000 /*(*/, 0b00000110 /*)*/, 0b01100011 /***/, 0b00000000 /*+*/, 0b00000100 /*,*/, 0b01000000 /*-*/, 0b10000000 /*.*/, 0b01010010 /*/*/,
    0b00111111 /*0*/, 0b00000110 /*1*/, 0b01011011 /*2*/, 0b01001111 /*3*/, 0b01100110 /*4*/, 0b01101101 /*5*/, 0b01111101 /*6*/, 0b00100111 /*7*/,
    0b01111111 /*8*/, 0b01101111 /*9*/, 0b00000000 /*:*/, 0b00000000 /*;*/, 0b00000000 /*<*/, 0b01001000 /*=*/, 0b00000000 /*>*/, 0b01010011 /*?*/,
    0b01011111 /*@*/, 0b01110111 /*A*/, 0b01111111 /*B*/, 0b00111001 /*C*/, 0b00111111 /*D*/, 0b01111001 /*E*/, 0b01110001 /*F*/, 0b00111101 /*G*/,
    0b01110110 /*H*/, 0b00000110 /*I*/, 0b00011111 /*J*/, 0b01101001 /*K*/, 0b00111000 /*L*/, 0b00010101 /*M*/, 0b00110111 /*N*/, 0b00111111 /*O*/,
    0b01110011 /*P*/, 0b01100111 /*Q*/, 0b00110001 /*R*/, 0b01101101 /*S*/, 0b01111000 /*T*/, 0b00111110 /*U*/, 0b00101010 /*V*/, 0b00011101 /*W*/,
    0b01110110 /*X*/, 0b01101110 /*Y*/, 0b01011011 /*Z*/, 0b00111001 /*[*/, 0b01100100 /*\*/, 0b00001111 /*]*/, 0b00000000 /*^*/, 0b00001000 /*_*/,
    0b00100000 /*`*/, 0b01011111 /*a*/, 0b01111100 /*b*/, 0b01011000 /*c*/, 0b01011110 /*d*/, 0b01111011 /*e*/, 0b00110001 /*f*/, 0b01101111 /*g*/,
    0b01110100 /*h*/, 0b00000100 /*i*/, 0b00001110 /*j*/, 0b01110101 /*k*/, 0b00110000 /*l*/, 0b01010101 /*m*/, 0b01010100 /*n*/, 0b01011100 /*o*/,
    0b01110011 /*p*/, 0b01100111 /*q*/, 0b01010000 /*r*/, 0b01101101 /*s*/, 0b01111000 /*t*/, 0b00011100 /*u*/, 0b00101010 /*v*/, 0b00011101 /*w*/,
    0b01110110 /*x*/, 0b01101110 /*y*/, 0b01000111 /*z*/, 0b01000110 /*{*/, 0b00000110 /*|*/, 0b01110000 /*}*/, 0b00000001 /*~*/ };

