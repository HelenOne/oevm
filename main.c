#include <iostream>
#include <cmath>
#include <sstream>
#include <climits>
#include <string>
#include <cstdio>
#include <termios.h>

static struct termios settings;

int isTerminalSetupCompleted = false;

char getch()
{
    if (!isTerminalSetupCompleted)
    {
        tcgetattr(0, &settings);
        settings.c_lflag &= ~ICANON;
        settings.c_lflag &= ~ECHO;
        tcsetattr(0, TCSANOW, &settings);
        isTerminalSetupCompleted = true;
    }
    return getchar();
}

void moveCursor(int x, int y)
{
    printf("%c[%d;%df", 0x1B, y, x + 1);
}

int rangeInput(int x, int y, int from, int to, bool echo)
{
    int value = 0;
    moveCursor(x, y);
    bool touched = false;
    while (true)
    {
        int maxInput = -1;
        int minInput = 10;
        for (int i = 0; i < 10; i++)
        {
            if (value * 10 + i >= from && value * 10 + i <= to)
            {
                maxInput = i;
            }
        }
        for (int i = 9; i >= 0; i--)
        {
            if (value * 10 + i <= to)
            {
                minInput = i;
            }
        }
        moveCursor(x, y);
        std::cout << "                                    " << std::flush;
        moveCursor(x, y);
        if (value > 0 || touched)
        {
            std::cout << value << std::flush;
        }
        std::cout << "    " << std::flush;
        if (maxInput == -1 && minInput == 10)
        {
            if (value >= from && value <= to)
            {
                std::cout << "(press enter to continue)" << std::flush;
            }
            else
            {
                std::cout << "(must be " << from << "-" << to << ")";
            }
        }
        else
        {
            std::cout << "(" << from << " - " << to << ")" << std::flush;
        }

        moveCursor(x, y);
        if (value > 0 || touched)
        {
            std::cout << value << std::flush;
        }

        char inputSymbol = getch();
        touched = true;

        if (inputSymbol >= '0' && inputSymbol <= '9')
        {
            int inputValue = inputSymbol - '0';
            if (inputValue >= minInput && inputValue <= maxInput)
            {
                value = value * 10 + inputValue;
            }
        }
        else if (inputSymbol == 127)
        {
            value /= 10;
            if (value == 0)
                touched = false;
        }
        else if (inputSymbol == '\n' && value >= from && value <= to)
        {
            moveCursor(x, y);
            std::cout << "                                    " << std::flush;
            moveCursor(x, y);
            if (echo)
            {
                std::cout << value;
            }
            return value;
        }
        else if (inputSymbol == '\t')
        {
            return -1;
        }
    }
    return value;
}

bool isNegativeMap[] = {
        false, // char
        true,  // signed char
        true,  // short
        true,  // short int
        true,  // signed short
        true,  // signed short int
        false, // unsigned short
        false, // unsigned short int
        true,  // int
        true,  // signed int
        false, // unsigned int
        true,  // long
        true,  // long int
        true,  // signed long
        true,  // signed long int
        false, // unsigned long
        false, // unsigned long int
        true,  // long long
        true,  // long long int
        true,  // signed long long
        true,  // signed long long int
        false, // unsigned long long
        false, // unsigned long long int
        true,  // float
        true,  // double
};

bool isFloatMap[] = {
        false, // char
        false, // signed char
        false, // short
        false, // short int
        false, // signed short
        false, // signed short int
        false, // unsigned short
        false, // unsigned short int
        false, // int
        false, // signed int
        false, // unsigned int
        false, // long
        false, // long int
        false, // signed long
        false, // signed long int
        false, // unsigned long
        false, // unsigned long int
        false, // long long
        false, // long long int
        false, // signed long long
        false, // signed long long int
        false, // unsigned long long
        false, // unsigned long long int
        true,  // float
        true,  // double
};

