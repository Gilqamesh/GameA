#include <iostream>

using namespace std;

#define LOG(x) (cout << x << endl)
#define ArrayCount(Array) sizeof(Array)/(sizeof(Array[0]))

void PrintArray(int *Array, int Size)
{
    for (int i = 0; i < Size; ++i)
    {
        cout << Array[i] << " ";
    }
    cout << endl;
}

int main()
{
    int Data[] = {
        62, 12, 917, 327, 8765, 51654, 1, 0, 123, 4325, 12325
    };
    int Aux[ArrayCount(Data)] = {};

    // in base 2
    int NumberOfDigits = 29;
    for (int DigitIndex = 0; DigitIndex < NumberOfDigits; ++DigitIndex)
    {
        int DigitFlag = (1 << DigitIndex);
        int Keys[2] = {};
        for (int Index = 0; Index < ArrayCount(Data); ++Index)
        {
            ++Keys[((Data[Index] & DigitFlag) == DigitFlag)];
        }
        for (int Index = 1; Index < ArrayCount(Keys); ++Index)
        {
            Keys[Index] += Keys[Index - 1];
        }
        for (int Index = ArrayCount(Data) - 1; Index >= 0; --Index)
        {
            Aux[--Keys[((Data[Index] & DigitFlag) == DigitFlag)]] = Data[Index];
        }
        for (int Index = 0; Index < ArrayCount(Data); ++Index)
        {
            Data[Index] = Aux[Index];
        }
        PrintArray(Data, ArrayCount(Data));
    }

    PrintArray(Data, ArrayCount(Data));
}
