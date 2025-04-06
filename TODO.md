# TODO

### Adobe compatibility

- Ignore minus signs in the middle of (the fractional part of real) numbers to match Adobe's behavior
- Acrobat ignores real number overflow: eg. 123450000000000000000678 is read as 678

- Adobe does not seem to enforce the "%%EOF" marker at the end of the file

- Adobe looks in the first 1024 bytes of a file for the "%PDF-" header
- Adobe looks in the last 1024 bytes of a file for the "%%EOF" marker
- Adobe accepts a file header of the form "%!PS−Adobe−N.n PDF−M.m"
- 
- PDF 1.7, Appendix H.3, Implementation Note 19:
  - "When creating or saving PDF files, Acrobat 6.0 limits the number of objects in individual object streams to 100 for linearized files and 200 for non-linearized files"

- Adobe ignores the generation number on compressed (xref) objects

- Empty (indirect) objects are silently ignored by Adobe and treated as null?

- Adobe Distiller 8 and Acrobat 8 produce and accept name objects longer than 127 bytes

### Other little requirements

- PDF 1.7 (ISO 32000-1:2008), Section 7.5.8.4:
    > "PDF comments shall not be included in a cross-reference table or in cross-reference streams."

### Top-level parser

- Read the high-level file structure
  - Read the file header
  - Skip to the end of the file
  - Read the file trailer
    - This is likely to be tricky, since it requires us to read the end of the file backwards.
    - The last line of the file is "%%EOF", with possibly some trailing whitespace?
    - The preceding two lines are the "startxref" keyword and the byte-offset of the last xref section.
      - Does this change if the file is using XRef streams?
