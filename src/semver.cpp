#include "semver.hpp"
#include <absl/strings/str_join.h>
#include <absl/strings/str_split.h>
#include <fmt/core.h>
#include <re2/re2.h>
#include <shared_mutex>
#include <unordered_set>

/* # Define */

#define SEMVER_SPEC_VERSION "2.0.0"
#define MAX_LENGTH 256
#define MAX_SAFE_INTEGER                                                       \
  9007199254740991 // much less than the max value of long long
#define MAX_SAFE_COMPONENT_LENGTH 16

/* # Regex */

enum class regexIndex : std::size_t {
  NUMERICIDENTIFIER,
  NUMERICIDENTIFIERLOOSE,
  NONNUMERICIDENTIFIER,
  MAINVERSION,
  MAINVERSIONLOOSE,
  PRERELEASEIDENTIFIER,
  PRERELEASEIDENTIFIERLOOSE,
  PRERELEASE,
  PRERELEASELOOSE,
  BUILDIDENTIFIER,
  BUILD,
  FULLPLAIN,
  FULL,
  LOOSEPLAIN,
  LOOSE,
  GTLT,
  XRANGEIDENTIFIERLOOSE,
  XRANGEIDENTIFIER,
  XRANGEPLAIN,
  XRANGEPLAINLOOSE,
  XRANGE,
  XRANGELOOSE,
  COERCE,
  COERCERTL,
  LONETILDE,
  TILDETRIM,
  TILDE,
  TILDELOOSE,
  LONECARET,
  CARETTRIM,
  CARET,
  CARETLOOSE,
  COMPARATORLOOSE,
  COMPARATOR,
  COMPARATORTRIM,
  HYPHENRANGE,
  HYPHENRANGELOOSE,
  STAR,
  GTE0,
  GTE0PRE,
  WHITESPACENORMALIZE,
  ENDOFENUM
};

class re {
private:
  static const std::string NUMERICIDENTIFIER;
  static const std::string NUMERICIDENTIFIERLOOSE;
  static const std::string NONNUMERICIDENTIFIER;
  static const std::string MAINVERSION;
  static const std::string MAINVERSIONLOOSE;
  static const std::string PRERELEASEIDENTIFIER;
  static const std::string PRERELEASEIDENTIFIERLOOSE;
  static const std::string PRERELEASE;
  static const std::string PRERELEASELOOSE;
  static const std::string BUILDIDENTIFIER;
  static const std::string BUILD;
  static const std::string FULLPLAIN;
  static const std::string FULL;
  static const std::string LOOSEPLAIN;
  static const std::string LOOSE;
  static const std::string GTLT;
  static const std::string XRANGEIDENTIFIERLOOSE;
  static const std::string XRANGEIDENTIFIER;
  static const std::string XRANGEPLAIN;
  static const std::string XRANGEPLAINLOOSE;
  static const std::string XRANGE;
  static const std::string XRANGELOOSE;
  static const std::string COERCE;
  static const std::string COERCERTL;
  static const std::string LONETILDE;
  static const std::string TILDETRIM;
  static const std::string TILDE;
  static const std::string TILDELOOSE;
  static const std::string LONECARET;
  static const std::string CARETTRIM;
  static const std::string CARET;
  static const std::string CARETLOOSE;
  static const std::string COMPARATORLOOSE;
  static const std::string COMPARATOR;
  static const std::string COMPARATORTRIM;
  static const std::string HYPHENRANGE;
  static const std::string HYPHENRANGELOOSE;
  static const std::string STAR;
  static const std::string GTE0;
  static const std::string GTE0PRE;
  static const std::string WHITESPACENORMALIZE;
  static const std::string *regexString[];

  std::vector<std::unique_ptr<RE2>> regexDense;
  std::vector<int> regexSparse;

  re() {
    regexSparse.resize(static_cast<std::size_t>(regexIndex::ENDOFENUM));
    std::fill(regexSparse.begin(), regexSparse.end(), -1);
  }

  std::shared_mutex mu;

public:
  static const std::string tildeTrimReplace;
  static const std::string caretTrimReplace;
  static const std::string comparatorTrimReplace;
  static const std::string whitespaceNormalizeReplace;
  static const std::string starReplace;
  static const std::string gte0Replace;

  static re &getInstance() {
    static re instance;
    return instance;
  }

  re(re const &) = delete;
  void operator=(re const &) = delete;

