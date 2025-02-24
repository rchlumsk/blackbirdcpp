#include <time.h>
#include "BlackbirdInclude.h"

//////////////////////////////////////////////////////////////////
/// \brief Converts any lowercase characters in a string to uppercase, returning the converted string
/// \param &s [in] String to be converted to uppercase
/// \return &s converted to uppercase
//
std::string StringToUppercase(const std::string& s)
{
  std::string ret(s.size(), char());
  for (int i = 0; i < (int)(s.size()); ++i)
  {
    if ((s[i] <= 'z' && s[i] >= 'a')) { ret[i] = s[i] - ('a' - 'A'); }
    else { ret[i] = s[i]; }
  }
  return ret;
}

//////////////////////////////////////////////////////////////////
/// \brief Simple and fast atof (ascii to float) function.
/// \notes Executes about 5x faster than standard MSCRT library atof().
///
/// \notes ported 09-May-2009 from Tom Van Baak (tvb) www.LeapSecond.com
//

#define white_space(c) ((c) == ' ' || (c) == '\t')
#define valid_digit(c) ((c) >= '0' && (c) <= '9')

double fast_s_to_d(const char* p)
{
  int frac;
  double sign, value, scale;

  // Skip leading white space, if any.

  while (white_space(*p)) {
    p += 1;
  }

  // Get sign, if any.

  sign = 1.0;
  if (*p == '-') {
    sign = -1.0;
    p += 1;

  }
  else if (*p == '+') {
    p += 1;
  }

  // Get digits before decimal point or exponent, if any.

  for (value = 0.0; valid_digit(*p); p += 1) {
    value = value * 10.0 + (*p - '0');
  }

  // Get digits after decimal point, if any.

  if (*p == '.') {
    double pow10 = 10.0;
    p += 1;
    while (valid_digit(*p)) {
      value += (*p - '0') / pow10;
      pow10 *= 10.0;
      p += 1;
    }
  }

  // Handle exponent, if any.

  frac = 0;
  scale = 1.0;
  if ((*p == 'e') || (*p == 'E')) {
    unsigned int expon;

    // Get sign of exponent, if any.

    p += 1;
    if (*p == '-') {
      frac = 1;
      p += 1;

    }
    else if (*p == '+') {
      p += 1;
    }

    // Get digits of exponent, if any.

    for (expon = 0; valid_digit(*p); p += 1) {
      expon = expon * 10 + (*p - '0');
    }
    if (expon > 308) expon = 308;

    // Calculate scaling factor.

    while (expon >= 50) { scale *= 1E50; expon -= 50; }
    while (expon >= 8) { scale *= 1E8;  expon -= 8; }
    while (expon > 0) { scale *= 10.0; expon -= 1; }
  }

  // Return signed and scaled floating point result.

  return sign * (frac ? (value / scale) : (value * scale));
}

//////////////////////////////////////////////////////////////////
/// \brief returns true if line is empty, begins with '#' or '*'
/// \param &s [in] first string token in file line
/// \param Len length of line
/// \return true if line is empty or a comment
//
bool IsComment(const char* s, const int Len)
{
  if ((Len == 0) || (s[0] == '#') || (s[0] == '*')) { return true; }
  return false;
}

/////////////////////////////////////////////////////////////////
/// \brief writes warning to screen and to Blackbird_errors.txt file
/// \param warn [in] warning message printed
//
void WriteWarning(const std::string warn, bool noisy)
{
  if (!g_suppress_warnings) {
    std::ofstream WARNINGS;
    WARNINGS.open((g_output_directory + "Blackbird_errors.txt").c_str(), std::ios::app);
    if (noisy) { std::cout << "WARNING!: " << warn << std::endl; }
    WARNINGS << "WARNING : " << warn << std::endl;
    WARNINGS.close();
  }
}

/////////////////////////////////////////////////////////////////
/// \brief writes advisory to screen and to Blackbird_errors.txt file
/// \param warn [in] warning message printed
//
void WriteAdvisory(const std::string warn, bool noisy)
{
  if (!g_suppress_warnings) {
    std::ofstream WARNINGS;
    WARNINGS.open((g_output_directory + "Blackbird_errors.txt").c_str(), std::ios::app);
    if (noisy) { std::cout << "ADVISORY: " << warn << std::endl; }
    WARNINGS << "ADVISORY : " << warn << std::endl;
    WARNINGS.close();
  }
}

/////////////////////////////////////////////////////////////////
/// \brief custom cpl error handler that does nothing
//
void SilentErrorHandler(CPLErr eErrClass, int err_no, const char *msg) {}