std::string dataTypeNames[] = {
        "char",
        "signed char",
        "short",
        "short int",
        "signed short",
        "signed short int",
        "unsigned short",
        "unsigned short int",
        "int",
        "signed int",
        "unsigned int",
        "long",
        "long int",
        "signed long",
        "signed long int",
        "unsigned long",
        "unsigned long int",
        "long long",
        "long long int",
        "signed long long",
        "signed long long int",
        "unsigned long long",
        "unsigned long long int",
        "float",
        "double",
};

short varSize[] = {
        sizeof(char),
        sizeof(signed char),
        sizeof(short),
        sizeof(short int),
        sizeof(signed short),
        sizeof(signed short int),
        sizeof(unsigned short),
        sizeof(unsigned short int),
        sizeof(int),
        sizeof(signed int),
        sizeof(unsigned int),
        sizeof(long),
        sizeof(long int),
        sizeof(signed long),
        sizeof(signed long int),
        sizeof(unsigned long),
        sizeof(unsigned long int),
        sizeof(long long),
        sizeof(long long int),
        sizeof(signed long long),
        sizeof(signed long long int),
        sizeof(unsigned long long),
        sizeof(unsigned long long int),
        sizeof(float),
        sizeof(double),
};

short dataTypeIndex = 0;

bool mayBeNegative = true;
bool mayBeFloat = true;

short base = 10;
char input[10000] = {'\0'};
bool isNegative = false;

int inputLength = 0;
int step = 0;
bool isInputValid = false;

void extractMantissa(const char src[], char dist[], int *mantissa)
{
    int length = 0;
    int srcLength = 0;
    int mantissaIndexFromStart = -1;
    for (int i = 0; *(src + i) != '\0'; i++)
    {
        if (*(src + i) == '.')
        {
            mantissaIndexFromStart = i + 1;
        }
        if (*(src + i) != '-' && *(src + i) != '.')
        {
            *(dist + length) = *(src + i);
            *(dist + length + 1) = '\0';
            length++;
        }
        srcLength++;
    }
    if (mantissaIndexFromStart != -1)
    {
        *mantissa = srcLength - mantissaIndexFromStart;
    }
}

