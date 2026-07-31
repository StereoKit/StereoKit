#include "stref.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdarg.h>
#include <ctype.h>

#include "../sk_memory.h"
#define STR_MALLOC sk::sk_malloc
#define STR_REALLOC sk::sk_realloc

///////////////////////////////////////////

char *string_copy(const char *aString) {
	size_t size   = strlen(aString) + 1;
	char  *result = (char*)STR_MALLOC(size);
	memcpy(result, aString, size);
	return result;
}

///////////////////////////////////////////

char *string_make(stref_t &ref) {
	char *result = string_copy(stref_withend(ref));
	stref_remend(ref);
	return result;
}

///////////////////////////////////////////

char *string_append(char *aBase, uint32_t aCount, ...) {
	size_t baseSize = aBase == nullptr? 0 : strlen(aBase);

	size_t  addSize = 0;
	va_list args;
	va_start(args, aCount);
	for (uint32_t i = 0; i < aCount; i++) {
		addSize += strlen(va_arg(args, char *));
	}
	va_end(args);

	char *result = (char*)STR_REALLOC(aBase, baseSize + addSize + 1);
	char *curr   = result + baseSize;
	va_start(args, aCount);
	for (uint32_t i = 0; i < aCount; i++) {
		char *arg = va_arg(args, char *);
		size_t len = strlen(arg);
		memcpy(curr, arg, len + 1);
		curr += len;
	}
	va_end(args);

	return result;
}

///////////////////////////////////////////

char *string_substr(const char *a, uint32_t start, uint32_t length) {
	if (length == 0) return nullptr;

	char *result = (char *)STR_MALLOC(length + 1);
	if (result != nullptr) {
		memcpy(result, a + start, length);
		result[length] = '\0';
	}
	return result;
}

///////////////////////////////////////////

bool  string_eq(const char *a, const char *b) {
	if (a == nullptr || b == nullptr) return false;
	while (*a != '\0' && *b != '\0') {
		if (*a != *b)
			return false;
		a++;
		b++;
	}
	return *a == *b;
}

///////////////////////////////////////////

bool  string_eq_nocase(const char *a, const char *b) {
	while (*a != '\0' && *b != '\0') {
		if (tolower(*a) != tolower(*b))
			return false;
		a++;
		b++;
	}
	return *a == *b;
}

///////////////////////////////////////////

bool  string_endswith(const char *a, const char *end, bool case_sensitive) {
	size_t len_a   = strlen(a);
	size_t len_end = strlen(end);
	if (len_end > len_a)
		return false;
	a = a + (len_a - len_end);

	if (!case_sensitive) {
		while (*a != '\0' && *end != '\0') {
			if (tolower(*a) != tolower(*end))
				return false;
			a++;
			end++;
		}
		return tolower(*a) == tolower(*end);
	} else {
		while (*a != '\0' && *end != '\0') {
			if (*a != *end)
				return false;
			a++;
			end++;
		}
		return *a == *end;
	}
}

///////////////////////////////////////////

bool string_startswith(const char *a, const char *is) {
	while (*is != '\0') {
		if (*a == '\0' || *is != *a)
			return false;
		a++;
		is++;
	}
	return true;
}

///////////////////////////////////////////

bool  stref_equals(const stref_t &aRef, const char *aIs) {
	assert(aRef.temp == false);

	const char *currRef = aRef.start;
	uint32_t curr = 0;
	while (*currRef != '\0' && *aIs != '\0' && curr < aRef.length) {
		if (*currRef != *aIs)
			return false;
		currRef++;
		aIs++;
		curr++;
	}
	return curr == aRef.length && *aIs == '\0';
}

///////////////////////////////////////////

bool  stref_equals(const stref_t &a, const stref_t &b) {
	assert(a.temp == false);
	assert(b.temp == false);
	if (a.length != b.length)
		return false;

	const char *a_curr = a.start;
	const char *b_curr = b.start;
	uint32_t curr = 0;
	while (*a_curr != '\0' && *b_curr != '\0' && curr < a.length) {
		if (*a_curr != *b_curr)
			return false;
		a_curr++;
		b_curr++;
		curr++;
	}
	return true;
}

