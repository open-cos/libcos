/*
 * Copyright (c) 2026 OpenCOS.
 */

#ifndef LIBCOS_PARSE_COS_PARSER_OPTIONS_PRIVATE_H
#define LIBCOS_PARSE_COS_PARSER_OPTIONS_PRIVATE_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>
#include <libcos/parse/CosParserOptions.h>

#include <stdbool.h>

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

/**
 * Reports a deviation from the specification under a strict-mode group.
 *
 * Applies the group's level: silent at @c CosStrictLevel_Off , a warning
 * diagnostic at @c CosStrictLevel_Warn , and an error diagnostic plus a
 * @c COS_ERROR_SYNTAX propagation at @c CosStrictLevel_Error .
 *
 * @param options The options whose group levels apply.
 * @param handler Where to send the diagnostic, or @c NULL to use the default
 * handler.
 * @param group The group governing this deviation.
 * @param message The message to report. Diagnostics do not copy it, so it must
 * be a string literal or otherwise outlive the call.
 * @param out_error Set if the group escalates the deviation to an error.
 *
 * @return @c true if the caller may continue, @c false if the deviation was
 * escalated to an error and the caller must abort.
 */
bool
cos_options_report_(const CosParserOptions *options,
                    CosDiagnosticHandler * COS_Nullable handler,
                    CosStrictGroup group,
                    const char *message,
                    CosError * COS_Nullable out_error)
    COS_ATTR_ACCESS_WRITE_ONLY(5);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_PARSE_COS_PARSER_OPTIONS_PRIVATE_H */