void changeRadix(char messySrc[], char decimalDest[], char binaryDest[])
{
    int mantissa = 0;
    char src[10000] = {'\0'};
    extractMantissa(messySrc, src, &mantissa);
    int length = 0;

    for (int i = 0; *(src + i) != '\0'; i++)
    {

        int one = 1;
        __asm__(
        "add %0, %1\n\t"
        : "+r"(one), "+r"(length));
    }

    union {
        char type0;
        signed char type1;
        short type2;
        short int type3;
        signed short type4;
        signed short int type5;
        unsigned short type6;
        unsigned short int type7;
        int type8;
        signed int type9;
        unsigned int type10;
        long type11;
        long int type12;
        signed long type13;
        signed long int type14;
        unsigned long type15;
        unsigned long int type16;
        long long type17;
        long long int type18;
        signed long long type19;
        signed long long int type20;
        unsigned long long type21;
        unsigned long long int type22;
        float type23;
        double type24;
        unsigned long long int sys;
    } decimalValue;

    decimalValue.type0 = 0;
    decimalValue.type1 = 0;
    decimalValue.type2 = 0;
    decimalValue.type3 = 0;
    decimalValue.type4 = 0;
    decimalValue.type5 = 0;
    decimalValue.type6 = 0;
    decimalValue.type7 = 0;
    decimalValue.type8 = 0;
    decimalValue.type9 = 0;
    decimalValue.type10 = 0;
    decimalValue.type11 = 0;
    decimalValue.type12 = 0;
    decimalValue.type13 = 0;
    decimalValue.type14 = 0;
    decimalValue.type15 = 0;
    decimalValue.type16 = 0;
    decimalValue.type17 = 0;
    decimalValue.type18 = 0;
    decimalValue.type19 = 0;
    decimalValue.type20 = 0;
    decimalValue.type21 = 0;
    decimalValue.type22 = 0;
    decimalValue.type23 = 0;
    decimalValue.type24 = 0;
    decimalValue.sys = 0;

    for (int i = 0; i < length; i++)
    {
        int symbolValue = 0;
        // Compare *srcstr with ASCII values
        if (*(src + i) >= '0' && *(src + i) <= '9') // is *srcstr Between 0-9
        {
            symbolValue = (((int)(*(src + i))) - '0');
        }
        else if ((*(src + i) >= 'A' && *(src + i) <= 'Z')) // is *srcstr Between A-Z
        {
            symbolValue = (((int)(*(src + i))) - 'A' + 10);
        }
        else if (*(src + i) >= 'a' && *(src + i) <= 'z') // is *srcstr Between a-z
        {
            symbolValue = (((int)(*(src + i))) - 'a' + 10);
        }

        long double addedValue = 0;

        if (i <= length - mantissa)
        {
            addedValue = symbolValue * pow(base, length - i - 1 - mantissa);
        }
        else
        {
            addedValue = symbolValue / pow(base, i);
        }
        if (dataTypeIndex == 0)
        {
            decimalValue.type0 = isNegative ? (decimalValue.type0 - addedValue) : (decimalValue.type0 + addedValue);
        }
        else if (dataTypeIndex == 1)
        {
            decimalValue.type1 = isNegative ? (decimalValue.type1 - addedValue) : (decimalValue.type1 + addedValue);
        }
        else if (dataTypeIndex == 2)
        {
            decimalValue.type2 = isNegative ? (decimalValue.type2 - addedValue) : (decimalValue.type2 + addedValue);
        }
        else if (dataTypeIndex == 3)
        {
            decimalValue.type3 = isNegative ? (decimalValue.type3 - addedValue) : (decimalValue.type3 + addedValue);
        }
        else if (dataTypeIndex == 4)
        {
            decimalValue.type4 = isNegative ? (decimalValue.type4 - addedValue) : (decimalValue.type4 + addedValue);
        }
        else if (dataTypeIndex == 5)
        {
            decimalValue.type5 = isNegative ? (decimalValue.type5 - addedValue) : (decimalValue.type5 + addedValue);
        }
        else if (dataTypeIndex == 6)
        {
            decimalValue.type6 = isNegative ? (decimalValue.type6 - addedValue) : (decimalValue.type6 + addedValue);
        }
        else if (dataTypeIndex == 7)
        {
            decimalValue.type7 = isNegative ? (decimalValue.type7 - addedValue) : (decimalValue.type7 + addedValue);
        }
        else if (dataTypeIndex == 8)
        {
            decimalValue.type8 = isNegative ? (decimalValue.type8 - addedValue) : (decimalValue.type8 + addedValue);
        }
        else if (dataTypeIndex == 9)
        {
            decimalValue.type9 = isNegative ? (decimalValue.type9 - addedValue) : (decimalValue.type9 + addedValue);
        }
        else if (dataTypeIndex == 10)
        {
            decimalValue.type10 = isNegative ? (decimalValue.type10 - addedValue) : (decimalValue.type10 + addedValue);
        }
        else if (dataTypeIndex == 11)
        {
            decimalValue.type11 = isNegative ? (decimalValue.type11 - addedValue) : (decimalValue.type11 + addedValue);
        }
        else if (dataTypeIndex == 12)
        {
            decimalValue.type12 = isNegative ? (decimalValue.type12 - addedValue) : (decimalValue.type12 + addedValue);
        }
        else if (dataTypeIndex == 13)
        {
            decimalValue.type13 = isNegative ? (decimalValue.type13 - addedValue) : (decimalValue.type13 + addedValue);
        }
        else if (dataTypeIndex == 14)
        {
            decimalValue.type14 = isNegative ? (decimalValue.type14 - addedValue) : (decimalValue.type14 + addedValue);
        }
        else if (dataTypeIndex == 15)
        {
            decimalValue.type15 = isNegative ? (decimalValue.type15 - addedValue) : (decimalValue.type15 + addedValue);
        }
        else if (dataTypeIndex == 16)
        {
            decimalValue.type16 = isNegative ? (decimalValue.type16 - addedValue) : (decimalValue.type16 + addedValue);
        }
        else if (dataTypeIndex == 17)
        {
            decimalValue.type17 = isNegative ? (decimalValue.type17 - addedValue) : (decimalValue.type17 + addedValue);
        }
        else if (dataTypeIndex == 18)
        {
            decimalValue.type18 = isNegative ? (decimalValue.type18 - addedValue) : (decimalValue.type18 + addedValue);
        }
        else if (dataTypeIndex == 19)
        {
            decimalValue.type19 = isNegative ? (decimalValue.type19 - addedValue) : (decimalValue.type19 + addedValue);
        }
        else if (dataTypeIndex == 20)
        {
            decimalValue.type20 = isNegative ? (decimalValue.type20 - addedValue) : (decimalValue.type20 + addedValue);
        }
        else if (dataTypeIndex == 21)
        {
            decimalValue.type21 = isNegative ? (decimalValue.type21 - addedValue) : (decimalValue.type21 + addedValue);
        }
        else if (dataTypeIndex == 22)
        {
            decimalValue.type22 = isNegative ? (decimalValue.type22 - addedValue) : (decimalValue.type22 + addedValue);
        }
        else if (dataTypeIndex == 23)
        {
            decimalValue.type23 = isNegative ? (decimalValue.type23 - addedValue) : (decimalValue.type23 + addedValue);
        }
        else if (dataTypeIndex == 24)
        {
            decimalValue.type24 = isNegative ? (decimalValue.type24 - addedValue) : (decimalValue.type24 + addedValue);
        }
    }


    std::stringstream ioStream;

    ioStream.precision(16);

    if (dataTypeIndex == 0)
    {
        ioStream << decimalValue.sys << ':' << decimalValue.type0;
    }
    else if (dataTypeIndex == 1)
    {
        ioStream << decimalValue.sys << ':' << decimalValue.type1;
    }
    else if (dataTypeIndex == 2)
    {
        ioStream << decimalValue.type2;
    }
    else if (dataTypeIndex == 3)
    {
        ioStream << decimalValue.type3;
    }
    else if (dataTypeIndex == 4)
    {
        ioStream << decimalValue.type4;
    }
    else if (dataTypeIndex == 5)
    {
        ioStream << decimalValue.type5;
    }
    else if (dataTypeIndex == 6)
    {
        ioStream << decimalValue.type6;
    }
    else if (dataTypeIndex == 7)
    {
        ioStream << decimalValue.type7;
    }
    else if (dataTypeIndex == 8)
    {
        ioStream << decimalValue.type8;
    }
    else if (dataTypeIndex == 9)
    {
        ioStream << decimalValue.type9;
    }
    else if (dataTypeIndex == 10)
    {
        ioStream << decimalValue.type10;
    }
    else if (dataTypeIndex == 11)
    {
        ioStream << decimalValue.type11;
    }
    else if (dataTypeIndex == 12)
    {
        ioStream << decimalValue.type12;
    }
    else if (dataTypeIndex == 13)
    {
        ioStream << decimalValue.type13;
    }
    else if (dataTypeIndex == 14)
    {
        ioStream << decimalValue.type14;
    }
    else if (dataTypeIndex == 15)
    {
        ioStream << decimalValue.type15;
    }
    else if (dataTypeIndex == 16)
    {
        ioStream << decimalValue.type16;
    }
    else if (dataTypeIndex == 17)
    {
        ioStream << decimalValue.type17;
    }
    else if (dataTypeIndex == 18)
    {
        ioStream << decimalValue.type18;
    }
    else if (dataTypeIndex == 19)
    {
        ioStream << decimalValue.type19;
    }
    else if (dataTypeIndex == 20)
    {
        ioStream << decimalValue.type20;
    }
    else if (dataTypeIndex == 21)
    {
        ioStream << decimalValue.type21;
    }
    else if (dataTypeIndex == 22)
    {
        ioStream << decimalValue.type22;
    }
    else if (dataTypeIndex == 23)
    {
        ioStream << decimalValue.type23;
    }
    else if (dataTypeIndex == 24)
    {
        ioStream << decimalValue.type24;
    }

    ioStream >> decimalDest;

    int binaryLength = varSize[dataTypeIndex] * CHAR_BIT;
    *(binaryDest + binaryLength + 1) = '\0';
    for (int i = 0; i < binaryLength; i++)
    {
        *(binaryDest + binaryLength - i - 1) = ((decimalValue.sys >> i) & 0x1) == 1 ? '1' : '0';
    }
}