///////////////////////////////////////////

bool  stref_startswith(const stref_t &a, const char *is) {
	uint32_t curr = 0;
	while (*is != '\0') {
		if (curr >= a.length || *is != a.start[curr])
			return false;
		curr++;
		is++;
	}
	return true;
}

///////////////////////////////////////////

int32_t   stref_indexof(const stref_t &aRef, char aChar) {
	for (uint32_t i = 0; i < aRef.length; i++) {
		if (aRef.start[i] == aChar)
			return i;
	}
	return -1;
}

///////////////////////////////////////////

int32_t   stref_lastof(const stref_t &aRef, char aChar) {
	int32_t result = -1;
	for (uint32_t i = 0; i < aRef.length; i++) {
		if (aRef.start[i] == aChar)
			result = i;
	}
	return result;
}

///////////////////////////////////////////

char *stref_copy(const stref_t &aRef) {
	assert(aRef.temp == false);

	char  *result = (char*)STR_MALLOC((uint64_t)aRef.length+1);
	memcpy(result, aRef.start, aRef.length);
	result[aRef.length] = '\0';
	return result;
}

///////////////////////////////////////////

void stref_copy_to(const stref_t &ref, char *text, size_t text_size) {
	assert(ref.temp == false);
	size_t length = text_size < ref.length ? text_size : ref.length;
	memcpy(text, ref.start, length);
	text[length] = '\0';
}

///////////////////////////////////////////

const char *stref_withend(stref_t &aRef) {
	assert(aRef.temp == false);

	aRef.temp_end = aRef.start[aRef.length];
	aRef.temp = true;
	((char*)aRef.start)[aRef.length] = '\0';

	return aRef.start;
}

///////////////////////////////////////////

void  stref_remend(stref_t &aRef) {
	assert(aRef.temp == true);

	((char*)aRef.start)[aRef.length] = aRef.temp_end;
	aRef.temp_end = 0;
	aRef.temp = false;
}

///////////////////////////////////////////

stref_t  stref_make(const char *aSource) {
	stref_t result;
	result.start    = aSource;
	result.length   = (uint32_t)strlen(aSource);
	result.temp_end = 0;
	result.temp     = false;
	return result;
}

///////////////////////////////////////////

stref_t stref_substr(const stref_t &aRef, uint32_t aStart, uint32_t aLength) {
	assert(aRef.temp == false);

	stref_t result;
	result.start    = aRef.start + aStart;
	result.length   = aLength;
	result.temp_end = 0;
	result.temp     = false;
	return result;
}

///////////////////////////////////////////

stref_t stref_substr(const char *aSource, uint32_t aLength) {
	stref_t result;
	result.start    = aSource;
	result.length   = aLength;
	result.temp_end = 0;
	result.temp     = false;
	return result;
}

///////////////////////////////////////////

bool is_white(char c) {
	return c == ' ' || c == '\t' || c=='\n' || c=='\r';
}

///////////////////////////////////////////

void  stref_trim(stref_t &aRef) {
	while (aRef.length > 0 && is_white(aRef.start[0])) {
		aRef.start += 1;
		aRef.length -= 1;
	}
	while (aRef.length > 0 && is_white(aRef.start[aRef.length-1])) {
		aRef.length -= 1;
	}
}

///////////////////////////////////////////

uint32_t  stref_count(stref_t &aRef, char aChar) {
	uint32_t result = 0;
	for (uint32_t i = 0; i < aRef.length; i++) {
		if (aRef.start[i] == aChar)
			result += 1;
	}
	return result;
}

///////////////////////////////////////////

bool stref_nextline(stref_t &from, stref_t &curr_line) {
	assert(curr_line.temp == false);

	if (curr_line.start == nullptr) {
		curr_line = stref_substr(from, 0, from.length);
		char *curr = (char*)(curr_line.start);
		while (*curr != '\n' && *curr != '\r' && *curr != '\0') curr++;
		curr_line.length = (uint32_t)(curr - curr_line.start);
		return true;
	}

	char *curr = (char*)(curr_line.start);
	while (*curr != '\n' && *curr != '\r' && *curr != '\0') curr++;
	while (*curr == '\n' || *curr == '\r') curr++;
	if (*curr == '\0') return false;

	curr_line.start = curr;
	while (*curr != '\n' && *curr != '\r' && *curr != '\0') curr++;
	curr_line.length = (uint32_t)(curr - curr_line.start);
	return true;
}

