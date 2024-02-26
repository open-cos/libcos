# TODO

### Adobe compatibility

- Ignore minus signs in the middle of (the fractional part of real) numbers to match Adobe's behavior
- Acrobat ignores real number overflow: eg. 123450000000000000000678 is read as 678

- Adobe does not seem to enforce the "%%EOF" marker at the end of the file

- Adobe looks in the first 1024 bytes of a file for the "%PDF-" header
- Adobe looks in the last 1024 bytes of a file for the "%%EOF" marker

- Adobe ignores the generation number on compressed (xref) objects
- Acrobat limits the number of objects in object streams to 100-200 objects
- 
- Empty (indirect) objects are silently ignored by Adobe and treated as null?

- Adobe Distiller 8 and Acrobat 8 produce and accept name objects longer than 127 bytes
