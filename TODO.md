# TODO

### Adobe compatibility

Most of the items that were here are done. Each implemented one is gated on a
`CosStrictGroup` (see `include/libcos/parse/CosParserOptions.h`): the lenient,
Adobe-compatible behaviour applies at `CosStrictLevel_Off` and
`CosStrictLevel_Warn`, and `CosStrictLevel_Error` restores the strict reading.

#### Done

- Ignore minus signs in the middle of numbers -- `CosStrictGroup_NumberSigns`.
  `1.2-3` reads as `1.23` and `-12.-1` as `-12.1`. When only zeros precede the
  sign it is taken as the number's sign instead, so `0.00000-33917698` is
  negative and `--16.33` is `-16.33`.

  This follows PDFBox's `COSFloat`, which special-cases these exact shapes
  (PDFBOX-2990, PDFBOX-4289, PDFBOX-5829). **Ghostscript does the opposite**:
  `pdfi_read_num` flags `E_PDF_MALFORMEDNUMBER` and ignores everything from the
  interior sign onward, so it reads `123-56` as `123`. That is also what libcos
  did before, and it is what `CosStrictLevel_Error` still does.

- Adobe looks in the first 1024 bytes of a file for the "%PDF-" header --
  `CosStrictGroup_HeaderPosition`. `COS_HEADER_SCAN_SIZE` in
  `src/parse/CosParser.c`.

- Adobe does not seem to enforce the "%%EOF" marker at the end of the file --
  `CosStrictGroup_EofMarker`. When the marker is absent the scan anchors on the
  last `startxref` keyword instead. A file with neither still fails.

- Adobe ignores the generation number on compressed (xref) objects --
  `CosStrictGroup_CompressedObjGen`, in `cos_doc_get_object`.

- Adobe looks in the last 1024 bytes of a file for the "%%EOF" marker -- this
  was already the behaviour of `cos_parser_find_startxref_`; the backward scan
  window has always been 1024 bytes.

- Adobe Distiller 8 and Acrobat 8 produce and accept name objects longer than
  127 bytes -- already the behaviour. `COS_NAME_MAX_LENGTH` is declared in
  `CosLimits.h` but never enforced anywhere, so long names already parse.

#### Declined

- ~~Acrobat ignores real number overflow: eg. 123450000000000000000678 is read
  as 678~~

  Not implemented, deliberately. libcos currently reads that input as
  `1.2345e+23`, which is correct, and reproducing Adobe here would mean
  silently substituting a wrong value for a right one. No caller benefits from
  bug-compatibility at that cost.

  The line came from Ghostscript, whose `pdfi_read_num` in `pdf/pdf_int.c`
  carries the comment nearly verbatim:

  > We deliberately ignore overflow here. Tests show that Acrobat handles
  > overflows in exactly the same way we do: 123450000000000000000678 is read
  > as 678.

  Worth noting that Ghostscript does not silently truncate: it raises
  `E_PDF_NUMBEROVERFLOW` and stops accumulating digits. PDFBox's `COSFloat`
  takes a third approach, clamping out-of-range values to `Float.MAX_VALUE`,
  which is what ISO 32000-1 Annex C suggests. So the three major
  implementations disagree, and none of them treats "678" as a feature to
  preserve.

  If a conformance mode is wanted later, the useful shape is a group that
  rejects an out-of-range real at `CosStrictLevel_Error` while leaving the
  correct parse in place at the lower levels -- not one that reproduces the
  truncation.

#### Not actionable

- Acrobat limits the number of objects in object streams to 100-200 objects.
  An observation about Acrobat's own limits, not a behaviour to match. libcos
  imposes no such limit, and adding one would reject valid files.

### Open questions

- Empty (indirect) objects are silently ignored by Adobe and treated as null?
  The question mark is from the original note and the premise is unverified.
  Worth confirming against a real Acrobat build before implementing, since it
  would become another leniency group.