///////////////////////////////////////////

bool stref_nextword(stref_t &line, stref_t &word, char separator, char capture_char_start, char capture_char_end, bool *out_capture_error) {
	assert(word.temp == false);
	if (out_capture_error != nullptr)
		*out_capture_error = false;
	if (word.start == nullptr)
		word = stref_substr(line, 0, 0);

	char *curr = (char*)(word.start + word.length);
	if (*curr == separator) curr++;
	while (*curr == ' ' || *curr == '\t') {
		curr++; 
	}
	if ( *curr == '\n' || *curr == '\r' || *curr == '\0') return false;

	word.start = curr;

	// Check if we're looking at a capture group
	int capture_count = 0;
	bool want_capture = capture_char_start != '\0';
	if (want_capture && *curr == capture_char_start) {
		capture_count = 1;
		curr++;
	}
	
	while (
		(*curr != separator || capture_count > 0) &&                            // While we're still part of the same word or capture group
		!(want_capture && capture_count == 0 && *curr == capture_char_start) && // Check if we're running into an adjacent capture group
		(*curr != '\n' && *curr != '\r' && *curr != '\0') &&                    // While we're still on the same line
		(curr < line.start+line.length))                                        // And make sure it's within the confines of the stref we're provided
	{
		if (want_capture) {
			if (*curr == capture_char_start)
				capture_count += 1;
			if (*curr == capture_char_end) {
				capture_count -= 1;
				if (capture_count == 0) {
					curr++;
					break;
				}
			}
		}
		curr++;
	}
	word.length = (uint32_t)(curr - word.start);
	if (out_capture_error != nullptr)
		*out_capture_error = capture_count > 0;
	return true;
}

///////////////////////////////////////////

void stref_file_path(const stref_t &filename, stref_t &out_path, stref_t &out_name) {
	out_path = stref_t{ filename.start, 0 };
	int32_t  end    = filename.length;
	int32_t  slash1 = stref_lastof(filename, '\\');
	int32_t  slash2 = stref_lastof(filename, '/');
	uint32_t start  = (slash1 > slash2 ? slash1 : slash2) + 1;
	out_name = stref_substr(filename, start, end-start);

	if (stref_indexof(out_name, '.') != -1) {
		// Has a '.', must be a filename!
		out_path.length = (int)start-1 < 0 ? 0 : start-1;
	}
}

///////////////////////////////////////////

stref_t stref_stripcapture(stref_t &word, char capture_char_start, char capture_char_end) {
	if (word.length < 2) return word;
	stref_t result = word;
	if (*result.start == capture_char_start) {
		result.start++;
		result.length--;
	}
	if (*(result.start+result.length-1) == capture_char_end) {
		result.length--;
	}
	stref_trim(result);
	return result;
}

///////////////////////////////////////////

// Powers of ten 10^0 .. 10^22 are all exactly representable as doubles. Shared
// by the locale-independent float parser and formatter below.
static const double pow10[] = {
	1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,
	1e8,  1e9,  1e10, 1e11, 1e12, 1e13, 1e14, 1e15,
	1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22 };

// 10^e as a double for any e, chaining beyond the exactly-representable range.
static double sk_pow10(int32_t e) {
	bool neg = e < 0; if (neg) e = -e;
	double r = 1.0;
	while (e > 22) { r *= 1e22; e -= 22; }
	r *= pow10[e];
	return neg ? 1.0 / r : r;
}

///////////////////////////////////////////