void printLabel(char label[], int x, int y)
{
    for (int i = 0; label[i] != '\0'; i++)
    {
        moveCursor(i + x, y);
        putchar(label[i]);
    }
}

void promptDataType()
{
    std::system("clear");
    char label0[] = { 'S', 'e', 'l', 'e', 'c', 't', ' ', 'd', 'a', 't', 'a', ' ', 't', 'y', 'p', 'e', ':', '\0'};
    printLabel(label0, 0, 0);
    char label1[] = {'1', ':', ' ', 'c', 'h', 'a', 'r', '\0'};
    printLabel(label1, 2, 2);
    char label2[] = {'2', ':', ' ', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'c', 'h', 'a', 'r', '\0'};
    printLabel(label2, 2, 3);
    char label3[] = {'3', ':', ' ', 's', 'h', 'o', 'r', 't', '\0'};
    printLabel(label3, 2, 4);
    char label4[] = {'4', ':', ' ', 's', 'h', 'o', 'r', 't', ' ', 'i', 'n', 't', '\0'};
    printLabel(label4, 2, 5);
    char label5[] = {'5', ':', ' ', 's', 'i', 'g', 'n', 'e', 'd', ' ', 's', 'h', 'o', 'r', 't', '\0'};
    printLabel(label5, 2, 6);
    char label6[] = {'6', ':', ' ', 's', 'i', 'g', 'n', 'e', 'd', ' ', 's', 'h', 'o', 'r', 't', ' ', 'i', 'n', 't', '\0'};
    printLabel(label6, 2, 7);
    char label7[] = {'7', ':', ' ', 'u', 'n', 's', 'i', 'g', 'n', 'e', 'd', ' ', 's', 'h', 'o', 'r', 't', '\0'};
    printLabel(label7, 2, 8);
    char label8[] = {'8', ':', ' ', 'u', 'n', 's', 'i', 'g', 'n', 'e', 'd', ' ', 's', 'h', 'o', 'r', 't', ' ', 'i', 'n', 't', '\0'};
    printLabel(label8, 2, 9);
    char label9[] = {'9', ':', ' ', 'i', 'n', 't', '\0'};
    printLabel(label9, 2, 10);
    char label10[] = {'1', '0', ':', ' ', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'i', 'n', 't', '\0'};
    printLabel(label10, 2, 11);
    char label11[] = {'1', '1', ':', ' ', 'u', 'n', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'i', 'n', 't', '\0'};
    printLabel(label11, 2, 12);
    char label12[] = {'1', '2', ':', ' ', 'l', 'o', 'n', 'g', '\0'};
    printLabel(label12, 2, 13);
    char label13[] = {'1', '3', ':', ' ', 'l', 'o', 'n', 'g', ' ', 'i', 'n', 't', '\0'};
    printLabel(label13, 2, 14);
    char label14[] = {'1', '4', ':', ' ', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'l', 'o', 'n', 'g', '\0'};
    printLabel(label14, 20, 2);
    char label15[] = {'1', '5', ':', ' ', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'l', 'o', 'n', 'g', ' ', 'i', 'n', 't', '\0'};
    printLabel(label15, 20, 3);
    char label16[] = {'1', '6', ':', ' ', 'u', 'n', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'l', 'o', 'n', 'g', '\0'};
    printLabel(label16, 20, 4);
    char label17[] = {'1', '7', ':', ' ', 'u', 'n', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'l', 'o', 'n', 'g', ' ', 'i', 'n', 't', '\0'};
    printLabel(label17, 20, 5);
    char label18[] = {'1', '8', ':', ' ', 'l', 'o', 'n', 'g', ' ', 'l', 'o', 'n', 'g', '\0'};
    printLabel(label18, 20, 6);
    char label19[] = {'1', '9', ':', ' ', 'l', 'o', 'n', 'g', ' ', 'l', 'o', 'n', 'g', ' ', 'i', 'n', 't', '\0'};
    printLabel(label19, 20, 7);
    char label20[] = {'2', '0', ':', ' ', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'l', 'o', 'n', 'g', ' ', 'l', 'o', 'n', 'g', '\0'};
    printLabel(label20, 20, 8);
    char label21[] = {'2', '1', ':', ' ', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'l', 'o', 'n', 'g', ' ', 'l', 'o', 'n', 'g', ' ', 'i', 'n', 't', '\0'};
    printLabel(label21, 20, 9);
    char label22[] = {'2', '2', ':', ' ', 'u', 'n', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'l', 'o', 'n', 'g', ' ', 'l', 'o', 'n', 'g', '\0'};
    printLabel(label22, 20, 10);
    char label23[] = {'2', '3', ':', ' ', 'u', 'n', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'l', 'o', 'n', 'g', ' ', 'l', 'o', 'n', 'g', ' ', 'i', 'n', 't', '\0'};
    printLabel(label23, 20, 11);
    char label24[] = {'2', '4', ':', ' ', 'f', 'l', 'o', 'a', 't', '\0'};
    printLabel(label24, 20, 12);
    char label25[] = {'2', '5', ':', ' ', 'd', 'o', 'u', 'b', 'l', 'e', '\0'};
    printLabel(label25, 20, 13);

    char label30[] = {'E', 'n', 't', 'e', 'r', ' ', 't', 'h', 'e', ' ', 'd', 'a', 't', 'a', ' ', 't', 'y', 'p', 'e', ':', ' ', '\0'};
    printLabel(label30, 2, 16);

    dataTypeIndex = rangeInput(2, 17, 1, 25, true) - 1;

    char emptyString[] = {' ',' ',' ',' ',' ',' ',' ',' ',' ', ' ', ' ', ' ', ' ', '\0'};
    printLabel(emptyString, 2, 17);

    mayBeNegative = isNegativeMap[dataTypeIndex];
    mayBeFloat = isFloatMap[dataTypeIndex];
    step = 1;
}

