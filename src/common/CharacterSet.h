//
// Created by david on 13/06/23.
//

#ifndef LIBCOS_CHARACTER_SET_H
#define LIBCOS_CHARACTER_SET_H

#include <stdbool.h>

enum CosCharacterSet {
    CosCharacterSet_Nul = 0x00,

    /**
     * @brief '\\b', or backspace.
     */
    CosCharacterSet_Backspace = 0x08,

    /**
     * @brief '\\t', or horizontal tab.
     */
    CosCharacterSet_HorizontalTab = 0x09,
    CosCharacterSet_LineFeed = 0x0A,
    CosCharacterSet_LineTabulation = 0x0B,
    CosCharacterSet_FormFeed = 0x0C,
    CosCharacterSet_CarriageReturn = 0x0D,
    CosCharacterSet_Space = 0x20,

    CosCharacterSet_NumberSign = 0x23,
    CosCharacterSet_PercentSign = 0x25,

    CosCharacterSet_LeftParenthesis = 0x28,
    CosCharacterSet_RightParenthesis = 0x29,

    CosCharacterSet_Asterisk = 0x2A,
    CosCharacterSet_PlusSign = 0x2B,
    CosCharacterSet_Comma = 0x2C,
    CosCharacterSet_HyphenMinus = 0x2D,
    CosCharacterSet_FullStop = 0x2E,
    CosCharacterSet_Solidus = 0x2F,

    CosCharacterSet_DigitZero = 0x30,
    CosCharacterSet_DigitOne = 0x31,
    CosCharacterSet_DigitTwo = 0x32,
    CosCharacterSet_DigitThree = 0x33,
    CosCharacterSet_DigitFour = 0x34,
    CosCharacterSet_DigitFive = 0x35,
    CosCharacterSet_DigitSix = 0x36,
    CosCharacterSet_DigitSeven = 0x37,
    CosCharacterSet_DigitEight = 0x38,
    CosCharacterSet_DigitNine = 0x39,

    /**
     * @brief '<'
     */
    CosCharacterSet_LessThanSign = 0x3C,

    /**
     * @brief '>'
     */
    CosCharacterSet_GreaterThanSign = 0x3E,

    CosCharacterSet_LatinCapitalLetterA = 0x41,
    CosCharacterSet_LatinCapitalLetterB = 0x42,
    CosCharacterSet_LatinCapitalLetterC = 0x43,
    CosCharacterSet_LatinCapitalLetterD = 0x44,
    CosCharacterSet_LatinCapitalLetterE = 0x45,
    CosCharacterSet_LatinCapitalLetterF = 0x46,

    CosCharacterSet_LatinSmallLetterA = 0x61,
    CosCharacterSet_LatinSmallLetterB = 0x62,
    CosCharacterSet_LatinSmallLetterC = 0x63,
    CosCharacterSet_LatinSmallLetterD = 0x64,
    CosCharacterSet_LatinSmallLetterE = 0x65,
    CosCharacterSet_LatinSmallLetterF = 0x66,

    CosCharacterSet_LeftSquareBracket = 0x5B,
    CosCharacterSet_ReverseSolidus = 0x5C,
    CosCharacterSet_RightSquareBracket = 0x5D,

    CosCharacterSet_LeftCurlyBracket = 0x7B,
    CosCharacterSet_RightCurlyBracket = 0x7D,
};

/**
 * @brief Check if a character is whitespace.
 *
 * @param character The character to check.
 *
 * @return true if the character is whitespace, false otherwise.
 */
bool
cos_is_whitespace(int character);

/**
 * @brief Check if a character is a delimiter.
 *
 * @param character The character to check.
 *
 * @return true if the character is a delimiter, false otherwise.
 */
bool
cos_is_delimiter(int character);

bool
cos_is_end_of_line(int character);

bool
cos_is_hex_digit(int character);

bool
cos_is_octal_digit(int character);

bool
cos_is_decimal_digit(int character);

#endif /* LIBCOS_CHARACTER_SET_H */