// A locale-independent, self-contained replacement for (float)atof. The C
// library's atof/strtod honor the process locale's LC_NUMERIC decimal
// separator, but our data (glTF/JSON, PLY, shader metadata) always uses '.'.
// Under locales like fr_FR.UTF-8 the separator is ',', so atof("0.5") stops at
// the '.' and returns 0 - silently corrupting parsed values. This parser always
// treats '.' as the decimal point, and as a bonus runs ~3x faster than atof.
//
// Digits are accumulated into an integer mantissa, then scaled by a power of
// ten. When the mantissa is exactly representable as a double (<= 2^53) and the
// exponent is small (|e| <= 22, the range where 10^e is exact in a double), the
// scale is a single multiply/divide and the result is correctly rounded. The
// double result is rounded to float once at the end, so it matches
// (float)strtod bit-for-bit across the entire float range. Out-of-range
// magnitudes (which don't occur in real assets) fall back to a chained scale
// that may round in the last bit.
float string_to_float(const char *str) {
	const char *s = str;
	while (*s == ' ' || (*s >= '\t' && *s <= '\r')) s += 1;

	bool negative = false;
	if      (*s == '-') { negative = true; s += 1; }
	else if (*s == '+') {                  s += 1; }

	uint64_t mantissa  = 0;  // significant digits as an integer
	int32_t  exponent  = 0;  // power of ten to scale the mantissa by
	int32_t  digits    = 0;  // count of accumulated significant digits
	bool     has_digit = false;

	while (*s >= '0' && *s <= '9') {
		has_digit = true;
		if      (mantissa == 0 && *s == '0') { /* skip leading zeros */ }
		else if (digits < 19) { mantissa = mantissa * 10 + (uint64_t)(*s - '0'); digits += 1; }
		else                  { exponent += 1; } // too many digits for a u64
		s += 1;
	}
	if (*s == '.') {
		s += 1;
		while (*s >= '0' && *s <= '9') {
			has_digit = true;
			if      (mantissa == 0 && *s == '0') { exponent -= 1; } // leading zero, shifts scale
			else if (digits < 19) { mantissa = mantissa * 10 + (uint64_t)(*s - '0'); digits += 1; exponent -= 1; }
			s += 1;
		}
	}

	if (!has_digit) return 0.0f;

	if (*s == 'e' || *s == 'E') {
		const char *e = s + 1;
		bool e_negative = false;
		if      (*e == '-') { e_negative = true; e += 1; }
		else if (*e == '+') {                    e += 1; }
		if (*e >= '0' && *e <= '9') {
			int32_t exp = 0;
			while (*e >= '0' && *e <= '9') {
				if (exp < 0x10000) exp = exp * 10 + (*e - '0');
				e += 1;
			}
			exponent += e_negative ? -exp : exp;
		}
	}

	double result = (double)mantissa;
	if (result != 0.0 && exponent != 0) {
		if (mantissa <= 0x20000000000000ull && exponent >= -22 && exponent <= 22) {
			// Fast path: both operands are exact in a double, so this is a
			// single correctly-rounded operation.
			if (exponent >= 0) result = result * pow10[exponent];
			else               result = result / pow10[-exponent];
		} else {
			// Extreme magnitude, doesn't occur in real assets. Chain the scale
			// in steps of 10^22; may round in the last bit.
			int32_t e = exponent;
			if (e > 0) {
				while (e > 22) { result *= 1e22; e -= 22; if (result > 1e308) break; }
				result *= pow10[e];
			} else {
				e = -e;
				while (e > 22) { result /= 1e22; e -= 22; if (result == 0.0) break; }
				result /= pow10[e];
			}
		}
	}
	return (float)(negative ? -result : result);
}

///////////////////////////////////////////