  const RE2 *getRegex(regexIndex regex) {
    int i = regexSparse[static_cast<std::size_t>(regex)];
    if (i == -1) {
      std::unique_lock lock(mu);
      i = regexSparse[static_cast<std::size_t>(regex)];
      if (i == -1) {
        i = regexDense.size();
        regexSparse[static_cast<std::size_t>(regex)] = i;
        regexDense.emplace_back(std::make_unique<RE2>(
            *(regexString[static_cast<std::size_t>(regex)])));
      }
      return regexDense[i].get();
    } else {
      std::shared_lock lock(mu);
      return regexDense[i].get();
    }
  }
};

// ## Numeric Identifier
// A single `0`, or a non-zero digit followed by zero or more digits.
const std::string re::NUMERICIDENTIFIER = "0|[1-9]\\d*";
const std::string re::NUMERICIDENTIFIERLOOSE = "[0-9]+";

// ## Non-numeric Identifier
// Zero or more digits, followed by a letter or hyphen, and then zero or
// more letters, digits, or hyphens.
const std::string re::NONNUMERICIDENTIFIER = "\\d*[a-zA-Z-][a-zA-Z0-9-]*";

// ## Main Version
// Three dot-separated numeric identifiers.
const std::string re::MAINVERSION =
    fmt::format("({0})\\.({0})\\.({0})", NUMERICIDENTIFIER);
const std::string re::MAINVERSIONLOOSE =
    fmt::format("({0})\\.({0})\\.({0})", NUMERICIDENTIFIERLOOSE);

// ## Pre-release Version Identifier
// A numeric identifier, or a non-numeric identifier.
const std::string re::PRERELEASEIDENTIFIER =
    fmt::format("(?:{}|{})", NUMERICIDENTIFIER, NONNUMERICIDENTIFIER);
const std::string re::PRERELEASEIDENTIFIERLOOSE =
    fmt::format("(?:{}|{})", NUMERICIDENTIFIERLOOSE, NONNUMERICIDENTIFIER);

// ## Pre-release Version
// Hyphen, followed by one or more dot-separated pre-release version
// identifiers.
const std::string re::PRERELEASE =
    fmt::format("(?:-({0}(?:\\.{0})*))", PRERELEASEIDENTIFIER);
const std::string re::PRERELEASELOOSE =
    fmt::format("(?:-?({0}(?:\\.{0})*))", PRERELEASEIDENTIFIERLOOSE);

// ## Build Metadata Identifier
// Any combination of digits, letters, or hyphens.
const std::string re::BUILDIDENTIFIER = "[0-9A-Za-z-]+";

// ## Build Metadata
// Plus sign, followed by one or more period-separated build metadata
// identifiers.
const std::string re::BUILD =
    fmt::format("(?:\\+({0}(?:\\.{0})*))", BUILDIDENTIFIER);

// ## Full Version String
// A main version, followed optionally by a pre-release version and
// build metadata.

// Note that the only major, minor, patch, and pre-release sections of
// the version string are capturing groups.  The build metadata is not a
// capturing group, because it should not ever be used in version
// comparison.
const std::string re::FULLPLAIN =
    fmt::format("v?{}{}?{}?", MAINVERSION, PRERELEASE, BUILD);
const std::string re::FULL = fmt::format("^{}$", FULLPLAIN);

// like full, but allows v1.2.3 and =1.2.3, which people do sometimes.
// also, 1.0.0alpha1 (prerelease without the hyphen) which is pretty
// common in the npm registry.
const std::string re::LOOSEPLAIN =
    fmt::format("[v=\\s]*{}{}?{}?", MAINVERSIONLOOSE, PRERELEASELOOSE, BUILD);
const std::string re::LOOSE = fmt::format("^{}$", LOOSEPLAIN);

const std::string re::GTLT = "((?:<|>)?=?)";

// Something like "2.*" or "1.2.x".
// Note that "x.x" is a valid xRange identifer, meaning "any version"
// Only the first item is strictly required.
const std::string re::XRANGEIDENTIFIERLOOSE =
    fmt::format("{}|x|X|\\*", NUMERICIDENTIFIERLOOSE);
const std::string re::XRANGEIDENTIFIER =
    fmt::format("{}|x|X|\\*", NUMERICIDENTIFIER);
const std::string re::XRANGEPLAIN =
    fmt::format("[v=\\s]*({0})(?:\\.({0})(?:\\.({0})(?:{1})?{2}?)?)?",
                XRANGEIDENTIFIER, PRERELEASE, BUILD);