char decimal[10000] = {'\0'};
char binary[10000] = {'\0'};

void prompt()
{
    std::system("clear");

    moveCursor(0, 10);

    changeRadix(input, decimal, binary);
    char label1[] = {'I', 'n', 'p', 'u', 't', ' ', 'n', 'u', 'm', 'b', 'e', 'r', ':', ' ', '\0'};
    printLabel(label1, 0, 1);

    for (int i = 0; input[i] != '\0'; i++)
    {
        moveCursor(i + 16, 1);
        putchar(input[i]);
    }
    if (isNegative && inputLength > 0)
    {
        moveCursor(15, 1);
        putchar('-');
    }
    moveCursor(0, 5);
    std::cout << "Available symbols:";
    for (int i = 0; i < base; i++)
    {
        moveCursor((i + 2) * 2, 6);
        if (i < 10)
        {
            putchar(i + '0');
        }
        else
        {
            putchar(i - 10 + 'A');
        }
    }
    if (mayBeNegative)
    {
        moveCursor(2, 6);
        putchar('-');
    }
    if (mayBeFloat)
    {
        moveCursor(0, 6);
        putchar('.');
    }
    moveCursor(0, 7);
    std::cout << "Input number base: " << base << std::flush;
    moveCursor(0, 8);
    std::cout << "Using data type: " << dataTypeNames[dataTypeIndex] << std::flush;

    moveCursor(0, 27);
    std::cout << "Use Tab to open previous (base input) screen";
    moveCursor(0, 28);
    std::cout << "Use Enter to restart/exit program";
    moveCursor(0, 10);


    moveCursor(16 + inputLength, 1);
}