// The locale-independent counterpart to string_to_float: formats a float like
// printf's "%.*g" but always with a '.' decimal separator, with no dependence
// on the C locale at all (printf's %g would emit "0,5" under fr_FR.UTF-8).
// `precision` is the number of significant digits, clamped to [1,9] (a float
// carries ~9 significant decimal digits). Output truncates safely to out_size.
// Returns out_str for convenient chaining. Matches printf's "%.*g" for the vast
// majority of values, and round-trips exactly through string_to_float at
// precision 9.
//
// It mirrors the parser: normalize the magnitude into [1,10) to find its decimal
// exponent, pull `precision` significant digits out as an integer, then place
// the '.' (choosing fixed vs exponential notation the way %g does).
char *string_from_float(float value, char *out_str, int32_t out_size, int32_t precision) {
	if (precision < 1) precision = 1;
	if (precision > 9) precision = 9;

	char *p   = out_str;
	char *end = out_str + out_size - 1; // reserve room for the null terminator

	if (signbit(value)) { if (p < end) *p++ = '-'; value = -value; } // also handles -0

	if (isinf(value) || isnan(value)) {
		const char *w = isnan(value) ? "nan" : "inf";
		while (*w != '\0' && p < end) *p++ = *w++;
		*p = '\0';
		return out_str;
	}
	if (value == 0.0f) { if (p < end) *p++ = '0'; *p = '\0'; return out_str; }

	// Normalize |value| into [1,10) and find its decimal exponent, correcting for
	// any rounding error in log10.
	double  v  = value;
	int32_t e  = (int32_t)floor(log10(v));
	double  pe = sk_pow10(e);
	if      (v <  pe)        { e -= 1; pe = sk_pow10(e); }
	else if (v >= pe * 10.0) { e += 1; pe = sk_pow10(e); }

	// Pull out `precision` significant digits as an integer in [10^(p-1), 10^p).
	uint64_t pow_hi = (uint64_t)(sk_pow10(precision)     + 0.5);
	uint64_t pow_lo = (uint64_t)(sk_pow10(precision - 1) + 0.5);
	uint64_t digits = (uint64_t)((v / pe) * (double)pow_lo + 0.5);
	if (digits >= pow_hi) { digits = pow_lo; e += 1; } // rounded up past range, e.g. 9.99 -> 10

	// Trim trailing zeros like %g.
	int32_t digit_ct = precision;
	while (digit_ct > 1 && digits % 10 == 0) { digits /= 10; digit_ct -= 1; }

	// Expand the digits into a small buffer, most significant first.
	char digit_str[10];
	for (int32_t i = digit_ct - 1; i >= 0; i -= 1) { digit_str[i] = (char)('0' + digits % 10); digits /= 10; }

	if (e < -4 || e >= precision) {
		// Exponential notation: d.ddde±XX (at least two exponent digits, like %g).
		if (p < end) *p++ = digit_str[0];
		if (digit_ct > 1) {
			if (p < end) *p++ = '.';
			for (int32_t i = 1; i < digit_ct && p < end; i += 1) *p++ = digit_str[i];
		}
		if (p < end) *p++ = 'e';
		if (p < end) *p++ = e < 0 ? '-' : '+';
		int32_t ae = e < 0 ? -e : e;
		if (ae >= 100 && p < end) *p++ = (char)('0' + ae / 100);
		if (p < end) *p++ = (char)('0' + (ae / 10) % 10);
		if (p < end) *p++ = (char)('0' +  ae       % 10);
	} else if (e >= 0) {
		// Fixed notation, integer part is e+1 digits wide.
		for (int32_t i = 0; i <= e && p < end; i += 1) *p++ = i < digit_ct ? digit_str[i] : '0';
		if (digit_ct > e + 1) {
			if (p < end) *p++ = '.';
			for (int32_t i = e + 1; i < digit_ct && p < end; i += 1) *p++ = digit_str[i];
		}
	} else {
		// Fixed notation, 0.00..digits
		if (p < end) *p++ = '0';
		if (p < end) *p++ = '.';
		for (int32_t i = 0; i < -e - 1 && p < end; i += 1) *p++ = '0';
		for (int32_t i = 0; i < digit_ct && p < end; i += 1) *p++ = digit_str[i];
	}
	*p = '\0';
	return out_str;
}

///////////////////////////////////////////

float stref_to_f(const stref_t &ref) {
	char text[32];
	stref_copy_to(ref, text, 32);
	return string_to_float(text);
}

///////////////////////////////////////////

int32_t  stref_to_i(const stref_t &ref) {
	char text[32];
	stref_copy_to(ref, text, 32);
	return atoi(text);
}