const std::string re::XRANGEPLAINLOOSE =
    fmt::format("[v=\\s]*({0})(?:\\.({0})(?:\\.({0})(?:{1})?{2}?)?)?",
                XRANGEIDENTIFIERLOOSE, PRERELEASELOOSE, BUILD);
const std::string re::XRANGE = fmt::format("^{}\\s*{}$", GTLT, XRANGEPLAIN);
const std::string re::XRANGELOOSE =
    fmt::format("^{}\\s*{}$", GTLT, XRANGEPLAINLOOSE);

// Coercion.
// Extract anything that could conceivably be a part of a valid semver
const std::string re::COERCE =
    fmt::format("(^|[^\\d])(\\d{{1,{0}}})(?:\\.(\\d{{1,{0}}}))?(?:\\.(\\d{{1,{"
                "0}}}))?(?:$|[^\\d])",
                MAX_SAFE_COMPONENT_LENGTH);
const std::string re::COERCERTL = COERCE;

// Tilde ranges.
// Meaning is "reasonably at or greater than"
const std::string re::LONETILDE = "(?:~>?)";
const std::string re::TILDETRIM = fmt::format("(\\s*){}\\s+", LONETILDE);
const std::string re::tildeTrimReplace = "\\1~";
const std::string re::TILDE = fmt::format("^{}{}$", LONETILDE, XRANGEPLAIN);
const std::string re::TILDELOOSE =
    fmt::format("^{}{}$", LONETILDE, XRANGEPLAINLOOSE);

// Caret ranges.
// Meaning is "at least and backwards compatible with"
const std::string re::LONECARET = "(?:\\^)";
const std::string re::CARETTRIM = fmt::format("(\\s*){}\\s+", LONECARET);
const std::string re::caretTrimReplace = "\\1^";
const std::string re::CARET = fmt::format("^{}{}$", LONECARET, XRANGEPLAIN);
const std::string re::CARETLOOSE =
    fmt::format("^{}{}$", LONECARET, XRANGEPLAINLOOSE);

// A simple gt/lt/eq thing, or just "" to indicate "any version"
const std::string re::COMPARATORLOOSE =
    fmt::format("^{}\\s*({})$|^$", GTLT, LOOSEPLAIN);
const std::string re::COMPARATOR =
    fmt::format("^{}\\s*({})$|^$", GTLT, FULLPLAIN);
// An expression to strip any whitespace between the gtlt and the thing
// it modifies, so that `> 1.2.3` ==> `>1.2.3`
const std::string re::COMPARATORTRIM =
    fmt::format("(\\s*){}\\s*({}|{})", GTLT, LOOSEPLAIN, XRANGEPLAIN);
const std::string re::comparatorTrimReplace = "\\1\\2\\3";

// Something like `1.2.3 - 1.2.4`
// Note that these all use the loose form, because they'll be
// checked against either the strict or loose comparator form
// later.
const std::string re::HYPHENRANGE =
    fmt::format("^\\s*({0})\\s+-\\s+({0})\\s*$", XRANGEPLAIN);
const std::string re::HYPHENRANGELOOSE =
    fmt::format("^\\s*({0})\\s+-\\s+({0})\\s*$", XRANGEPLAINLOOSE);

// Star ranges basically just allow anything at all.
const std::string re::STAR = "(<|>)?=?\\s*\\*";
// >=0.0.0 is like a star
const std::string re::GTE0 = "^\\s*>=\\s*0\\.0\\.0\\s*$";
const std::string re::GTE0PRE = "^\\s*>=\\s*0\\.0\\.0-0\\s*$";

// Whitespace
const std::string re::WHITESPACENORMALIZE = "\\s+";
const std::string re::whitespaceNormalizeReplace = " ";

const std::string re::starReplace = "";
const std::string re::gte0Replace = "";

