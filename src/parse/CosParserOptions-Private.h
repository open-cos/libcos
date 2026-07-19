/*
 * Copyright (c) 2026 OpenCOS.
 */

#ifndef LIBCOS_PARSE_COS_PARSER_OPTIONS_PRIVATE_H
#define LIBCOS_PARSE_COS_PARSER_OPTIONS_PRIVATE_H

#include <libcos/common/CosDefines.h>
#include <libcos/parse/CosParserOptions.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * Resolves an optional options pointer to a concrete options value.
 *
 * Every entry point that accepts options funnels through this, so that a
 * @c NULL pointer means the defaults in exactly one place.
 *
 * @param options The options to resolve, or @c NULL for the defaults.
 *
 * @return A copy of @p options , or the default options if @p options is @c NULL .
 */
CosParserOptions
cos_parser_options_resolve_(const CosParserOptions * COS_Nullable options);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_PARSE_COS_PARSER_OPTIONS_PRIVATE_H */
