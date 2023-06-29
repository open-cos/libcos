//
// Created by david on 19/06/23.
//
#include "CharacterSet.h"

bool
cos_is_whitespace(char character)
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
cos_is_delimiter(char character)
{
    switch (character) {
        case CosCharacterSet_LeftParenthesis:
        case CosCharacterSet_RightParenthesis:
        case CosCharacterSet_LessThanSign:
        case CosCharacterSet_GreaterThanSign:
        case CosCharacterSet_LeftSquareBracket:
        case CosCharacterSet_RightSquareBracket:
        case CosCharacterSet_LeftCurlyBracket:
        case CosCharacterSet_RightCurlyBracket:
        case CosCharacterSet_Solidus:
        case CosCharacterSet_PercentSign:
            return true;

        default:
            return false;
    }
}

bool
cos_is_end_of_line(char character)
{
    switch (character) {
        case CosCharacterSet_LineFeed:
        case CosCharacterSet_CarriageReturn:
            return true;

        default:
            return false;
    }
}