const std::string *re::regexString[] = {&NUMERICIDENTIFIER,
                                        &NUMERICIDENTIFIERLOOSE,
                                        &NONNUMERICIDENTIFIER,
                                        &MAINVERSION,
                                        &MAINVERSIONLOOSE,
                                        &PRERELEASEIDENTIFIER,
                                        &PRERELEASEIDENTIFIERLOOSE,
                                        &PRERELEASE,
                                        &PRERELEASELOOSE,
                                        &BUILDIDENTIFIER,
                                        &BUILD,
                                        &FULLPLAIN,
                                        &FULL,
                                        &LOOSEPLAIN,
                                        &LOOSE,
                                        &GTLT,
                                        &XRANGEIDENTIFIERLOOSE,
                                        &XRANGEIDENTIFIER,
                                        &XRANGEPLAIN,
                                        &XRANGEPLAINLOOSE,
                                        &XRANGE,
                                        &XRANGELOOSE,
                                        &COERCE,
                                        &COERCERTL,
                                        &LONETILDE,
                                        &TILDETRIM,
                                        &TILDE,
                                        &TILDELOOSE,
                                        &LONECARET,
                                        &CARETTRIM,
                                        &CARET,
                                        &CARETLOOSE,
                                        &COMPARATORLOOSE,
                                        &COMPARATOR,
                                        &COMPARATORTRIM,
                                        &HYPHENRANGE,
                                        &HYPHENRANGELOOSE,
                                        &STAR,
                                        &GTE0,
                                        &GTE0PRE,
                                        &WHITESPACENORMALIZE};

/* # Compare Helper */

inline int compareIdentifiers(long long a, long long b) {
  return a == b ? 0 : a < b ? -1 : 1;
}

// a.size() and b.size() must be less than MAX_SAFE_COMPONENT_LENGTH
inline int compareIdentifiers(const std::string &a, const std::string &b) {
  bool anum = a.size() && a.size() < MAX_SAFE_COMPONENT_LENGTH &&
              std::all_of(a.begin(), a.end(), ::isdigit);
  bool bnum = b.size() && b.size() < MAX_SAFE_COMPONENT_LENGTH &&
              std::all_of(b.begin(), b.end(), ::isdigit);
  if (anum && bnum) {
    return compareIdentifiers(std::stoll(a), std::stoll(b));
  } else if (anum) {
    return -1;
  } else if (bnum) {
    return 1;
  } else {
    return a.compare(b);
  }
}

bool eq(const semver &a, const semver &b) { return a.compare(b) == 0; }

bool gt(const semver &a, const semver &b) { return a.compare(b) > 0; }

bool gte(const semver &a, const semver &b) { return a.compare(b) >= 0; }

bool lt(const semver &a, const semver &b) { return a.compare(b) < 0; }

bool lte(const semver &a, const semver &b) { return a.compare(b) <= 0; }

// MUST in the order of enum class comparatorOperator
bool (*compareFunc[])(const semver &, const semver &) = {eq, gt, gte, lt, lte};

/* # Semver */

semver::semver(std::string_view version) {
  std::string versionTrim = std::string(absl::StripAsciiWhitespace(version));

  std::string major;
  std::string minor;
  std::string patch;
  std::string prerelease;
  std::string build;

  const auto looseRegex = re::getInstance().getRegex(regexIndex::LOOSE);
  valid = RE2::FullMatch(versionTrim, *looseRegex, &major, &minor, &patch,
                         &prerelease, &build);

  if (!valid) {
    return;
  }

  if (major.size() > MAX_SAFE_COMPONENT_LENGTH ||
      minor.size() > MAX_SAFE_COMPONENT_LENGTH ||
      patch.size() > MAX_SAFE_COMPONENT_LENGTH) {
    valid = false;
    return;
  }

  this->major = std::stoll(major);
  this->minor = std::stoll(minor);
  this->patch = std::stoll(patch);

  if (this->major > MAX_SAFE_INTEGER || this->minor > MAX_SAFE_INTEGER ||
      this->patch > MAX_SAFE_INTEGER) {
    valid = false;
    return;
  }

  if (prerelease.size()) {
    this->prerelease = absl::StrSplit(prerelease, '.');
  }
  if (build.size()) {
    this->build = absl::StrSplit(build, '.');
  }

  if (prerelease.size()) {
    this->value =
        fmt::format("{}.{}.{}-{}", this->major, this->minor, this->patch,
                    absl::StrJoin(this->prerelease, "."));
  } else {
    this->value =
        fmt::format("{}.{}.{}", this->major, this->minor, this->patch);
  }
}

std::string semver::format() const { return value; }

int semver::compare(const semver &other) const {
  if (value == other.value)
    return 0;
  int ret = compareMain(other);
  return ret ? ret : comparePre(other);
}

