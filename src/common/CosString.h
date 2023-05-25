//
// Created by david on 24/05/23.
//

#ifndef LIBCOS_COS_STRING_H
#define LIBCOS_COS_STRING_H

typedef struct CosString CosString;

CosString *
cos_string_alloc(void);

void
cos_string_free(CosString *string);

#endif /* LIBCOS_COS_STRING_H */