void renderResult() {
    char label2[] = {'D', 'e', 'c', 'i', 'm', 'a', 'l', ':', ' ', '\0'};
    printLabel(label2, 0, 2);
    char label3[] = {'B', 'i', 'n', 'a', 'r', 'y', ':', ' ', '\0'};
    printLabel(label3, 0, 3);

    int binaryLength = 0;
    while (binary[binaryLength] != '\0')
        binaryLength++;


    printLabel(binary, 16, 3);

    printLabel(decimal, 16, 2);
}

void getBase()
{
    std::system("clear");
    char label[] = {'I', 'n', 'p', 'u', 't', ' ', 'b', 'a', 's', 'e', ':', ' ', '\0'};
    printLabel(label, 0, 0);
    moveCursor(0, 3);
    std::cout << "Use Tab to open previous screen";
    base = rangeInput(13, 0, 2, 36, false);
    if (base == -1)
    {
        step = 0;
        return;
    }
    step = 2;
    return;
}

int main()
{

    char inputSymbol = '0';
    int floatDelimeter = -1;

    while (input[inputLength] != '\0')
    {
        inputLength++;
    }

    while (true)
    {
        if (step == 0)
        {
            promptDataType();
            std::system("clear");
        }
        else if (step == 1)
        {
            std::system("clear");
            getBase();
            inputLength = 0;
            input[0] = '\0';
            isNegative = false;
            floatDelimeter = -1;
            isInputValid = false;
        }
        else if (step == 2)
        {
            prompt();
            inputSymbol = getch();
            if (inputSymbol == '\t')
            {
                std::system("clear");
                step = 1;
                continue;
            }
            else if (inputSymbol == '\n')
            {
                if (isInputValid) {
                    step = 3;
                }
            }
            else if (inputSymbol == 127)
            {
                if (inputLength > 0)
                {
                    inputLength--;
                }
                if (floatDelimeter >= inputLength)
                {
                    floatDelimeter = -1;
                }
                input[inputLength] = '\0';
            }
            else if (mayBeNegative && inputSymbol == '-')
            {
                isNegative = !isNegative;
            }
            else if ((inputSymbol >= '0' && (inputSymbol - '0' < base)) || (inputSymbol >= 'a' && inputSymbol - 'a' < base - 10))
            {
                input[inputLength] = inputSymbol;
                input[inputLength + 1] = '\0';
                inputLength++;
            }
            else if (mayBeFloat && (inputSymbol == '.' || inputSymbol == ',') && ((inputLength > 0 && input[0] != '-') || inputLength > 1) && floatDelimeter == -1)
            {
                floatDelimeter = inputLength;
                input[inputLength] = '.';
                input[inputLength + 1] = '\0';
                inputLength++;
            }
            if (inputLength == floatDelimeter + 1)
            {
                isInputValid = false;
            }
            else
            {
                isInputValid = true;
            }
        } if (step == 3)
        {
            renderResult();
            inputSymbol = getch();
            if (inputSymbol == '\n')
            {
                std::system("clear");
                moveCursor(0, 0);
                std::cout << "Restart/Exit (r/e)? ";
                while (inputSymbol != 'r' && inputSymbol != 'e')
                    inputSymbol = getch();
                if (inputSymbol == 'r')
                {
                    step = 0;
                }
                else
                {
                    std::system("clear");
                    return 0;
                }
            }
        }
    }
    return 0;
}