int semver::compareMain(const semver &other) const {
  if (int ret = compareIdentifiers(major, other.major))
    return ret;
  if (int ret = compareIdentifiers(minor, other.minor))
    return ret;
  return compareIdentifiers(patch, other.patch);
}

int semver::comparePre(const semver &other) const {
  std::size_t thisPreLength = prerelease.size();
  std::size_t otherPreLength = other.prerelease.size();

  if (!otherPreLength) {
    if (thisPreLength) {
      return -1;
    } else {
      return 0;
    }
  } else {
    if (!thisPreLength) {
      return 1;
    }
  }

  std::size_t i = 0;
  do {
    if (i >= thisPreLength) {
      if (i >= otherPreLength) {
        return 0;
      } else {
        return -1;
      }
    } else {
      if (i >= otherPreLength) {
        return 1;
      } else if (prerelease[i] == other.prerelease[i]) {
        continue;
      } else {
        break;
      }
    }
  } while (++i);

  return compareIdentifiers(prerelease[i], other.prerelease[i]);
}

/* # Comparator */

comparator::comparator(std::string_view comp) {
  std::string compTrim = std::string(absl::StripAsciiWhitespace(comp));

  std::string oper;
  std::string ver;

  const auto looseRegex =
      re::getInstance().getRegex(regexIndex::COMPARATORLOOSE);
  valid = RE2::FullMatch(compTrim, *looseRegex, &oper, &ver);

  if (!valid) {
    return;
  }

  if (oper.empty() || oper == "=") {
    op = comparatorOperator::EQ;
  } else if (oper == ">") {
    op = comparatorOperator::GT;
  } else if (oper == ">=") {
    op = comparatorOperator::GTE;
  } else if (oper == "<") {
    op = comparatorOperator::LT;
  } else if (oper == "<=") {
    op = comparatorOperator::LTE;
  }

  if (ver.empty()) {
    this->value = "";
  } else {
    this->ver = semver(ver);

    if (!this->ver.valid) {
      valid = false;
      return;
    }

    if (op == comparatorOperator::EQ) {
      value = this->ver.value;
    } else {
      value = oper + this->ver.value;
    }
  }
}

std::string comparator::format() const { return value; }

bool comparator::test(const semver &version) const {
  if (!version.valid) {
    return false;
  }

  if (isAny()) {
    return true;
  }

  return compareFunc[static_cast<std::size_t>(op)](version, ver);
}

/* # Range Parsing Helper */

inline bool isX(std::string_view id) {
  return id.empty() || id == "x" || id == "X" || id == "*";
}

// This function is passed to string.replace(re[t.HYPHENRANGE])
// M, m, patch, prerelease, build
// 1.2 - 3.4.5 => >=1.2.0 <=3.4.5
// 1.2.3 - 3.4 => >=1.2.0 <3.5.0-0 Any 3.4.x will do
// 1.2 - 3.4 => >=1.2.0 <3.5.0-0
bool hyphenReplace(std::string &range) {
  const auto looseRegex =
      re::getInstance().getRegex(regexIndex::HYPHENRANGELOOSE);

  std::string from;
  std::string fM;
  std::string fm;
  std::string fp;
  std::string fpr;

  std::string to;
  std::string tM;
  std::string tm;
  std::string tp;
  std::string tpr;

  if (RE2::FullMatch(range, *looseRegex, &from, &fM, &fm, &fp, &fpr, nullptr,
                     &to, &tM, &tm, &tp, &tpr)) {
    if (tM.size() > MAX_SAFE_COMPONENT_LENGTH ||
        tm.size() > MAX_SAFE_COMPONENT_LENGTH ||
        tp.size() > MAX_SAFE_COMPONENT_LENGTH) {
      return true;
    } else {
      // includePrerelease is OFF

      std::string newFrom, newTo;

      if (isX(fM)) {
        newFrom = "";
      } else if (isX(fm)) {
        newFrom = fmt::format(">={}.0.0", fM);
      } else if (isX(fp)) {
        newFrom = fmt::format(">={}.{}.0", fM, fm);
      } else {
        newFrom = fmt::format(">={}", from);
      }

      if (isX(tM)) {
        newTo = "";
      } else if (isX(tm)) {
        newTo = fmt::format("<{}.0.0-0", std::stoll(tM) + 1);
      } else if (isX(tp)) {
        newTo = fmt::format("<{}.{}.0-0", tM, std::stoll(tm) + 1);
      } else if (tpr.size()) {
        newTo = fmt::format("<={}.{}.{}-{}", tM, tm, tp, tpr);
      } else {
        newTo = fmt::format("<={}", to);
      }

      range = fmt::format("{} {}", newFrom, newTo);
      return true;
    }
  } else {
    return false;
  }
}

