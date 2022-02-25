#pragma once

int myfunc();

void encodeTagAndValue(unsigned char* buffer, int tag, int length, unsigned long value);

void encodeTagAndN(unsigned char* buffer, int majorType, unsigned long n);
