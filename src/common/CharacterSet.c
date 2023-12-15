//
// Created by david on 19/06/23.
//
#include "CharacterSet.h"

bool
cos_is_whitespace(int character)
{
    switch (character) {
        case CosCharacterSet_Nul:
        case CosCharacterSet_HorizontalTab:
        case CosCharacterSet_LineFeed:
        case CosCharacterSet_FormFeed:
        case CosCharacterSet_CarriageReturn:
        case CosCharacterSet_Space:
            return true;

        default:
            return false;
    }
}

bool
cos_is_delimiter(int character)
{
    switch (character) {
        case CosCharacterSet_PercentSign:
        case CosCharacterSet_LeftParenthesis:
        case CosCharacterSet_RightParenthesis:
        case CosCharacterSet_Solidus:
        case CosCharacterSet_LessThanSign:
        case CosCharacterSet_GreaterThanSign:
        case CosCharacterSet_LeftSquareBracket:
        case CosCharacterSet_RightSquareBracket:
        case CosCharacterSet_LeftCurlyBracket:
        case CosCharacterSet_RightCurlyBracket:
            return true;

        default:
            return false;
    }
}

bool
cos_is_end_of_line(int character)
{
    switch (character) {
        case CosCharacterSet_LineFeed:
        case CosCharacterSet_CarriageReturn:
            return true;

        default:
            return false;
    }
}

bool
cos_is_hex_digit(int character)
{
    switch (character) {
        case CosCharacterSet_DigitZero:
        case CosCharacterSet_DigitOne:
        case CosCharacterSet_DigitTwo:
        case CosCharacterSet_DigitThree:
        case CosCharacterSet_DigitFour:
        case CosCharacterSet_DigitFive:
        case CosCharacterSet_DigitSix:
        case CosCharacterSet_DigitSeven:
        case CosCharacterSet_DigitEight:
        case CosCharacterSet_DigitNine:
        case CosCharacterSet_LatinCapitalLetterA:
        case CosCharacterSet_LatinCapitalLetterB:
        case CosCharacterSet_LatinCapitalLetterC:
        case CosCharacterSet_LatinCapitalLetterD:
        case CosCharacterSet_LatinCapitalLetterE:
        case CosCharacterSet_LatinCapitalLetterF:
        case CosCharacterSet_LatinSmallLetterA:
        case CosCharacterSet_LatinSmallLetterB:
        case CosCharacterSet_LatinSmallLetterC:
        case CosCharacterSet_LatinSmallLetterD:
        case CosCharacterSet_LatinSmallLetterE:
        case CosCharacterSet_LatinSmallLetterF:
            return true;

        default:
            return false;
    }
}

bool
cos_is_octal_digit(int character)
{
    switch (character) {
        case CosCharacterSet_DigitZero:
        case CosCharacterSet_DigitOne:
        case CosCharacterSet_DigitTwo:
        case CosCharacterSet_DigitThree:
        case CosCharacterSet_DigitFour:
        case CosCharacterSet_DigitFive:
        case CosCharacterSet_DigitSix:
        case CosCharacterSet_DigitSeven:
            return true;

        default:
            return false;
    }
}

bool
cos_is_decimal_digit(int character)
{
    switch (character) {
        case CosCharacterSet_DigitZero:
        case CosCharacterSet_DigitOne:
        case CosCharacterSet_DigitTwo:
        case CosCharacterSet_DigitThree:
        case CosCharacterSet_DigitFour:
        case CosCharacterSet_DigitFive:
        case CosCharacterSet_DigitSix:
        case CosCharacterSet_DigitSeven:
        case CosCharacterSet_DigitEight:
        case CosCharacterSet_DigitNine:
            return true;

        default:
            return false;
    }
}