// ~, ~> --> * (any, kinda silly)
// ~2, ~2.x, ~2.x.x, ~>2, ~>2.x ~>2.x.x --> >=2.0.0 <3.0.0-0
// ~2.0, ~2.0.x, ~>2.0, ~>2.0.x --> >=2.0.0 <2.1.0-0
// ~1.2, ~1.2.x, ~>1.2, ~>1.2.x --> >=1.2.0 <1.3.0-0
// ~1.2.3, ~>1.2.3 --> >=1.2.3 <1.3.0-0
// ~1.2.0, ~>1.2.0 --> >=1.2.0 <1.3.0-0
// ~0.0.1 --> >=0.0.1 <0.1.0-0
std::string replaceTildes(const std::string &range) {
  std::string M;
  std::string m;
  std::string p;
  std::string pr;

  const auto looseRegex = re::getInstance().getRegex(regexIndex::TILDELOOSE);

  if (RE2::FullMatch(range, *looseRegex, &M, &m, &p, &pr)) {
    if (M.size() > MAX_SAFE_COMPONENT_LENGTH ||
        m.size() > MAX_SAFE_COMPONENT_LENGTH ||
        p.size() > MAX_SAFE_COMPONENT_LENGTH) {
      return range;
    } else {
      std::string result;

      if (isX(M)) {
        result = "";
      } else if (isX(m)) {
        result = fmt::format(">={}.0.0 <{}.0.0-0", M, std::stoll(M) + 1);
      } else if (isX(p)) {
        // ~1.2 == >=1.2.0 <1.3.0-0
        result =
            fmt::format(">={}.{}.0 <{}.{}.0-0", M, m, M, std::stoll(m) + 1);
      } else if (pr.size()) {
        result = fmt::format(">={}.{}.{}-{} <{}.{}.0-0", M, m, p, pr, M,
                             std::stoll(m) + 1);
      } else {
        // ~1.2.3 == >=1.2.3 <1.3.0-0
        result =
            fmt::format(">={}.{}.{} <{}.{}.0-0", M, m, p, M, std::stoll(m) + 1);
      }

      return result;
    }
  } else {
    return range;
  }
}

// ^ --> * (any, kinda silly)
// ^2, ^2.x, ^2.x.x --> >=2.0.0 <3.0.0-0
// ^2.0, ^2.0.x --> >=2.0.0 <3.0.0-0
// ^1.2, ^1.2.x --> >=1.2.0 <2.0.0-0
// ^1.2.3 --> >=1.2.3 <2.0.0-0
// ^1.2.0 --> >=1.2.0 <2.0.0-0
// ^0.0.1 --> >=0.0.1 <0.0.2-0
// ^0.1.0 --> >=0.1.0 <0.2.0-0
std::string replaceCarets(const std::string &range) {
  std::string M;
  std::string m;
  std::string p;
  std::string pr;

  const auto looseRegex = re::getInstance().getRegex(regexIndex::CARETLOOSE);

  if (RE2::FullMatch(range, *looseRegex, &M, &m, &p, &pr)) {
    if (M.size() > MAX_SAFE_COMPONENT_LENGTH ||
        m.size() > MAX_SAFE_COMPONENT_LENGTH ||
        p.size() > MAX_SAFE_COMPONENT_LENGTH) {
      return range;
    } else {
      // includePrerelease is OFF

      std::string result;

      if (isX(M)) {
        result = "";
      } else if (isX(m)) {
        result = fmt::format(">={}.0.0 <{}.0.0-0", M, std::stoll(M) + 1);
      } else if (isX(p)) {
        if (M == "0") {
          result =
              fmt::format(">={}.{}.0 <{}.{}.0-0", M, m, M, std::stoll(m) + 1);
        } else {
          result = fmt::format(">={}.{}.0 <{}.0.0-0", M, m, std::stoll(M) + 1);
        }
      } else if (pr.size()) {
        if (M == "0") {
          if (m == "0") {
            result = fmt::format(">={}.{}.{}-{} <{}.{}.{}-0", M, m, p, pr, M, m,
                                 std::stoll(p) + 1);
          } else {
            result = fmt::format(">={}.{}.{}-{} <{}.{}.0-0", M, m, p, pr, M,
                                 std::stoll(m) + 1);
          }
        } else {
          result = fmt::format(">={}.{}.{}-{} <{}.0.0-0", M, m, p, pr,
                               std::stoll(M) + 1);
        }
      } else {
        if (M == "0") {
          if (m == "0") {
            result = fmt::format(">={}.{}.{} <{}.{}.{}-0", M, m, p, M, m,
                                 std::stoll(p) + 1);
          } else {
            result = fmt::format(">={}.{}.{} <{}.{}.0-0", M, m, p, M,
                                 std::stoll(m) + 1);
          }
        } else {
          result =
              fmt::format(">={}.{}.{} <{}.0.0-0", M, m, p, std::stoll(M) + 1);
        }
      }

      return result;
    }
  } else {
    return range;
  }
}

