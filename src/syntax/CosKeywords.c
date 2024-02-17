/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "CosKeywords.h"

#include <libcos/common/CosString.h>

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

CosKeywordType
cos_keyword_type_from_string(CosStringRef string)
{
    // The longest keywords are 9 characters long.
    if (string.length == 0 || string.length > 9) {
        return CosKeywordType_Unknown;
    }

    switch (string.data[0]) {
        case 't': {
            if (cos_string_ref_cmp(string, cos_string_ref_const("true")) == 0) {
                return CosKeywordType_True;
            }
            else if (cos_string_ref_cmp(string, cos_string_ref_const("trailer")) == 0) {
                return CosKeywordType_Trailer;
            }
        } break;
        case 'f': {
            if (cos_string_ref_cmp(string, cos_string_ref_const("false")) == 0) {
                return CosKeywordType_False;
            }
        } break;
        case 'n': {
            if (cos_string_ref_cmp(string, cos_string_ref_const("null")) == 0) {
                return CosKeywordType_Null;
            }
        } break;
        case 'R': {
            if (string.length == 1) {
                return CosKeywordType_R;
            }
        } break;
        case 'o': {
            if (cos_string_ref_cmp(string, cos_string_ref_const("obj")) == 0) {
                return CosKeywordType_Obj;
            }
        } break;
        case 'e': {
            if (cos_string_ref_cmp(string, cos_string_ref_const("endobj")) == 0) {
                return CosKeywordType_EndObj;
            }
            else if (cos_string_ref_cmp(string, cos_string_ref_const("endstream")) == 0) {
                return CosKeywordType_EndStream;
            }
        } break;
        case 's': {
            if (cos_string_ref_cmp(string, cos_string_ref_const("stream")) == 0) {
                return CosKeywordType_Stream;
            }
            else if (cos_string_ref_cmp(string, cos_string_ref_const("startxref")) == 0) {
                return CosKeywordType_StartXRef;
            }
        } break;
        case 'x': {
            if (cos_string_ref_cmp(string, cos_string_ref_const("xref")) == 0) {
                return CosKeywordType_XRef;
            }
        } break;

        default:
            break;
    }

    return CosKeywordType_Unknown;
}

const char *
cos_keyword_type_to_string(CosKeywordType keyword_type)
{
    switch (keyword_type) {
        case CosKeywordType_Unknown: {
            return NULL;
        }

        case CosKeywordType_True: {
            return "true";
        }
        case CosKeywordType_False: {
            return "false";
        }
        case CosKeywordType_Null: {
            return "null";
        }
        case CosKeywordType_R: {
            return "R";
        }
        case CosKeywordType_Obj: {
            return "obj";
        }
        case CosKeywordType_EndObj: {
            return "endObj";
        }
        case CosKeywordType_Stream: {
            return "stream";
        }
        case CosKeywordType_EndStream: {
            return "endStream";
        }
        case CosKeywordType_XRef: {
            return "xref";
        }
        case CosKeywordType_Trailer: {
            return "trailer";
        }
        case CosKeywordType_StartXRef: {
            return "startxref";
        }
    }

    return NULL;
}

COS_ASSUME_NONNULL_END
