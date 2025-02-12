#pragma once

#include <string>
#include <vector>

class semver {
public:
  bool valid;

  // we do not use unsigned long long for major, minor and patch
  // because long long is already enough for MAX_SAFE_INTEGER

  long long major;
  long long minor;
  long long patch;

  std::vector<std::string> prerelease;
  std::vector<std::string> build;

  std::string value;

  semver(std::string_view version);
  semver() = default;

  std::string format() const;

  // this function will not validate both semvers,
  // you should check their `valid` before calling this!
  int compare(const semver &other) const;

  // this function will not validate both semvers,
  // you should check their `valid` before calling this!
  int compareMain(const semver &other) const;

  // this function will not validate both semvers,
  // you should check their `valid` before calling this!
  int comparePre(const semver &other) const;
};

enum class comparatorOperator : std::size_t { EQ, GT, GTE, LT, LTE, ENDOFENUM };

class comparator {
public:
  bool valid;

  comparatorOperator op;
  semver ver;

  std::string value; // value == "" means ANY

  comparator(std::string_view comp);

  // this function will not validate this comparator,
  // you should check the `valid` before calling this!
  bool test(const semver &version) const;

  std::string format() const;

  bool isNullSet() const { return value == "<0.0.0-0"; }

  bool isAny() const { return value.empty(); }
};

class range {
public:
  bool valid;

  std::string value;

  std::vector<std::vector<comparator>> set;

  range(std::string_view range);

  std::string format() const;

  // this function will not validate this range,
  // you should check the `valid` before calling this!
  bool test(std::string_view version) const;

private:
  std::vector<comparator> parseRange(std::string_view comp);
};