std::string replaceXRanges(const std::string &range) {
  std::string gtlt;
  std::string M;
  std::string m;
  std::string p;

  const auto looseRegex = re::getInstance().getRegex(regexIndex::XRANGELOOSE);

  if (RE2::FullMatch(range, *looseRegex, &gtlt, &M, &m, &p)) {

    if (M.size() > MAX_SAFE_COMPONENT_LENGTH ||
        m.size() > MAX_SAFE_COMPONENT_LENGTH ||
        p.size() > MAX_SAFE_COMPONENT_LENGTH) {
      return range;
    } else {
      // includePrerelease is OFF

      std::string result;

      std::string pr;

      const bool xM = isX(M);
      const bool xm = xM || isX(m);
      const bool xp = xm || isX(p);
      const bool anyX = xp;

      if (anyX && gtlt == "=") {
        gtlt = "";
      }

      if (xM) {
        if (gtlt == ">" || gtlt == "<") {
          result = "<0.0.0-0";
        } else {
          result = "*";
        }
      } else if (anyX && gtlt.size()) {
        if (xm) {
          m = "0";
        }
        p = "0";

        if (gtlt == ">") {
          // >1 => >=2.0.0
          // >1.2 => >=1.3.0
          gtlt = ">=";
          if (xm) {
            M = std::to_string(std::stoll(M) + 1);
          } else {
            m = std::to_string(std::stoll(m) + 1);
          }
        } else if (gtlt == "<=") {
          // <=0.7.x is actually <0.8.0, since any 0.7.x should
          // pass.  Similarly, <=7.x is actually <8.0.0, etc.
          gtlt = "<";
          pr = "-0";
          if (xm) {
            M = std::to_string(std::stoll(M) + 1);
          } else {
            m = std::to_string(std::stoll(m) + 1);
          }
        } else if (gtlt == "<") {
          pr = "-0";
        }

        result = fmt::format("{}{}.{}.{}{}", gtlt, M, m, p, pr);
      } else if (xm) {
        result = fmt::format(">={}.0.0{} <{}.0.0-0", M, pr, std::stoll(M) + 1);
      } else if (xp) {
        result = fmt::format(">={}.{}.0{} <{}.{}.0-0", M, m, pr, M,
                             std::stoll(m) + 1);
      }

      if (result.empty()) {
        result = range;
      }

      if (result == "*") {
        result = "";
      }

      return result;
    }
  } else {
    return range;
  }
}

// Because * is AND-ed with everything else in the comparator,
// and '' means "any version", just remove the *s entirely.
bool replaceStars(std::string &range) {
  const auto looseRegex = re::getInstance().getRegex(regexIndex::STAR);
  return RE2::GlobalReplace(&range, *looseRegex, re::starReplace);
}

bool replaceGTE0(std::string &range) {
  // includePrerelease is OFF
  const auto looseRegex = re::getInstance().getRegex(regexIndex::GTE0);
  return RE2::GlobalReplace(&range, *looseRegex, re::gte0Replace);
}

// comprised of xranges, tildes, stars, and gtlt's at this point.
// already replaced the hyphen ranges
// turn into a set of JUST comparators.
std::string parseComparator(const std::string &range) {
  if (range.empty()) {
    return range;
  }

  std::string comp;
  if (range[0] == '^') {
    comp = replaceCarets(range);
  } else if (range[0] == '~') {
    comp = replaceTildes(range);
  } else {
    comp = replaceXRanges(range);
  }
  return comp;
}

/* # Range */

range::range(std::string_view range) {
  valid = true;
  bool hasNullSet = false;
  for (const auto r : absl::StrSplit(range, "||")) {
    std::vector<comparator> cs = parseRange(r);
    if (cs.empty()) {
      continue;
    }

    if (cs.size() == 1) {
      // if we have any that are not the null set, throw out null sets.
      if (cs[0].isNullSet()) {
        hasNullSet = true;
        continue;
      }
      // if we have any that are *, then the range is just *
      if (cs[0].isAny()) {
        set = {cs};
        return;
      }
    }

    set.emplace_back(cs);
  }

  if (set.empty()) {
    if (hasNullSet) {
      set = {{comparator("<0.0.0-0")}};
    } else {
      valid = false;
    }
  }

  if (valid) {
    std::vector<std::string> orSetString;
    for (const auto &andSet : set) {
      std::vector<std::string_view> andSetString;
      for (const auto &comp : andSet) {
        andSetString.emplace_back(comp.value);
      }
      orSetString.emplace_back(absl::StrJoin(andSetString, " "));
    }
    value = absl::StrJoin(orSetString, " || ");
  }
}

std::string range::format() const { return value; }

bool range::test(std::string_view version) const {
  if (version.empty()) {
    return false;
  }

  semver ver(version);
  if (!ver.valid) {
    return false;
  }

  for (const auto &andSet : set) {
    bool passAndSet = true;
    for (const auto &comp : andSet) {
      if (!comp.test(ver)) {
        passAndSet = false;
        break;
      }
    }

    if (!passAndSet) {
      continue;
    }

    // includePrerelease is OFF
    if (ver.prerelease.size()) {
      bool prereleaseAllowed = false;
      for (const auto &comp : andSet) {
        if (comp.isAny()) {
          continue;
        }

        if (comp.ver.prerelease.size()) {
          const semver &allowed = comp.ver;
          if (allowed.major == ver.major && allowed.minor == ver.minor &&
              allowed.patch == ver.patch) {
            prereleaseAllowed = true;
            break;
          }
        }
      }
      passAndSet = prereleaseAllowed;
    }

    if (passAndSet) {
      return true;
    }
  }

  return false;
}

std::vector<comparator> range::parseRange(std::string_view range) {
  std::string tmp = std::string(absl::StripAsciiWhitespace(range));

  // `1.2.3 - 1.2.4` => `>=1.2.3 <=1.2.4`
  bool isHyphen = hyphenReplace(tmp);

  if (!isHyphen) {
    // `> 1.2.3 < 1.2.5` => `>1.2.3 <1.2.5`
    RE2::GlobalReplace(&tmp,
                       *re::getInstance().getRegex(regexIndex::COMPARATORTRIM),
                       re::comparatorTrimReplace);

    // `~ 1.2.3` => `~1.2.3`
    RE2::GlobalReplace(&tmp, *re::getInstance().getRegex(regexIndex::TILDETRIM),
                       re::tildeTrimReplace);

    // `^ 1.2.3` => `^1.2.3`
    RE2::GlobalReplace(&tmp, *re::getInstance().getRegex(regexIndex::CARETTRIM),
                       re::caretTrimReplace);

    // normalize spaces
    RE2::GlobalReplace(
        &tmp, *re::getInstance().getRegex(regexIndex::WHITESPACENORMALIZE),
        re::whitespaceNormalizeReplace);
  }

  std::vector<comparator> comparators;
  for (const auto c : absl::StrSplit(tmp, ' ')) {
    for (const auto c : absl::StrSplit(parseComparator(std::string(c)), ' ')) {
      std::string tmp(c);
      replaceGTE0(tmp);
      comparator comp(tmp);

      if (comp.valid) {
        comparators.emplace_back(comp);
      }
    }
  }

  // an empty comparator list generally means that it was not a valid range
  if (comparators.empty()) {
    return {};
  }

  // if any comparators are the null set, then replace with JUST null set
  // if more than one comparator, remove any * comparators
  // also, don't include the same comparator more than once

  std::unordered_set<std::string> rangeSet;
  std::vector<comparator> result;
  for (const auto &comp : comparators) {
    if (comp.isNullSet()) {
      return {comp};
    }
    if (comp.isAny()) {
      continue;
    }
    if (rangeSet.find(comp.value) == rangeSet.end()) {
      rangeSet.insert(comp.value);
      result.emplace_back(comp);
    }
  }

  if (result.empty()) {
    return {comparator("")};
  }

  return result;
}
